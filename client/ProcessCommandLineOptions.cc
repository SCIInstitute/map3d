/* ProcessCommandLineOptions.cxx */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#pragma warning(disable:4514)
#undef TRACE
#endif

#include "ProcessCommandLineOptions.h"
#include "ParseCommandLineOptions.h"
#include "MainWindow.h"
#include "Map3d_Geom.h"
#include "map3d-struct.h"
#include "Surf_Data.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "WindowManager.h"
#include "Contour_Info.h"
#include "colormaps.h"
#include "eventdata.h"
#include "geomlib.h"
#include "landmarks.h"
#include "MatlabIO.h"
#include "pickinfo.h"
#include "readfiles.h"
#include "scalesubs.h"
#include "dialogs.h"
#include "Transforms.h"
#include "glprintf.h"
#include "savescreen.h"
#include "FileDialog.h"
#include "ScaleDialog.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#ifndef MAX
#define MAX(x,y) ((x>y)?x:y)
#endif
#ifndef MIN
#define MIN(x,y) ((x<y)?x:y)
#endif

  // we initialize these values to -1 (or -FLT_MAX) for most types, so if the values are -1
  // do not use them.  This will compensate for when there is a global option set and the
  // local option is set to the default value to override.

  // so the structure is:
  // if the surface has a value other than -1, use it
  // else: if the global option is not -1, use that
  // else: use a default value.

  //   origVal is the value to compare for it to be unset
#define INITIALIZE_VALUE(surfVar, meshVar, origVal, defaultVal) \
  if (s->surfVar != origVal) m->meshVar = s->surfVar; \
  else if (globalSurf->surfVar != origVal) m->meshVar = globalSurf->surfVar; \
  else m->meshVar = defaultVal;
#define INITIALIZE_VALUE_WITH_ACTION(surfVar, meshVar, origVal, action) \
  if (s->surfVar != origVal) m->meshVar = s->surfVar; \
  else if (globalSurf->surfVar != origVal) m->meshVar = globalSurf->surfVar; \
  else { action; } 

Map3d_Geom *map3d_geom;
extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
extern FileDialog *filedialog;
extern ScaleDialog *scaledialog;
extern FileCache file_cache;
extern int fstep;
extern vector<Surface_Group> surf_group;

vector<Surface_Group> surf_group;


int ProcessCommandLineOptions(Global_Input & g)
{
  if (g.imagefile)
    strcpy(map3d_info.imagefile, g.imagefile);
  int loop, length;
  Mesh_List *meshes = new Mesh_List;
  Mesh_Info *curmesh = 0;

  int width, height;

  map3d_info.screenHeight = QApplication::desktop()->height();
  map3d_info.screenWidth = QApplication::desktop()->width();

  /* extract series numbers from all filenames (the @ notation) */
  ExtractSeriesNumbers(g);
  length = g.numgeomfiles;

  // set some of g's values to map3d_info
  map3d_info.qnovalidity = g.qnovalidity; // don't check validity of meshes
  map3d_info.qnoborders = g.qnoborders;
  map3d_info.singlewin = g.qsinglewin;
  map3d_info.reportlevel = g.report_level;
  map3d_info.scale_scope = g.scale_scope;
  map3d_info.scale_model = g.scale_model;
  map3d_info.scale_mapping = g.scale_mapping;
  map3d_info.lockgeneral = g.lockgeneral;
  map3d_info.lockrotate = g.lockrotate;
  map3d_info.lockframes = g.lockframes;
  map3d_info.pickmode = g.pickmode;
  map3d_info.user_fstep = g.framestep;
  if (g.time_unit && (strcmp(g.time_unit, "s") == 0 || strcmp(g.time_unit, "ms") == 0 || 
      strcmp(g.time_unit, "us")  == 0 || strcmp(g.time_unit, "ns") == 0))
    map3d_info.time_unit = g.time_unit; // defaults to ms
  map3d_info.frames_per_time_unit = g.frames_per_time_unit;
  map3d_info.same_scale = g.same_scale;
  fstep = g.framestep;
  map3d_info.frame_loop = g.frameloop;

  /* if -b was indicated then create a main (composition) window */
  if (g.qnoborders) {
    map3d_info.posx = map3d_info.posy = 0;
    int geoms = 0;
    masterWindow = new MainWindow;
    masterWindow->show();

    //determine main text
    masterWindow->textLines = 0;
    for (loop = 0; loop <= 5 && loop < length; loop++) {
      if (masterWindow->textLines > 5)
        break;
      if ((int)g.SurfList[loop]->potfilenames.size() + 1 > loop) {
        for (int x = 0; x < (int)g.SurfList[loop]->potfilenames.size(); x++) {
          strcpy(masterWindow->mainWindowText[masterWindow->textLines], g.SurfList[loop]->potfilenames[x]);
          masterWindow->textLines++;
        }
      }
      else if (geoms < 3) {     //only print out up to 3 geomfilenames
        strcpy(masterWindow->mainWindowText[masterWindow->textLines], g.SurfList[loop]->geomfilename);
        geoms++;
        masterWindow->textLines++;
      }
      else
        continue;
    }
    if (geoms >= 3 || length > 5) {
      strcpy(masterWindow->mainWindowText[masterWindow->textLines], "+ others\0");
      masterWindow->textLines++;
    }
  }

  /* load the data indicated in the global parse struct.
   * copy from g.SurfList[] to meshes.
   * after this, use only meshes, not g.SurfList[]
   */
  Mesh_List returnedMeshes;
  Mesh_List currentMeshes(1);
  GeomWindow *geompriv=0;
  for (loop = 0; loop < length && loop < MAX_SURFS; loop++) {
    Surf_Input* surf = g.SurfList[loop];
    currentMeshes[0] = new Mesh_Info;
    returnedMeshes = FindAndReadGeom(surf, currentMeshes, RELOAD_NONE);
    if (returnedMeshes.size() == 0)
      delete currentMeshes[0]; // this shouldn't happen that often
    else {
      // assign the meshes to their windows
      if (!geompriv || !g.qsinglewin || surf->newwindow) {
        width = surf->winxmax - surf->winxmin;
        height = surf->winymax - surf->winymin;
        geompriv = GeomWindow::GeomWindowCreate(width, height, surf->winxmin, surf->winymin);
      }
      for (unsigned i = 0; i < returnedMeshes.size(); i++) {
        meshes->push_back(returnedMeshes[i]);
        geompriv->addMesh(returnedMeshes[i]);
      }
    }
  }

  if (loop == MAX_SURFS && loop != length)
    printf("Surfaces after the %dth surface are ignored\n", MAX_SURFS);

  length = meshes->size();
  if (length == 0) {
    // show the files window to start if no geoms loaded
    if (masterWindow)
      masterWindow->startHidden = true;
    filesDialogCreate();
    filedialog->addRow(0);
  }

  for (loop = 0; loop < length; loop++) {
    curmesh = (*meshes)[loop];
    /* Now we should create scalar windows if they exist */
    for (int sloop = 0; sloop < (int)curmesh->scalarInputs.size(); sloop++) {
      //check for existence
      if (curmesh->scalarInputs[sloop].xmin == -1 && curmesh->scalarInputs[sloop].scalarnum == -1)
        continue;

      Scalar_Input si = curmesh->scalarInputs[sloop];
      width = si.xmax - si.xmin;
      height = si.ymax - si.ymin;
      PickWindow *pickPriv = PickWindow::PickWindowCreate(width, height, si.xmin, si.ymin);
      if (!pickPriv) // can fail if more than MAX_PICKS
        continue;

      pickPriv->showinfotext = 1;
      PickInfo *pInfo = new PickInfo;
      pickPriv->pick = pInfo;
      pickPriv->mesh = curmesh;
      pInfo->mesh = curmesh;
      pInfo->show = 1;
      pickPriv->show();

      /* if we have given the -t option */
      if (curmesh->scalarInputs[sloop].scalarnum != -1)
        if (curmesh->geom->leadlinks && curmesh->scalarInputs[sloop].scalarnum <= curmesh->geom->numleadlinks){
	  pInfo->rms = 0;
          pInfo->node = curmesh->geom->leadlinks[curmesh->scalarInputs[sloop].scalarnum - 1];
	}
        else{
	  pInfo->rms = 0;
          pInfo->node = curmesh->scalarInputs[sloop].scalarnum - 1;
	}
      else{
	pInfo->rms = 1;
        pInfo->node = 0;        /* default set to channel 1 */
      }
      if (pInfo->node > curmesh->geom->numpts || pInfo->node < 0) { // error checking
        fprintf(stderr, "Invalid node %d.  Choose between 1 and %d\n", pInfo->node + 1, curmesh->geom->numpts);
        pInfo->node = 0;
      }
      pInfo->type = 1;          /* OK, default we set it equal to pick a node */
      pInfo->pickwin = pickPriv;
      curmesh->pickstacktop++;
      curmesh->pickstack[curmesh->pickstacktop] = pInfo;

    }
  }

  /* clean un needed memory */
  delete meshes;

  for (unsigned i = 0; i < surf_group.size(); i++)
    recalcGroup(i);
  assignMasters(&g);
  GlobalMinMax();

  if (g.bgimage)
    map3d_info.bg_texture = readImage(g.bgimage);

  Broadcast(MAP3D_UPDATE);

  return 1;
}

// look for filenames with a @# at the end of them, and set 
// the # as the series num.  Default series number for each
// surface is 1
void ExtractSeriesNumbers(Global_Input & g)
{
  char *curfilename = 0;
  int loop1, loop2;
  int length2;

  for (loop1 = 0; loop1 < g.numgeomfiles; loop1++) {
    curfilename = g.SurfList[loop1]->geomfilename;

    if (!curfilename) {
      fprintf(stderr, "*** MAP3D ERROR: Bad filename\n");
      continue;
    }

    // it appears that the code reflects geom files' series to be one-based
    // but the data files' to be zero-based. we should fix that someday.
    length2 = strlen(curfilename);
    g.SurfList[loop1]->geomsurfnum = 0;
    for (loop2 = length2 - 1; loop2 >= 0; loop2--) {
      if (curfilename[loop2] == '@') {
        g.SurfList[loop1]->geomsurfnum = atoi(&curfilename[loop2 + 1]);
        curfilename[loop2] = '\0';
      }
    }

    if (GetNumGeoms(curfilename) == 1) {
      g.SurfList[loop1]->geomsurfnum = 1;
    }

    // assign potfile from vector of potfiles
    if (g.SurfList[loop1]->potfilenames.size() > 0)
      g.SurfList[loop1]->potfilename = g.SurfList[loop1]->potfilenames[0];

    g.SurfList[loop1]->timeseries = 0;
    for (unsigned i = 0; i < g.SurfList[loop1]->potfilenames.size(); i++) {
      curfilename = g.SurfList[loop1]->potfilenames[i];
      if (curfilename) {
        length2 = strlen(curfilename);
        for (loop2 = length2 - 1; loop2 >= 0; loop2--) {
          if (curfilename[loop2] == '@') {
            g.SurfList[loop1]->timeseriesnums.push_back(atoi(&curfilename[loop2 + 1]) - 1);  // make it 0-based
            curfilename[loop2] = '\0';
          }
        }
      }
    }
  }
}

Mesh_List FindAndReadGeom(Surf_Input * surf, Mesh_List currentMeshes, int reload)
{
  Mesh_List meshes;
  Mesh_Info* m = currentMeshes[0];

  // This function is getting annoying - but probably better than having 
  // 3 versions of it.  It needs to do 3 things:
  // load in surfaces for the first time, reload surfaces, and change the 
  // surface filename (i.e., if you change the surface from the files window).

  // The multi-surface-in-one-geom-file model is a little different now.  If you choose
  // to load all surfaces, they will be treated as one surface as far as files are concerned,
  // but the same as normal mesh attributes are concerned, they are independent.

  // if you load all the geoms out of a file, then we need to return them all in meshes.
  // Likewise, if you had loaded all the geoms out of a file, and then change it to not,
  // we need to delete those meshes.

  // Since we assume that this is not common, we will have a 'primary' mesh, which
  // will not be deleted.

  // m and m->geom cannot be NULL.  (currentMeshes must have at least 1 good mesh in it).

  Map3d_Geom *g = 0;
  int check = -1;
  unsigned num = 1;  //default value for the number of meshes to open
  FILE *filep = 0;
//  char *filename = surf->geomfilename;
  char filename[256];
  char geomfilename[100];

  if (reload == RELOAD_GEOM && m && m->geom)
    m->geom->destroy();


  if (reload != RELOAD_DATA) {
    m->mysurf = surf;
    bool badfile = false;    
    strcpy(filename, surf->geomfilename);
    switch (DetectFileType(filename)) {
    case PTS_ONLY:
      sprintf(m->geom->basefilename, "%s.pts", filename);
      check = ReadPts(m->geom);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      meshes.push_back(m);
      break;
    case PTS_SEG:
      sprintf(m->geom->basefilename, "%s.pts", filename);
      check = ReadPts(m->geom);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      sprintf(m->geom->basefilename, "%s.seg", filename);
      filep = fopen(m->geom->basefilename, "r");
      if (map3d_info.reportlevel)
        fprintf(stderr, "Reading seg file: %s\n", m->geom->basefilename);
      check = ReadSegs(filep, m->geom);
      fclose(filep);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      meshes.push_back(m);
      break;
    case PTS_FAC:
      sprintf(m->geom->basefilename, "%s.pts", filename);
      check = ReadPts(m->geom);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      sprintf(m->geom->basefilename, "%s.fac", filename);
      check = ReadTris(m->geom);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      meshes.push_back(m);
      ComputeTriNormals(m->geom);
      break;
    case PTS_TETRA:
      sprintf(m->geom->basefilename, "%s.pts", filename);
      check = ReadPts(m->geom);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      sprintf(m->geom->basefilename, "%s.tetra", filename);
      filep = fopen(m->geom->basefilename, "r");
      if (map3d_info.reportlevel)
        fprintf(stderr, "Reading tetra file: %s\n", m->geom->basefilename);
      check = ReadTetras(filep, m->geom);
      fclose(filep);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      meshes.push_back(m);
      ComputeTetNormals(m->geom);
      break;
    case GEOM:
    {
      printf("MAP3D WARNING: map3d no longer supports .geom files\n");
      badfile = true;
      break;
    }
    case MATLAB_FILE:
    {
      sprintf(geomfilename, "%s.mat", filename);
      num = GetNumGeoms(geomfilename);
      int startsurf = 1; // geom surf is 1-based

      if (surf->geomsurfnum > 0 || reload == LOAD_RMS_DATA) {
        num = 1;
        startsurf = surf->geomsurfnum;
      }

      for (unsigned i = 0; i < num; i++) {
        if (i > 0) {
          if (i < currentMeshes.size())
            m = currentMeshes[i];
          else {
            m = new Mesh_Info;
            m->mysurf = surf;
          }
        }
        try {
          // the matlab code can throw an exception.  Potentially it could cause
          // a memory leak if it throws between the time it creates and deletes
          // the "elements" array.  However, the checking above it should make
          // sure that doesn't happen.
          g = m->geom;
          sprintf(g->basefilename, "%s.mat", filename);
          check = ReadMatlabGeomFile(m->geom, i+startsurf);
        } catch (matlabfile::could_not_open_file)	{
          printf("Could not open matlab file\n");
          check = 0;
        } catch (matlabfile::invalid_file_format)	{
          printf("Invalid file format, file is not a matlab file\n");
          check = 0;
        } catch (matlabfile::io_error) {
          printf("IO error\n"); 
          check = 0;
        } catch (matlabfile::out_of_range) {
          printf("Out of range\n");
          check = 0;
        } catch (matlabfile::invalid_file_access) {
          printf("Invalid file access, only read or write access is allowed not both\n");
          check = 0;
        } catch (matlabfile::empty_matlabarray) {
          printf("Usage of an empty matlab array detected\n");
          check = 0;
        } catch (matlabfile::matfileerror) {
          printf("Internal error in MatlabIO library\n");
          check = 0;
        } catch (...) {
          printf("Unknown error in Matlab file\n");
          check = 0;
        }
        if (check == 1 && m->geom->CheckPointValidity() != GEOM_ERROR) {
          ComputeTriNormals(m->geom);
          meshes.push_back(m);
          if (num > 1)
            m->geom->subsurf = i + 1;
        }
        else if (i == 0) {
          // the first file in the multi-surf file needs to be a good surf 
          badfile = true;
          break;
        }
        else {
          // the normal case is that loading the geom will work,
          // but if it doesn't inside a multi-surf file, just delete it.
          // if it is already in a window, remove it, if not, this call is harmless
          m->gpriv->removeMesh(m);
          delete m;
        }
      }
      break;
    }
    case LMARK_FILE:
      sprintf(m->geom->basefilename, "%s.lmark", filename);

      check = ReadLandmarkGeomFile(m->geom);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      ComputeTriNormals(m->geom);
      meshes.push_back(m);
      break;
   case LMARKS_FILE:
      sprintf(m->geom->basefilename, "%s.lmarks", filename);

      check = ReadLandmarkGeomFile(m->geom);
      if (check != 0 || m->geom->CheckPointValidity() == GEOM_ERROR) {
        badfile = true;
        break;
      }
      ComputeTriNormals(m->geom);
      meshes.push_back(m);
      break;
    case DATA_FILE:
      badfile = true;
      break;
    default:
      printf("MAP3D WARNING: Unable to detect file type for file \"%s\"\n", surf->geomfilename);
      badfile = true;
      break;
    }

    if (meshes.size() > 1) {
      // add this mesh to its own group, if it's not already in one
      if (surf->groupid == 0)
        surf->groupid = surf_group.size() + 1;
    }

    // if we started with a multi-surface mesh, but reloaded to have less surfaces, delete the spares
    for (unsigned i = num; i < currentMeshes.size(); i++) {
      currentMeshes[i]->gpriv->removeMesh(currentMeshes[i]);
      delete currentMeshes[i];
    }

    if (badfile) {
      printf("MAP3D WARNING: Unable to read geometry file \"%s\".\n", m->geom->basefilename);
      m->geom->destroy();
    }

    for (unsigned i = 0; i < meshes.size(); i++) {
      Mesh_Info* m = meshes[i];

      ComputeGeomStatistics(m->geom);
      if (reload == RELOAD_NONE || meshes.size() > currentMeshes.size())
        CopySurfToMesh(surf, surf->parent->SurfList[MAX_SURFS], m);

      m->geom->surfnum = surf->displaysurfnum;
      //m->contourspacing = fabs(surf->contourstep);
      m->scalarInputs.clear();
      for (int loop2 = 0; loop2 < (int)surf->scalarInputs.size(); loop2++) {
        /* copy over the -t/-at stuff */
        m->scalarInputs.push_back(surf->scalarInputs[loop2]);
      }

      if (surf->chfilename && strcmp(surf->chfilename,"") != 0) {
        FindAndReadChannels(surf, m->geom);
      }
      if (surf->llfilename && strcmp(surf->llfilename,"") != 0) {
        FindAndReadLeadlinks(surf, m->geom);
      }
      if (surf->lmfilename && strcmp(surf->lmfilename,"") != 0) {
        m->geom->landmarks = DefALandMarkSurf(m->geom->surfnum);
        if (map3d_info.reportlevel)
          fprintf(stderr, "Reading landmark geom file %s\n", surf->lmfilename);
        if (ReadLandMarkFile(surf->lmfilename, m->geom->landmarks, map3d_info.reportlevel) < 0) {
          free(m->geom->landmarks);
          m->geom->landmarks = 0;
        }
      }
    }
    
  } // end if reload != data 
  else
    meshes = currentMeshes;
      
  if (surf->potfilename && strcmp(surf->potfilename, "") != 0 && reload != RELOAD_GEOM) {
    char origPotfilename[256];
    strcpy(origPotfilename, surf->potfilename);
    for (unsigned i = 0; i < meshes.size(); i++) {
      Mesh_Info* m = meshes[i];
      if (i > 0 && i < surf->potfilenames.size())
        // multi-surf geom, specified with multiple data files.
        // the first one will automatically be copied to surf->potfilename
        strcpy(surf->potfilename, surf->potfilenames[i]);
      FindAndReadData(surf, m, reload);
      if (m->data) {
        // data load successful
        sprintf(m->data->potfilename, "%s", surf->potfilename);
        m->data->surfnum = surf->displaysurfnum;
      }
      else if (reload) {
        // bad data load
        // FIX filesDialogCreate();
      }
    }
    strcpy(surf->potfilename, origPotfilename);
  }
  if (map3d_info.reportlevel)
    fprintf(stderr, "\n");
  return meshes;
}

void FindAndReadChannels(Surf_Input * surf, Map3d_Geom * geom)
{
  FILE *filep = 0;
  char *filename = surf->chfilename;

  switch (DetectFileType(filename)) {
  case CHANNELS:
    if (map3d_info.reportlevel)
      fprintf(stderr, "Reading channels file %s\n", filename);
    filep = fopen(filename, "r");
    if (filep == 0) {
      printf("MAP3D WARNING: Unable to find file \"%s\".\n", filename);
      break;
    }
    ReadChannelsFile(filep, geom);
    fclose(filep);
    break;
  case -1:
    printf("MAP3D WARNING: Unable to detect file type for file \"%s\"\n", surf->chfilename);
    break;
  default:
    printf("MAP3D WARNING: file \"%s\" does not contain channels information.\n", surf->chfilename);
    break;
  }
}

void FindAndReadLeadlinks(Surf_Input * surf, Map3d_Geom * geom)
{
  FILE *filep = 0;
  char *filename = surf->llfilename;

  switch (DetectFileType(filename)) {
  case LEADLINKS:
    if (map3d_info.reportlevel)
      fprintf(stderr, "Reading leadlinks file %s\n", surf->llfilename);
    filep = fopen(filename, "r");
    if (filep == 0) {
      printf("MAP3D WARNING: Unable to find file \"%s\".\n", filename);
      break;
    }
    ReadLeadFile(filep, geom);
    fclose(filep);
    break;
  case -1:
    printf("MAP3D WARNING: Unable to detect file type for file \"%s\"\n", surf->llfilename);
    break;
  default:
    printf("MAP3D WARNING: file \"%s\" does not contain leadlinks information.\n", surf->llfilename);
    break;
  }
}

void FindAndReadData(Surf_Input * surf, Mesh_Info * mesh, int reload)
{
  Surf_Data *s = 0;
  Surf_Data *save_mastersurf = 0;
  int fileType;
//  for (int i = 0; i < 512; i++)
//  {
//    if (surf->potfilenames[surf->potfileindex][i] == 0)
//      break;
//    printf("%c", surf->potfilenames[surf->potfileindex][i]);
//    fflush(stdout);
//  }
//  printf("surf->potfileindex = %i\n", surf->potfileindex);
//  printf("strlen(surf->potfilenames[surf->potfileindex]) = %i\n", strlen(surf->potfilenames[surf->potfileindex]));
  //strcpy(filename, surf->potfilename);
  s = mesh->data;
  vector<Mesh_Info*> slaved_meshes;
  if (s) {
    save_mastersurf = s->mastersurf;
    // if s exists, then check to see if there are surfaces slaved to this surface
    for (unsigned i = 0; i < numGeomWindows(); i++)
      for (unsigned j = 0; j < GetGeomWindow(i)->meshes.size(); j++) {
        Mesh_Info* m2 = GetGeomWindow(i)->meshes[j];
        if (m2->data && m2->data->mastersurf == s) {
          m2->data->mastersurf = 0;
          slaved_meshes.push_back(m2);
        }
      }
        
    delete s;
    s = 0;
  }
  map3d_geom = mesh->geom;
  
  fileType = DetectFileType(surf->potfilename);

  switch (fileType) {
  case POT_ONLY:
    //    return;
    sprintf(surf->potfilename, "%s.pot", surf->potfilename);
    s = ReadPotFiles(surf, s, mesh->geom,
                     /*long numsurfsread */ 1, /*long insurfnum */ 0);
    if (s == NULL) {
      printf("MAP3D WARNING: Unable to read data file: %s\n", surf->potfilename);
      mesh->data = 0;
      return;
    }
    mesh->data = s;
    s->mesh = mesh;
    break;
  case GRAD_ONLY:
    break;
  case TSDFC_FILE:
  case DATA_FILE:
  case TSDF_FILE:
    printf("MAP3D WARNING: map3d no longer supports .tsdf, .tsdfc or .data files\n");
    mesh->data = 0;
    return;
  case MATLAB_FILE:
    sprintf(surf->potfilename, "%s.mat", surf->potfilename);
    try {
      s = ReadMatlabDataFile(surf, s, 1, surf->timeseries);
    } catch (matlabfile::could_not_open_file)	{
      printf("Could not open matlab file");
      s = 0;
    } catch (matlabfile::invalid_file_format)	{
      printf("Invalid file format, file is not a matlab file");
      s = 0;
    } catch (matlabfile::io_error) {
      printf("IO error"); 
      s = 0;
    } catch (matlabfile::out_of_range) {
      printf("Out of range");
      s = 0;
    } catch (matlabfile::invalid_file_access) {
      printf("Invalid file access, only read or write access is allowed not both");
      s = 0;
    } catch (matlabfile::empty_matlabarray) {
      printf("Usage of an empty matlab array detected");
      s = 0;
    } catch (matlabfile::matfileerror) {
      printf("Internal error in MatlabIO library");
      s = 0;
    } catch (...) {
      printf("Unknown error in Matlab file");
      s = 0;
    }
    if (s == NULL) {
      printf("MAP3D WARNING: Unable to read data file: %s\n", surf->potfilename);
      mesh->data = 0;
      return;
    }
    mesh->data = s;
    s->mesh = mesh;
    break; 
  case -1:
    printf("MAP3D WARNING: Unable to detect file type for file \"%s\"\n", surf->potfilename);
    mesh->data = 0;
    break;
  default:
    printf("MAP3D WARNING: file \"%s\" does not contain data\n", surf->potfilename);
    mesh->data = 0;
    break;
  }

  /* if data load was successful */
  if (s && s->potvals) {

    //if this is a reload, delete the mesh's contours
    if (mesh->cont) {
      delete mesh->cont;
      mesh->cont = 0;
    }
    s->surfnum = mesh->geom->surfnum;
    s->userpotmax = surf->potusermax;
    s->userpotmin = surf->potusermin;

    // check for fids on non-tsdfc files
    if (fileType != TSDFC_FILE && surf->fidfilename && strcmp(surf->fidfilename,"") != 0) {
      //printf("reading fids\n");
      ReadMap3dFidfile(surf, s, 1, 1);
    }

    // check the fiducials - whether they came from tsdfc or from .fid file or matlab file
    // also adjust the fids if the data does not start at the beginning

    int fidAdjustment = mesh->data->ts_start;
    Series_Fids& fids = mesh->data->fids;
    // adjust the fids
    if (fidAdjustment > 0) {
      for (int lead = 0; lead < fids.numfidleads; lead++) {
        for (int numfidinlead = 0; numfidinlead < fids.leadfids[lead].numfids; numfidinlead++)
          fids.leadfids[lead].fidvals[numfidinlead] -= fidAdjustment;
      }
      //for (int globalfid = 0; globalfid < mesh->data->globalfids.numfids; globalfid++)
        //mesh->data->globalfids.fidvals[globalfid] -= fidAdjustment;
    }
    for(int numfids = 0; numfids < fids.numfidtypes; numfids++){
//#if 0 
      //for new fid dialog
      mesh->fidConts.push_back(new Contour_Info(mesh));
      mesh->fidMaps.push_back(new Contour_Info(mesh));
      mesh->drawFidConts.push_back(true);
      mesh->fidConts.back()->datatype = fids.fidtypes[numfids];
      mesh->fidMaps.back()->datatype = fids.fidtypes[numfids];
      mesh->fidMaps.back()->fidmap = 1;
      mesh->drawfidmap = 0;

      QColor& fidcolor = mesh->fidConts.back()->fidcolor;
      switch(mesh->fidConts.back()->datatype){
        case 0:  fidcolor = Qt::gray;     break;
        case 1:  fidcolor = Qt::gray;     break;
        case 2:  fidcolor = Qt::magenta;  break;
        case 3:  fidcolor = Qt::gray;     break;
        case 4:  fidcolor = Qt::gray;     break;
        case 5:  fidcolor = Qt::gray;     break;
        case 6:  fidcolor = Qt::gray;     break;
        case 7:  fidcolor = Qt::cyan;     break;
        case 8:  fidcolor = Qt::gray;     break;
        case 9:  fidcolor = Qt::gray;     break;
        case 10: fidcolor = Qt::red;      break;
        case 11: fidcolor = Qt::gray;     break;
        case 12: fidcolor = Qt::gray;     break;
        case 13: fidcolor = Qt::green;    break;
        case 14: fidcolor = Qt::yellow;   break;
        case 15: fidcolor = Qt::gray;     break;
        case 16: fidcolor = Qt::blue;     break;              
      }

//#endif    
    //printf("fid %d - %d\n",numfids,mesh->data->fids[fidsets].leadfids[0].fidtypes[numfids]);
//        if((mesh->data->fids[fidsets].leadfids[0].fidtypes[numfids] == 10)){
//	      mesh->fidactcont = new Contour_Info(mesh);
//	      mesh->fidactcont->datatype = 10;
//	      mesh->fidactmapcont = new Contour_Info(mesh);
//	      mesh->fidactmapcont->datatype = 10;
//	      mesh->fidactmapcont->fidmap = 1;
//        }
//        if((mesh->data->fids[fidsets].leadfids[0].fidtypes[numfids] == 13)){
//          mesh->fidreccont = new Contour_Info(mesh);
//          mesh->fidreccont->datatype = 13;
//          mesh->fidrecmapcont = new Contour_Info(mesh);
//          mesh->fidrecmapcont->datatype = 13;
//          mesh->fidrecmapcont->fidmap = 1;
//        }
    }
    



    //parseCommandLineOption_Scale(map3d_geom, s);
    /* compute data extrema */
    s->MinMaxPot(mesh->geom);
    
#if 0
    // FIX
    // set default scopes (don't replace them if something is already set)
    if (surf->scale_lock > 0 && map3d_info.scale_scope == LOCAL_SCALE) {
      map3d_info.scale_scope = SLAVE_FRAME;
      if (scaledialog)
        gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[SLAVE_FRAME])->check_button.toggle_button,true);
    }
    else if (surf->potusermin < surf->potusermax && map3d_info.scale_scope == LOCAL_SCALE) {
      //if (scaledialog)
        //gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[USER_SCALE])->check_button.toggle_button,true);
      //map3d_info.scale_scope = USER_SCALE;
      s->user_scaling = true;
    }
    else if (surf->groupid > 0 && map3d_info.scale_scope == LOCAL_SCALE) {
      if (scaledialog)
        gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GROUP_FRAME])->check_button.toggle_button,true);
      map3d_info.scale_scope = GROUP_FRAME;
    }
#endif

    //number of contours taken care of in buildContour if spacing
    //build contours at the end of Processcommandlineoptions,
    // or after calling this function
    if (mesh->contourspacing)
      map3d_info.use_spacing = true;
    else {
      if (mesh->mysurf->numconts != DEF_NUMCONTS) 
        mesh->data->numconts = mesh->mysurf->numconts;
      else if (mesh->mysurf->parent->SurfList[MAX_SURFS]->numconts != DEF_NUMCONTS)
        mesh->data->numconts = mesh->mysurf->parent->SurfList[MAX_SURFS]->numconts;
      else
        mesh->data->numconts = 10;
    }

    // reslave meshes that were originally assigned to this surf
    for (unsigned m = 0; m < slaved_meshes.size(); m++) {
      slaved_meshes[m]->data->mastersurf = s;
    }

    s->mastersurf = save_mastersurf;

    /* create colormap legend window */
    LegendWindow *lpriv = NULL;
    if (!s->mesh->legendwin && reload != LOAD_RMS_DATA) {

      int width, height;
      width = mesh->lw_xmax - mesh->lw_xmin;
      height = mesh->lw_ymax - mesh->lw_ymin;
      lpriv = LegendWindow::LegendWindowCreate(mesh, width, height, mesh->lw_xmin, mesh->lw_ymin, !mesh->showlegend);

      if (lpriv != NULL) {
        // can fail if more than MAX_SURFS l-wins.
        lpriv->orientation = surf->lworientation;
        if (mesh->mysurf->legendticks != -1) {
          lpriv->nticks = mesh->mysurf->legendticks;
          lpriv->matchContours = false;
        }
        lpriv->surf = s;
        lpriv->mesh = mesh;
        lpriv->map = &mesh->cmap;
        mesh->legendwin = lpriv;
        
        // background color
        lpriv->bgcolor[0] = mesh->mysurf->colour_bg[0] / 255.f;
        lpriv->bgcolor[1] = mesh->mysurf->colour_bg[1] / 255.f;
        lpriv->bgcolor[2] = mesh->mysurf->colour_bg[2] / 255.f;
        lpriv->fgcolor[0] = mesh->mysurf->colour_fg[0] / 255.f;
        lpriv->fgcolor[1] = mesh->mysurf->colour_fg[1] / 255.f;
        lpriv->fgcolor[2] = mesh->mysurf->colour_fg[2] / 255.f;

        if (!mesh->showlegend) {
          lpriv->hide();
        }
        mesh->showlegend = 1;
        GlobalMinMax();
      }
    }
    else if (s->mesh->legendwin) {
      // otherwise, set the current legend win to the new surf data
      s->mesh->legendwin->surf = s;
      GlobalMinMax();
    }
  }
}

void CopySurfToMesh(Surf_Input * s, Surf_Input* globalSurf, Mesh_Info * m)
{
  // copy command-line options to the mesh - start with options that can
  // be set globally OR for each mesh. Make sure mesh-specific values were
  // not set before using the global options
  

  // we do need to set the clipping plane rotation, the vfov, and bg/fg color 
  // when we create the geom window


  // Mesh color
  INITIALIZE_VALUE(colour_mesh[0], meshcolor[0], -1, DEF_MESH_RED);
  INITIALIZE_VALUE(colour_mesh[1], meshcolor[1], -1, DEF_MESH_GREEN);
  INITIALIZE_VALUE(colour_mesh[2], meshcolor[2], -1, DEF_MESH_BLUE);
  m->meshcolor[0] /= 255.f;
  m->meshcolor[1] /= 255.f;
  m->meshcolor[2] /= 255.f;

  // default leadlinks color to purple
  INITIALIZE_VALUE(colour_ll[0], mark_lead_color[0], -1, 255);
  INITIALIZE_VALUE(colour_ll[1], mark_lead_color[1], -1, 0);
  INITIALIZE_VALUE(colour_ll[2], mark_lead_color[2], -1, 255);
  m->mark_lead_color[0] /= 255.f;
  m->mark_lead_color[1] /= 255.f;
  m->mark_lead_color[2] /= 255.f;

  // contour spacing
  // set it to a default later, after we've read data
  INITIALIZE_VALUE(contourstep, contourspacing, 0, DEF_CONTOURSPACING);
  // Shading mode
  INITIALIZE_VALUE(shadingmodel, shadingmodel, -1, DEF_SHADINGMODEL);

  // colormap - cannot use the shorcut, since we are assigning colormaps and not values
  int val = -1;
  if (s->colormap != -1)
    val = s->colormap;
  else if (globalSurf->colormap != -1)
    val = globalSurf->colormap;
  switch (val) {
    case 0:
      m->cmap = &Rainbow;
      break;
    case 1:
      m->cmap = &Green2Red;
      break;
    case 2:
      m->cmap = &Grayscale;
      break;
    case 3:
      m->cmap = &Jet;
      break;
    default:
      m->cmap = &Jet;
      break;
  }

  // Rendering mode
  INITIALIZE_VALUE_WITH_ACTION(drawmesh, drawmesh, -1, 
    if (m->geom->numpts > 0) m->drawmesh = RENDER_MESH_PTS_CONN;
    else m->drawmesh = DEF_DRAWMESH);

  // inverted colormap
  INITIALIZE_VALUE(invert, invert, -1, DEF_INVERT);

  // lighting
  INITIALIZE_VALUE(lighting, lighting, -1, DEF_LIGHTING);

  // fogging
  INITIALIZE_VALUE(fogging, fogging, -1, DEF_FOGGING);

  // node marks
  INITIALIZE_VALUE(all_sphere, mark_all_sphere, -1, 0);
  INITIALIZE_VALUE(all_mark, mark_all_number, -1, 0);
  INITIALIZE_VALUE(all_value, mark_all_sphere_value, -1, 0);
  INITIALIZE_VALUE(extrema_sphere, mark_extrema_sphere, -1, 0);
  INITIALIZE_VALUE(extrema_mark, mark_extrema_number, -1, 0);
  INITIALIZE_VALUE(lead_sphere, mark_lead_sphere, -1, 1);
  INITIALIZE_VALUE(lead_mark, mark_lead_number, -1, 4);
  INITIALIZE_VALUE(pick_sphere, mark_ts_sphere, -1, 1);
  INITIALIZE_VALUE(pick_mark, mark_ts_number, -1, 3);

  INITIALIZE_VALUE(size_ll, mark_lead_size, -1, 3);
  // rotation quaternion - special case
  if (s->rotationQuat.w != -FLT_MAX && s->rotationQuat.x != -FLT_MAX && 
      s->rotationQuat.y != -FLT_MAX && s->rotationQuat.z != -FLT_MAX) {
    m->tran->setRotationQuaternion(s->rotationQuat);
  }
  else if (globalSurf->rotationQuat.w != -FLT_MAX && globalSurf->rotationQuat.x != -FLT_MAX && 
           globalSurf->rotationQuat.y != -FLT_MAX && globalSurf->rotationQuat.z != -FLT_MAX) {
    m->tran->setRotationQuaternion(globalSurf->rotationQuat);
  }
  else{
    m->tran->setRotationQuaternion(qOne);
  }

  // translation coordinates
  INITIALIZE_VALUE(translation[0], tran->tx, -FLT_MAX, 0);
  INITIALIZE_VALUE(translation[1], tran->ty, -FLT_MAX, 0);
  INITIALIZE_VALUE(translation[2], tran->tz, -FLT_MAX, 0);

  // show legend window (or not)
  INITIALIZE_VALUE(showlegend, showlegend, -1, DEF_SHOWLEGEND);
  if (s->showlegend != -1)
    m->showlegend = s->showlegend != 0;
  else if (globalSurf->showlegend != -1)
    m->showlegend = globalSurf->showlegend != 0;
  else
    m->showlegend = DEF_SHOWLEGEND;

  // number of ticks in LW
  if (s->legendticks == -1 && globalSurf->legendticks != -1)
    s->legendticks = globalSurf->legendticks;

  // show contour lines (or not)
  INITIALIZE_VALUE(drawcont, drawcont, -1, DEF_DRAWCONT);

  // render fiducials (or not)
  INITIALIZE_VALUE(drawfids, drawfids, -1, 1);

  // whether to draw the negative contours dashed or not
  INITIALIZE_VALUE(negcontdashed, negcontdashed, -1, DEF_NEGCONTDASHED);

  // draw axes (or not)
  INITIALIZE_VALUE(axes, axes, -1, DEF_AXES);
  if (s->axes != -1)
    m->axes = s->axes != 0;
  else if (globalSurf->axes != -1)
    m->axes = globalSurf->axes  != 0;
  else
    m->axes = DEF_AXES;

  // axes color
  INITIALIZE_VALUE(axes_color[0], axescolor[0], -1, DEF_AXES_COLOR_RGB);
  INITIALIZE_VALUE(axes_color[1], axescolor[1], -1, DEF_AXES_COLOR_RGB);
  INITIALIZE_VALUE(axes_color[2], axescolor[2], -1, DEF_AXES_COLOR_RGB);
  m->axescolor[0] /= 255.0f;
  m->axescolor[1] /= 255.0f;
  m->axescolor[2] /= 255.0f;

  // set up vfov in the surf for when we create the window so we can copy it to the window
  if (s->vfov == -FLT_MAX) {
    if (globalSurf->vfov != -FLT_MAX)
      s->vfov = globalSurf->vfov;
    else
      s->vfov = DEF_VFOV;
  }

  if (s->showinfotext == -1 && globalSurf->showinfotext != -1)
    s->showinfotext = globalSurf->showinfotext;
  if (s->showlocks == -1 && globalSurf->showlocks != -1)
    s->showlocks = globalSurf->showlocks;

  
  // background color - set up here and copy to window later
  if (s->colour_bg[0] == -1 || s->colour_bg[1] == -1 || s->colour_bg[2] == -1) {
    if (globalSurf->colour_bg[0] != -1 && globalSurf->colour_bg[1] != -1 && globalSurf->colour_bg[2] != -1) {
      s->colour_bg[0] = globalSurf->colour_bg[0];
      s->colour_bg[1] = globalSurf->colour_bg[1];
      s->colour_bg[2] = globalSurf->colour_bg[2];
    }
    else {
      s->colour_bg[0] = 0;
      s->colour_bg[1] = 0;
      s->colour_bg[2] = 0;
    }    
  }
  // foreground color - set up here and copy to window later
  if (s->colour_fg[0] == -1 || s->colour_fg[1] == -1 || s->colour_fg[2] == -1) {
    if (globalSurf->colour_fg[0] != -1 && globalSurf->colour_fg[1] != -1 && globalSurf->colour_fg[2] != -1) {
      s->colour_fg[0] = globalSurf->colour_fg[0];
      s->colour_fg[1] = globalSurf->colour_fg[1];
      s->colour_fg[2] = globalSurf->colour_fg[2];
    }
    else {
      s->colour_fg[0] = 255;
      s->colour_fg[1] = 255;
      s->colour_fg[2] = 255;
    }    
  }  
  
  //font sizes
  if (s->large_font == -1)
    if (globalSurf->large_font != -1)
      s->large_font = globalSurf->large_font;
    else
      s->large_font = 5;

  if (s->med_font == -1)
    if (globalSurf->med_font != -1)
      s->med_font = globalSurf->med_font;
    else
      s->med_font = 4;

  if (s->small_font == -1)
    if (globalSurf->small_font != -1)
      s->small_font = globalSurf->small_font;
    else
      s->small_font = 3;

#ifdef __APPLE__
  s->large_font++;
  s->med_font++;
  s->small_font++;
#endif
  
  m->groupid = (s->groupid > 0 ) ? s->groupid -1 : 0;
  if (m->groupid >= (int)surf_group.size()) {
    surf_group.resize(m->groupid + 1);
  }


  // -as stuff
  m->w_xmin = s->winxmin;
  m->w_xmax = s->winxmax;
  m->w_ymin = s->winymin;
  m->w_ymax = s->winymax;

  // -al stuff
  m->lw_xmin = s->lwxmin;
  m->lw_xmax = s->lwxmax;
  m->lw_ymin = s->lwymin;
  m->lw_ymax = s->lwymax;

}

int DetectFileType(char *f)
{
  // here we find what type of file we are looking at.  First look at the 
  // extension, then if there is no extension, add some and check for 
  // existence.  This is so the user doesn't have to type the 
  // extension if he doesn't want to (but we also need to compensate if
  // he did have one).
  const char *ext = 0;
  int filetype = -1;
  FILE *filein = 0;
  int length = strlen(f);
  char *filename = new char[length + 12];
  char *newname = new char[length + 12];

  strcpy(filename, f);

  length = strlen(filename);

  ext = GetExtension(filename);

  if (map3d_info.reportlevel >= 2)
    printf("ext for %s  is %s\n", filename, ext);

  if (strcmp(ext, ".pts") == 0) {
    filetype = PTS_ONLY;
    f[length - 4] = '\0';
  }
  else if (strcmp(ext, ".seg") == 0) {
    filetype = PTS_SEG;
    f[length - 4] = '\0';
  }
  else if (strcmp(ext, ".fac") == 0) {
    filetype = PTS_FAC;
    f[length - 4] = '\0';
  }
  else if (strcmp(ext, ".tetra") == 0) {
    filetype = PTS_TETRA;
    f[length - 6] = '\0';
  }
  else if (strcmp(ext, ".geom") == 0) {
    filetype = GEOM;
    f[length - 5] = '\0';
  }
  else if (strcmp(ext, ".mat") == 0) {
    filetype = MATLAB_FILE;
    f[length - 4] = '\0';
  }
  else if (strcmp(ext, ".tsdf") == 0) {
    filetype = TSDF_FILE;
    f[length - 5] = '\0';
  }
  else if (strcmp(ext, ".data") == 0) {
    filetype = DATA_FILE;
    f[length - 5] = '\0';
  }
  else if (strcmp(ext, ".pot") == 0) {
    filetype = POT_ONLY;
    f[length - 4] = '\0';
  }
  else if (strcmp(ext, ".grad") == 0) {
    filetype = GRAD_ONLY;
    f[length - 5] = '\0';
  }
  else if (strcmp(ext, ".tsdfc") == 0) {
    filetype = TSDFC_FILE;
    f[length - 6] = '\0';
  }
  else if (strcmp(ext, ".channels") == 0) {
    filetype = CHANNELS;  
  }
  else if (strcmp(ext, ".leadlinks") == 0) {
    filetype = LEADLINKS; 
  }
  else if (strcmp(ext, ".lmark") == 0) {
    filetype = LMARK_FILE;
    f[length - 6] = '\0';
  }
  else if (strcmp(ext, ".lmarks") == 0) {
    filetype = LMARKS_FILE;
    f[length - 7] = '\0';
  }
  else {
    /*
    sprintf(newname, "%s.channels", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = CHANNELS;
      fclose(filein);
    }
    sprintf(newname, "%s.leadlinks", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = LEADLINKS;
      fclose(filein);
    }
    */
    sprintf(newname, "%s.pts", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = PTS_ONLY;
      fclose(filein);
    }

    sprintf(newname, "%s.seg", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = PTS_SEG;
      fclose(filein);
    }

    sprintf(newname, "%s.fac", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = PTS_FAC;
      fclose(filein);
    }

    sprintf(newname, "%s.tetra", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = PTS_TETRA;
      fclose(filein);
    }

    sprintf(newname, "%s.geom", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = GEOM;
      fclose(filein);
    }

    sprintf(newname, "%s.data", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = DATA_FILE;
      fclose(filein);
    }

    sprintf(newname, "%s.tsdf", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = TSDF_FILE;
      fclose(filein);
    }
    sprintf(newname, "%s.pot", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = POT_ONLY;
      fclose(filein);
    }

    sprintf(newname, "%s.grad", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = GRAD_ONLY;
      fclose(filein);
    }

    sprintf(newname, "%s.tsdfc", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = TSDFC_FILE;
      fclose(filein);
    }

    sprintf(newname, "%s.mat", filename);
    filein = fopen(newname, "r");
    if (filein) {
      filetype = MATLAB_FILE;
      fclose(filein);
    }
  }

  delete[]filename;
  delete[]newname;
  return filetype;
}

const char *GetExtension(const char *s)
{
  int index;
  for (index = strlen(s); s[index] != '.' && index != 0; index--);
  if (index == 0)
    return &(s[strlen(s)]);
  return &(s[index]);
}

unsigned GetNumGeoms(char *s)
{
  // if we're not a geom file or there is an opening problem...
  if (strcmp(GetExtension(s),".mat") == 0) {
    try {
      matlabfile mf(s, "r");
      return mf.getmatlabarrayinfo(0).getnumelements();
    } catch(...) {
      return 1;
    }
  }
  else
    return (1);
}

void GetDataFileInfo(std::string filename, int& numTimeSeries, std::vector<int>& numFramesPerSeries, std::vector<std::string>& timeSeriesLabels, MatlabIO::matlabarray*& outArray)
{
  numTimeSeries = 1;
  numFramesPerSeries.clear();
  timeSeriesLabels.clear();
  outArray = NULL;

  if (strcmp(GetExtension(filename.c_str()),".mat") == 0) {
    try {
      outArray = new matlabarray;
      matlabfile mf(filename, "r");

      *outArray = mf.getmatlabarray(0);
      if (!outArray->isempty() && (outArray->isstruct() || outArray->iscell()))
        numTimeSeries = outArray->getnumelements();

      for (int i = 0; i < numTimeSeries; i++)
      {
        // series labels
        // default label - number to string
        std::string name = std::to_string(i);
        if (!outArray->isempty()) {
          if (outArray->isstruct()) {
            if (i < outArray->getnumelements() && outArray->isfieldCI("label")) {
              name = outArray->getfieldCI(i, "label").getstring();
            }
          }
          if (outArray->iscell()) {
            if (i < outArray->getnumelements() && outArray->getcell(i).isfieldCI("label")) {
              name = outArray->getcell(i).getfieldCI(0, "label").getstring();
            }
          }
        }

        timeSeriesLabels.push_back(name);

        // num frames
        int numframes = 1;
        matlabarray temp,cell;
        if (outArray->isstruct()) {
          if (outArray->isfieldCI("potvals")) temp = outArray->getfieldCI(i, "potvals");
          if (outArray->isfieldCI("data")) temp = outArray->getfieldCI(i, "data");
          if (outArray->isfieldCI("field")) temp = outArray->getfieldCI(i, "field");
          if (outArray->isfieldCI("scalarfield")) temp = outArray->getfieldCI(i, "scalarfield");

          if (!temp.isempty())
          {
            numframes = temp.getn();
          }
        }
        else if (outArray->iscell()) {
          cell = outArray->getcell(i);
          if (cell.isstruct())
          {
            if (cell.isfieldCI("potvals")) temp = cell.getfieldCI(0, "potvals");
            if (cell.isfieldCI("data")) temp = cell.getfieldCI(0, "data");
            if (cell.isfieldCI("field")) temp = cell.getfieldCI(0, "field");
            if (cell.isfieldCI("scalarfield")) temp = cell.getfieldCI(0, "scalarfield");

            if (!temp.isempty())
            {
              numframes = temp.getn();
            }
          }
        }
        else if (outArray->isdense())
        {
          numframes = outArray->getn();
        }

        numFramesPerSeries.push_back(numframes);
      }
    } catch (...) {
      // do nothing - warning will appear when we try to read the data
      delete outArray;
      outArray = NULL;
      numFramesPerSeries.clear();
      timeSeriesLabels.clear();
      // defaults on failure
      numFramesPerSeries.push_back(1);
      timeSeriesLabels.push_back("1");

    }
  }

  else if (strcmp(GetExtension(filename.c_str()),".pot") == 0) {
    // find faster way of doing this?
    // strip off the .pot and the numbers before it, and increment numbers
    // until we find a file that doesn't exist

    numTimeSeries = 1;
    timeSeriesLabels.push_back("1");
    int numframes = 1;

    char basefilename[256];
    strcpy(basefilename, filename.c_str());
    StripExtension(basefilename);
    basefilename[strlen(basefilename)-3] = 0;
    int filenum;
    for (filenum = 0; /*until it breaks*/; filenum++) {
      char filename[256];
      sprintf(filename, "%s%-3.3ld.pot", basefilename, filenum + 1);
      FILE* f = fopen(filename, "r");
      if (!f) break;
      fclose(f);
    }
    if (filenum > 0)
      numframes = filenum;

    numFramesPerSeries.push_back(numframes);
  }

  // sane defaults - old code used to expect numframes 1 and label "1" if they weren't valid
  if (numFramesPerSeries.size() == 0)
    numFramesPerSeries.push_back(1);
  if (timeSeriesLabels.size() == 0)
    timeSeriesLabels.push_back("1");

}

void ComputeGeomStatistics(Map3d_Geom * m)
{
  float xsize, ysize, zsize;

  m->xmax = m->ymax = m->zmax = -FLT_MAX;
  m->xmin = m->ymin = m->zmin = FLT_MAX;

  for (unsigned i = 0; i < m->points.size(); i++) {
    float **modelpts = m->points[i];

    for (int loop1 = 0; loop1 < m->numpts; loop1++) {
      m->xmax = MAX(m->xmax, modelpts[loop1][0]);
      m->ymax = MAX(m->ymax, modelpts[loop1][1]);
      m->zmax = MAX(m->zmax, modelpts[loop1][2]);
      m->xmin = MIN(m->xmin, modelpts[loop1][0]);
      m->ymin = MIN(m->ymin, modelpts[loop1][1]);
      m->zmin = MIN(m->zmin, modelpts[loop1][2]);
    }
  }
  xsize = m->xmax - m->xmin;
  ysize = m->ymax - m->ymin;
  zsize = m->zmax - m->zmin;
  m->xcenter = xsize / 2.f + m->xmin;
  m->ycenter = ysize / 2.f + m->ymin;
  m->zcenter = zsize / 2.f + m->zmin;
  m->l2norm = (float)sqrt(xsize * xsize + ysize * ysize + zsize * zsize);
}

void ComputeTetNormals(Map3d_Geom * m)
{
  int index0, index1, index2, index3;
  int loop, length = m->numelements;
  float v1x, v1y, v1z, v2x, v2y, v2z;
  float n;

  m->ptnormals = Alloc_fmatrix(m->numelements * 4, 3);
  m->fcnormals = Alloc_fmatrix(m->numelements * 4, 3);

  for (loop = 0; loop < length; loop++) {
    float **modelpts = m->points[m->geom_index];
    index0 = m->elements[loop][0];
    index1 = m->elements[loop][1];
    index2 = m->elements[loop][2];
    index3 = m->elements[loop][3];

    v1x = modelpts[index1][0] - modelpts[index0][0];
    v1y = modelpts[index1][1] - modelpts[index0][1];
    v1z = modelpts[index1][2] - modelpts[index0][2];

    v2x = modelpts[index2][0] - modelpts[index0][0];
    v2y = modelpts[index2][1] - modelpts[index0][1];
    v2z = modelpts[index2][2] - modelpts[index0][2];

    m->fcnormals[loop][0] += v1y * v2z - v2y * v1z;
    m->fcnormals[loop][1] += v1z * v2x - v2z * v1x;
    m->fcnormals[loop][2] += v1x * v2y - v2x * v1y;

    m->ptnormals[index0][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index0][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index0][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index1][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index1][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index1][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index2][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index2][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index2][2] += v1x * v2y - v2x * v1y;

    v1x = modelpts[index2][0] - modelpts[index0][0];
    v1y = modelpts[index2][1] - modelpts[index0][1];
    v1z = modelpts[index2][2] - modelpts[index0][2];

    v2x = modelpts[index3][0] - modelpts[index0][0];
    v2y = modelpts[index3][1] - modelpts[index0][1];
    v2z = modelpts[index3][2] - modelpts[index0][2];

    m->fcnormals[loop + 1][0] += v1y * v2z - v2y * v1z;
    m->fcnormals[loop + 1][1] += v1z * v2x - v2z * v1x;
    m->fcnormals[loop + 1][2] += v1x * v2y - v2x * v1y;

    m->ptnormals[index0][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index0][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index0][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index2][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index2][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index2][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index3][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index3][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index3][2] += v1x * v2y - v2x * v1y;

    v1x = modelpts[index3][0] - modelpts[index0][0];
    v1y = modelpts[index3][1] - modelpts[index0][1];
    v1z = modelpts[index3][2] - modelpts[index0][2];

    v2x = modelpts[index1][0] - modelpts[index0][0];
    v2y = modelpts[index1][1] - modelpts[index0][1];
    v2z = modelpts[index1][2] - modelpts[index0][2];

    m->fcnormals[loop + 2][0] += v1y * v2z - v2y * v1z;
    m->fcnormals[loop + 2][1] += v1z * v2x - v2z * v1x;
    m->fcnormals[loop + 2][2] += v1x * v2y - v2x * v1y;

    m->ptnormals[index0][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index0][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index0][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index3][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index3][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index3][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index1][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index1][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index1][2] += v1x * v2y - v2x * v1y;

    v1x = modelpts[index3][0] - modelpts[index1][0];
    v1y = modelpts[index3][1] - modelpts[index1][1];
    v1z = modelpts[index3][2] - modelpts[index1][2];

    v2x = modelpts[index2][0] - modelpts[index1][0];
    v2y = modelpts[index2][1] - modelpts[index1][1];
    v2z = modelpts[index2][2] - modelpts[index1][2];

    m->fcnormals[loop + 3][0] += v1y * v2z - v2y * v1z;
    m->fcnormals[loop + 3][1] += v1z * v2x - v2z * v1x;
    m->fcnormals[loop + 3][2] += v1x * v2y - v2x * v1y;

    m->ptnormals[index3][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index3][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index3][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index1][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index1][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index1][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index2][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index2][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index2][2] += v1x * v2y - v2x * v1y;
  }

  length = m->numpts;
  for (loop = 0; loop < length; loop++) {
    n = (float)sqrt(m->ptnormals[loop][0] * m->ptnormals[loop][0] +
                    m->ptnormals[loop][1] * m->ptnormals[loop][1] + m->ptnormals[loop][2] * m->ptnormals[loop][2]);

    if (n < 0.0001 && n > -0.0001)
      n = 1;

    m->ptnormals[loop][0] /= n;
    m->ptnormals[loop][1] /= n;
    m->ptnormals[loop][2] /= n;
  }

  length = m->numelements;
  for (loop = 0; loop < length; loop++) {
    n = (float)sqrt(m->fcnormals[loop][0] * m->fcnormals[loop][0] +
                    m->fcnormals[loop][1] * m->fcnormals[loop][1] + m->fcnormals[loop][2] * m->fcnormals[loop][2]);

    if (n < 0.0001 && n > -0.0001)
      n = 1;

    m->fcnormals[loop][0] /= n;
    m->fcnormals[loop][1] /= n;
    m->fcnormals[loop][2] /= n;

    n = (float)sqrt(m->fcnormals[loop + 1][0] * m->fcnormals[loop + 1][0] +
                    m->fcnormals[loop + 1][1] * m->fcnormals[loop + 1][1] +
                    m->fcnormals[loop + 1][2] * m->fcnormals[loop + 1][2]);

    if (n < 0.0001 && n > -0.0001)
      n = 1;

    m->fcnormals[loop + 1][0] /= n;
    m->fcnormals[loop + 1][1] /= n;
    m->fcnormals[loop + 1][2] /= n;

    n = (float)sqrt(m->fcnormals[loop + 2][0] * m->fcnormals[loop + 2][0] +
                    m->fcnormals[loop + 2][1] * m->fcnormals[loop + 2][1] +
                    m->fcnormals[loop + 2][2] * m->fcnormals[loop + 2][2]);

    if (n < 0.0001 && n > -0.0001)
      n = 1;

    m->fcnormals[loop + 2][0] /= n;
    m->fcnormals[loop + 2][1] /= n;
    m->fcnormals[loop + 2][2] /= n;

    n = (float)sqrt(m->fcnormals[loop + 3][0] * m->fcnormals[loop + 3][0] +
                    m->fcnormals[loop + 3][1] * m->fcnormals[loop + 3][1] +
                    m->fcnormals[loop + 3][2] * m->fcnormals[loop + 3][2]);

    if (n < 0.0001 && n > -0.0001)
      n = 1;

    m->fcnormals[loop + 3][0] /= n;
    m->fcnormals[loop + 3][1] /= n;
    m->fcnormals[loop + 3][2] /= n;
  }
}

void ComputeTriNormals(Map3d_Geom * m)
{
  int index0, index1, index2;
  int loop, length = m->numelements;
  float v1x, v1y, v1z, v2x, v2y, v2z;
  float n;

  m->ptnormals = Alloc_fmatrix(m->numpts, 3);
  m->fcnormals = Alloc_fmatrix(m->numelements, 3);
  m->adjacent_triangles = Alloc_lmatrix(m->numelements, 3);

  for (loop = 0; loop < length; loop++) {
    float **modelpts = m->points[m->geom_index];
    index0 = m->elements[loop][0];
    index1 = m->elements[loop][1];
    index2 = m->elements[loop][2];

    v1x = modelpts[index1][0] - modelpts[index0][0];
    v1y = modelpts[index1][1] - modelpts[index0][1];
    v1z = modelpts[index1][2] - modelpts[index0][2];

    v2x = modelpts[index2][0] - modelpts[index0][0];
    v2y = modelpts[index2][1] - modelpts[index0][1];
    v2z = modelpts[index2][2] - modelpts[index0][2];

    m->fcnormals[loop][0] += v1y * v2z - v2y * v1z;
    m->fcnormals[loop][1] += v1z * v2x - v2z * v1x;
    m->fcnormals[loop][2] += v1x * v2y - v2x * v1y;

    m->ptnormals[index0][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index0][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index0][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index1][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index1][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index1][2] += v1x * v2y - v2x * v1y;
    m->ptnormals[index2][0] += v1y * v2z - v2y * v1z;
    m->ptnormals[index2][1] += v1z * v2x - v2z * v1x;
    m->ptnormals[index2][2] += v1x * v2y - v2x * v1y;

    // build an adjacency list.  This is basically for finding the normals for
    // the band shading points, whose points will either lay on the edge or node of
    // a triangle
    for (int edge = 0; edge < 3; edge++) { 
      int loop2;
      if (length < 20000) {
        int pt1 = m->elements[loop][edge];
        int pt2 = m->elements[loop][(edge+1)%3];
        for (loop2 = 0; loop2 < length; loop2++) {
          long* tri = m->elements[loop2];
          if ((tri[0] == pt1 || tri[1] == pt1 || tri[2] == pt1) && (tri[0] == pt2 || tri[1] == pt2 || tri[2] == pt2)) {
            m->adjacent_triangles[loop][edge] = loop2;
            break;
          }
        }
      }
      else {
        static bool warned = false;
        if (!warned) {
          printf("Geometry has many triangles.  Disabling Band-shaded lighting,\nas it would take too long to set up\n");
          warned = true;
        }
        loop2 = length;
      }
      if (loop2 == length) {
        // edge not found
        m->adjacent_triangles[loop][edge] = -1;
      }
    }
  }

  for (loop = 0; loop < length; loop++) {
    n = (float)sqrt(m->fcnormals[loop][0] * m->fcnormals[loop][0] +
                    m->fcnormals[loop][1] * m->fcnormals[loop][1] + m->fcnormals[loop][2] * m->fcnormals[loop][2]);

    if (n < 0.0001 && n > -0.0001)
      n = 1;

    m->fcnormals[loop][0] /= n;
    m->fcnormals[loop][1] /= n;
    m->fcnormals[loop][2] /= n;
  }

  length = m->numpts;
  for (loop = 0; loop < length; loop++) {
    n = (float)sqrt(m->ptnormals[loop][0] * m->ptnormals[loop][0] +
                    m->ptnormals[loop][1] * m->ptnormals[loop][1] + m->ptnormals[loop][2] * m->ptnormals[loop][2]);

    if (n < 0.0001 && n > -0.0001)
      n = 1;

    m->ptnormals[loop][0] /= n;
    m->ptnormals[loop][1] /= n;
    m->ptnormals[loop][2] /= n;
  }
}

void PrintCommandLineOptions(Global_Input & g)
{
  int loop = 0;

  printf("\nGlobal info:\n\n");

  if (g.numgeomfiles != 0)
    printf("  g.numgeomfiles = %d\n", g.numgeomfiles);
  if (g.qnoborders)
    printf("  g.qnoborders = true\n");
  if (!g.qsinglewin)
    printf("  g.qsinglwin = false\n");
  if (g.qnovalidity)
    printf("  g.qnovalidity = true\n");
  if (g.imagefile)
    printf("  g.imagefile = %s\n", g.imagefile);
  if (g.report_level)
    printf("  g.report_level = %ld\n", g.report_level);
  //if (g.dominantsurf)
    //printf("  g.dominantsurf = %ld\n", g.dominantsurf);
  if (g.qabsolute)
    printf("  g.qabsolute = true\n");

  printf("\nGeometry files:\n\n");

  while (loop < g.numgeomfiles) {
    int loop2;
    if (g.SurfList[loop]->geomfilename)
      printf("  geomfilename = %s\n", g.SurfList[loop]->geomfilename);
    for (loop2 = 0; loop2 < (int)g.SurfList[loop]->potfilenames.size(); loop2++) {
      printf("  potfilename = %s\n", g.SurfList[loop]->potfilenames[loop2]);
    }
    if (g.SurfList[loop]->fidfilename)
      printf("  fidfilename = %s\n", g.SurfList[loop]->fidfilename);
    if (g.SurfList[loop]->chfilename)
      printf("  chfilename = %s\n", g.SurfList[loop]->chfilename);
    if (g.SurfList[loop]->clfilename)
      printf("  clfilename = %s\n", g.SurfList[loop]->clfilename);
    if (g.SurfList[loop]->llfilename)
      printf("  llfilename = %s\n", g.SurfList[loop]->llfilename);
    if (g.SurfList[loop]->lmfilename)
      printf("  lmfilename = %s\n", g.SurfList[loop]->lmfilename);
    if (g.SurfList[loop]->parent->geompathname)
      printf("  geompathname = %s\n", g.SurfList[loop]->parent->geompathname);
    if (g.SurfList[loop]->parent->datapathname)
      printf("  datapathname = %s\n", g.SurfList[loop]->parent->datapathname);
    if (g.SurfList[loop]->geomfiletype != -1)
      printf("  geomfiletype = %ld\n", g.SurfList[loop]->geomfiletype);
    if (g.SurfList[loop]->datafiletype != -1)
      printf("  datafiletype = %ld\n", g.SurfList[loop]->datafiletype);
    if (g.SurfList[loop]->geomsurfnum != -1)
      printf("  geomsurfnum = %ld\n", g.SurfList[loop]->geomsurfnum);
    if (g.SurfList[loop]->colour_grad != -1)
      printf("  colour_grad = %ld\n", g.SurfList[loop]->colour_grad);
    if (g.SurfList[loop]->colour_mesh[0] != 255)
      printf("  colour_mesh = %d\n", g.SurfList[loop]->colour_mesh[0]);
    if (g.SurfList[loop]->colour_mesh[1] != 255)
      printf("  colour_mesh = %d\n", g.SurfList[loop]->colour_mesh[1]);
    if (g.SurfList[loop]->colour_mesh[2] != 255)
      printf("  colour_mesh = %d\n", g.SurfList[loop]->colour_mesh[2]);
    if (g.SurfList[loop]->numscalars != -1)
      printf("  numscalars = %ld\n", g.SurfList[loop]->numscalars);
    if (g.SurfList[loop]->scale_lock != -1)
      printf("  scalelock = %ld\n", g.SurfList[loop]->scale_lock);
    if (g.SurfList[loop]->displaysurfnum != -1)
      printf("  displaysurfnum = %ld\n", g.SurfList[loop]->displaysurfnum);
    if (g.SurfList[loop]->ts_end != -1)
      printf("  ts_end = %ld\n", g.SurfList[loop]->ts_end);
    if (g.SurfList[loop]->winxmin)
      printf("  winxmin = %d\n", g.SurfList[loop]->winxmin);
    if (g.SurfList[loop]->winxmax)
      printf("  winxmax = %d\n", g.SurfList[loop]->winxmax);
    if (g.SurfList[loop]->winymin)
      printf("  winymin = %d\n", g.SurfList[loop]->winymin);
    if (g.SurfList[loop]->winymax)
      printf("  winymax = %d\n", g.SurfList[loop]->winymax);
    if (g.SurfList[loop]->numsurfsread)
      printf("  loopread = %ld\n", g.SurfList[loop]->numsurfsread);
    if (g.SurfList[loop]->potusermax)
      printf("  potusermax = %f\n", g.SurfList[loop]->potusermax);
    if (g.SurfList[loop]->potusermin)
      printf("  potusermin = %f\n", g.SurfList[loop]->potusermin);
    if (g.SurfList[loop]->contourstep)
      printf("  contourstep = %f\n", g.SurfList[loop]->contourstep);
    if (g.SurfList[loop]->fidseriesnum)
      printf("  fidseriesnum = %ld\n", g.SurfList[loop]->fidseriesnum);
    if (g.SurfList[loop]->ts_start)
      printf("  ts_start = %ld\n", g.SurfList[loop]->ts_start);
    if (g.SurfList[loop]->timestart)
      printf("  timestart = %f\n", g.SurfList[loop]->timestart);
    if (g.SurfList[loop]->timestep != 1)
      printf("  timestep = %f\n", g.SurfList[loop]->timestep);

    for (loop2 = 0; loop2 < (int)g.SurfList[loop]->scalarInputs.size(); loop2++) {
      printf("  scalar #%d", loop2);
      if (g.SurfList[loop]->scalarInputs[loop2].scalarnum != -1)
        printf("    scalarnum = %ld\n", g.SurfList[loop]->scalarInputs[loop2].scalarnum);
      if (g.SurfList[loop]->scalarInputs[loop2].channelnum != -1)
        printf("    channelnum = %ld\n", g.SurfList[loop]->scalarInputs[loop2].channelnum);
      if (g.SurfList[loop]->scalarInputs[loop2].xmin != -1)
        printf("    xmin = %d\n", g.SurfList[loop]->scalarInputs[loop2].xmin);
      if (g.SurfList[loop]->scalarInputs[loop2].xmax != -1)
        printf("    xmax = %d\n", g.SurfList[loop]->scalarInputs[loop2].xmax);
      if (g.SurfList[loop]->scalarInputs[loop2].ymin != -1)
        printf("    ymin = %d\n", g.SurfList[loop]->scalarInputs[loop2].ymin);
      if (g.SurfList[loop]->scalarInputs[loop2].ymax != -1)
        printf("    ymax = %d\n", g.SurfList[loop]->scalarInputs[loop2].ymax);
    }
    printf("\n");
    loop++;
  }
}
