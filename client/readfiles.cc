/* readfiles.cxx */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdlib.h>

#include "landmarks.h"
#include "readfiles.h"
#include "Surf_Data.h"
#include "Map3d_Geom.h"
#include "MeshList.h"
#include "geomlib.h"
#include "graphicsio.h"
#include "map3dmath.h"
#include "MatlabIO.h"
#include "tsdfcio.h"
#include "dialogs.h"
#include "Contour_Info.h"
#include <math.h>

#ifdef _WIN32
#pragma warning(disable:4172 4514)  /* quiet visual c++ */
#endif


extern Map3d_Info map3d_info;
extern Map3d_Geom *map3d_geom;
extern char *units_strings[5];
extern char datalabels[][100];
extern char geomlabels[][100];
extern Map3d_Info map3d_info;
FileCache file_cache;
using std::string;

Container* FileCache::readContainerFile(std::string str)
{
  if (cache.find(str) != cache.end())
    return (Container*) cache[str];
  Container* c = new Container;
  if (c->Read(str) == -1) {
    delete c;
    c = 0;
    fprintf(stderr, "ERROR: Read of container file %sfailed.\n",str.c_str());
  }
  cache[str] = (void*) c;
  return c;
}

matlabarray* FileCache::readMatlabFile(std::string str)
{
  if (cache.find(str) != cache.end())
    return (matlabarray*) cache[str];
  matlabfile mf(str, "r");
  matlabarray *ma = new matlabarray;
  *ma = mf.getmatlabarray(0);
  cache[str] = (void*) ma;
  return ma;  
}

//Try filename, and then path1/filename, and then path2/filename.  Return a FILE* if the
//  user wants to use it (some files are opened with openfile_), and the filename in out.
//  path1 is the datapath/geompath environment var, and path2 is command-line overriding of
//  those paths.
bool getFileWithPath(char* filename, char* path1, char* path2, char out[])
{
  FILE* luin_p;
  // try original filename
  string file_to_test = filename;
  if ((luin_p = fopen(file_to_test.c_str(), "r")) == NULL) {
    // if it fails, prepend path1
    if (path1)
      file_to_test = string(path1) + '/' + filename;
    if ((luin_p = fopen(file_to_test.c_str(), "r")) == NULL) {
      //if path1/filename fails, prepend path2
      if (path2)
        file_to_test = string(path2) + '/' + filename;
      if ((luin_p = fopen(file_to_test.c_str(), "r")) == NULL) {
        fprintf(stderr,"Could not not file: %s\n",filename);
        strcpy(out, filename);
        return false;
      }
    }
  }
  if (luin_p)
    fclose(luin_p);
  strcpy(out, file_to_test.c_str());
  if (map3d_info.reportlevel >= 2)
    printf("Final filename is %s\n",out);
  return true;
}
long ReadPts(Map3d_Geom* onemap3dgeom)
/*** 
Read the point locations from the already open file
Keep track of the number of points in each part or surface.

This routine also uses dynamic memory allocation to set space
aside for the points array.

***/
{
  long error = 0, insurfnum, numsurfsread = 1;
  char filename[100], basefilename[100];
  Surf_Geom *onesurfgeom = NULL;
  /*******************************************************************/
  
  insurfnum = onemap3dgeom->surfnum;
  if (map3d_info.reportlevel > 1)
    fprintf(stderr, "In ReadPts for surface #%ld\n", insurfnum + 1);
  
  if ((onesurfgeom = AddASurfGeom(onesurfgeom, 0, &numsurfsread)) == NULL)
    exit(ERR_MEM);
  
  /*** Use the standard routine to read the points in. ***/
  
  strcpy(basefilename, onemap3dgeom->basefilename);
  StripExtension(basefilename);
  strcpy(filename, basefilename);
  strcat(filename, ".pts");
    
  strcpy(onesurfgeom->filename, filename);
  
  if (map3d_info.reportlevel)
    fprintf(stderr, "Reading pts file: %s\n", filename);
  
  error = ReadPtsFile(onesurfgeom, map3d_info.reportlevel > 1);
  if (error < 0)
    return (error);
    
  /*** Now transfer stuff between structures. ***/
  
  onemap3dgeom->points[onemap3dgeom->geom_index] = onesurfgeom->nodes;
  onemap3dgeom->numpts = onesurfgeom->numpts;
  onemap3dgeom->channels = onesurfgeom->channels;
  
  //strcpy(onemap3dgeom->label, basefilename);
  return (error);
}


/*=====================================================================*/

long ReadSegs(FILE * luin_p, Map3d_Geom* onemap3dgeom)
/*** Read the segment connectivities from the open file.
This version has dynamic meory allocatiom but no new format
with a line specifying number of lines in the file.

***/
{
  long nfsegs, nsegread, i, j;
  long i1, i2, i3;
  char instring[100];
  bool gotgroups;
  int base = 1;
  /*********************************************************************/
  
  if (fgets(instring, 100, luin_p) == NULL) {
    fprintf(stderr, "**** ReadSegs Error: end of file hit " " at start of file" " ***\n");
    exit(ERR_FILE);
  }
  
  /*** See if we have a group number appended to the triangle
    connectivity. ***/
  
  gotgroups = (((sscanf(instring, " %ld %ld %ld ", &i1, &i2, &i3)) == 3) ? true : false);
  if (gotgroups) {
    fprintf(stderr, " Looks like there are group " "number in the segment" " file\n");
  }
  
  /*** Now rewind the file and see how many segments are in the file. ***/
  
  rewind(luin_p);
  nfsegs = GetFileLength(luin_p);
  rewind(luin_p);
  if (map3d_info.reportlevel > 1)
    fprintf(stderr, " Number of segments in the file should be %ld\n", nfsegs);
  
  /*** Allocate space for the map3dgeom segment array. ***/
  
  long error = onemap3dgeom->SetupMap3dSurfElements(nfsegs, 2);
  if (error < 0) {
    return error;
  }
  
  if (gotgroups) {
    if ((onemap3dgeom->seggroups = (long *)calloc((size_t) nfsegs, sizeof(long))) == NULL) {
      printf("+++ ReadSegs: no memory for point groups\n");
      gotgroups = false;
    }
  }
  
  /*** Read some data finally.  ***/
  
  nsegread = 0;
  if (gotgroups) {
    nsegread = 0;
    rewind(luin_p);
    for (i = 0; i < nfsegs; i++) {
      if (fscanf(luin_p, "%ld %ld %ld\n",
                 &onemap3dgeom->elements[i][0], &onemap3dgeom->elements[i][1], &onemap3dgeom->seggroups[i]) != 3) {
        fprintf(stderr, "***** In ReadSeg, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nfsegs, nsegread);
        exit(ERR_FILE);
      }
      nsegread++;
    }
  }
  else {
    nsegread = 0;
    rewind(luin_p);
    for (i = 0; i < nfsegs; i++) {
      if (fscanf(luin_p, "%ld %ld\n", &onemap3dgeom->elements[i][0], &onemap3dgeom->elements[i][1]) != 2) {
        fprintf(stderr, "***** In ReadSeg, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nfsegs, nsegread);
        exit(ERR_FILE);
      }
      if (onemap3dgeom->elements[i][0] == 0 || onemap3dgeom->elements[i][1] == 0)
        base = 0;
      nsegread++;
    }
  }
  
  /* allow user to specify 0-based files.  Default is 1-based, but 
    if there is a 0 in the file, make the whole file 0-based. 
    If there is a 0, don't subtract 1. */
  
  
  if (base == 1) {
    for (i = 0; i < nfsegs; i++) {
      for (j = 0; j < 3; j++) {
        onemap3dgeom->elements[i][j]--;
      }
    }
  }
  onemap3dgeom->numelements = nfsegs;
  return 0;
}

/*=====================================================================*/

long ReadTris(Map3d_Geom* onemap3dgeom)
/*** Read the triangle connectivities from the open file 
This version uses dynamic memory allocation to read the data
from the file.

Dymanic memory allocation version.
***/
{
  long error = 0, numsurfsread = 1;
  char filename[100], basefilename[100];
  Surf_Geom *onesurfgeom = NULL;
  /*********************************************************************/
    
  if ((onesurfgeom = AddASurfGeom(onesurfgeom, 0, &numsurfsread)) == NULL)
    exit(ERR_MEM);
  
  
  /*** Use the standard routine to read the points in. ***/
  
  strcpy(basefilename, onemap3dgeom->basefilename);
  StripExtension(basefilename);
  strcpy(filename, basefilename);
  strcat(filename, ".fac");
  strcpy(onesurfgeom->filename, filename);
  
  if (map3d_info.reportlevel)
    fprintf(stderr, "Reading tris file %s\n", filename);

  error = ReadFacFile(onesurfgeom, map3d_info.reportlevel > 1);
  if (error < 0)
    return (error);
  
  if (onesurfgeom->elementsize != 3) {
    fprintf(stderr, "**** ReadTri: elements in file are of size %ld\n"
            "**** Filename: %s\n", onesurfgeom->numelements, filename);
    return (ERR_MISC);
  }
  
  /*** Now transfer stuff between structures. ***/
  
  onemap3dgeom->elements = onesurfgeom->elements;
  onemap3dgeom->elementsize = 3;
  onemap3dgeom->numelements = onesurfgeom->numelements;
  onemap3dgeom->maxnumelts = onesurfgeom->numelements;
  return (error);
}

/*================================================================*/

long ReadTetras(FILE * luin_p, Map3d_Geom* onemap3dgeom)
/*** Read the tetrahedral connectivities from the open file 
New format using dynamic memory allocation.

***/
{
  long nftetras, ntetraread, i, j;
  long i1, i2, i3, i4, i5;
  char instring[100];
  bool gotgroups;
  int base = 1;
  /*********************************************************************/
  
  if (fgets(instring, 100, luin_p) == NULL) {
    fprintf(stderr, "**** ReadTetra Error: end of file hit " " at start of file" " ***\n");
    return ERR_FILE;
  }
  
  
  /*** See if we have a group number appended to the tetrahedron
    connectivity. ***/
  
  gotgroups = (((sscanf(instring, " %ld %ld %ld %ld %ld", &i1, &i2, &i3, &i4, &i5)) == 5) ? true : false);
  if (gotgroups) {
    fprintf(stderr, " Looks like there are group number in the tetra" " file\n");
  }
  
  /*** Now rewind the file and see how many tetra are in the file. ***/
  
  rewind(luin_p);
  nftetras = GetFileLength(luin_p);
  rewind(luin_p);
  fprintf(stderr, " Number of tetras in the file should be %ld\n", nftetras);
  
  /*** Allocate space for the map3dgeom tetra array. ***/
  int error = onemap3dgeom->SetupMap3dSurfElements(nftetras, 4);
  if (error < 0) {
    fprintf(stderr, "*** ReadTeteras: error getting memory" " for tetras\n");
    return ERR_MEM;
  }
  
  if (gotgroups) {
    if ((onemap3dgeom->tetragroups = (long *)calloc((size_t) nftetras, sizeof(long))) == NULL) {
      printf("+++ ReadPts: no memory for point groups\n");
      return ERR_MEM;
    }
    printf(" Set up space for %ld group numbers\n", nftetras);
  }
  
  /*** Now read the data from the file.  ***/
  
  ntetraread = 0;
  if (gotgroups) {
    rewind(luin_p);
    for (i = 0; i < nftetras; i++) {
      if (fscanf(luin_p, "%ld %ld %ld %ld %ld",
                 &onemap3dgeom->elements[i][0],
                 &onemap3dgeom->elements[i][1],
                 &onemap3dgeom->elements[i][2], &onemap3dgeom->elements[i][3], &onemap3dgeom->tetragroups[i]) != 5)
      {
        fprintf(stderr, "***** In ReadTetra, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nftetras, ntetraread);
        return ERR_FILE;
      }
      ntetraread++;
    }
  }
  else {
    rewind(luin_p);
    for (i = 0; i < nftetras; i++) {
      if (fscanf(luin_p, "%ld %ld %ld %ld",
                 &onemap3dgeom->elements[i][0],
                 &onemap3dgeom->elements[i][1],
                 &onemap3dgeom->elements[i][2], &onemap3dgeom->elements[i][3]) != 4) {
        fprintf(stderr, "***** In ReadTetra, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nftetras, ntetraread);
        return ERR_FILE;
      }
      if (onemap3dgeom->elements[i][0] == 0 || onemap3dgeom->elements[i][1] == 0 || 
          onemap3dgeom->elements[i][2] == 0 || onemap3dgeom->elements[i][3] == 0)
        base = 0;
      ntetraread++;
    }
  }
  
  /* allow user to specify 0-based files.  Default is 1-based, but 
    if there is a 0 in the file, make the whole file 0-based. 
    If there is a 0, don't subtract 1. */
  
  if (base == 1) {
    for (i = 0; i < nftetras; i++) {
      for (j = 0; j < 4; j++) {
        onemap3dgeom->elements[i][j]--;
      }
    }
  }
  onemap3dgeom->numelements = nftetras;
  return 0;
}

Surf_Data *ReadDataFile(Surf_Input * sinputlist, Surf_Data * surfdata, long numsurfsread, long insurfnum)
{
  /***
Input:
  sinputlist	    structure of all the surface information we need
  surfdata	    the surfdata array for all the surfaces
  numsurfsread    number of surfaces for which we have geometry
  and hence, expect to find data.
  insurfnum	    surface number in which to start storing data
  
Output:	    Loads of externals
Return:	    pointer to updated surf_data array or NULL for error
  
  
  ***********************************************************************/
  long errorlevel, error;
  long filetype, numsurfs, numbsurfs, numseries, numfileframes;
  long seriesnum;
  long framenum, modelframenum;
  long framestart, frameend, framestep, numreadframes;
  long displaysurfnum, surfstart, surfend, surfcount;
  long nodenum, numsurfnodes;
  long numleads, numfileleads;
  long index, maxindex;
  long localunits;
  long reportlevel;
  float potscale;
  float *databuff;
  char pakfilename[80], label[100], csurface[20], cseries[20];
  Boolean qsettings;
  FileInfoPtr luin;
  
  /*********************************************************************/
  reportlevel = map3d_info.reportlevel;
  errorlevel = 1;
  
  if (reportlevel > 1)
    fprintf(stderr, "In ReadDataFile\n Attempting to get data from\n"
            " File: %s\n"
            " Starting at surface #%ld\n", sinputlist->potfilename, insurfnum + 1);
  
  else if (reportlevel > 0)
    fprintf(stderr, "Reading data file %s\n", sinputlist->potfilename);

  if (insurfnum < 0) {
    fprintf(stderr, "*** In ReadDataFile: Error since insurfnum = %ld\n" " Exit here\n", insurfnum);
    exit(ERR_MISC);
  }
  
  // Try an open the data file.  If it fails to find it in the current path,
  // prepend the MAP3D_DATAPATH environment variable
  
  char datafilename[256];
  char* filename = sinputlist->tmpfilename ?sinputlist->tmpfilename:(char *)sinputlist->potfilename;
  if (!getFileWithPath(filename,getenv("MAP3D_DATAPATH"),sinputlist->parent->datapathname,datafilename))
    return NULL;
  
  error = openfile_(datafilename, errorlevel, &luin);
  if (error < 0) {
    fprintf(stderr, "error returned from openfile: %ld\n", error);
    exit(0);
    
  }
  
  /*** Set the datapath if we have one for the data files. ***/
  
  if (sinputlist->parent->datapathname) {
    if (reportlevel > 1)
      fprintf(stderr, " We have an alternate data path!\n" " Path: %s\n", sinputlist->parent->datapathname);
    settimeseriesdatapath_(luin, sinputlist->parent->datapathname);
  }
  
  /*
   if (strlen( sinputlist->geompathname ) > 1)
   {
     if ( reportlevel > 1)
       fprintf(stderr," We have an alternate geometry file path!\n"
               " Path: %s\n", sinputlist->parent->geompathname);
     settimeseriesdatapath_(luin, sinputlist->parent->geompathname);
   }
   */
  
  /*** Get the basic info from the file.  ***/
  
  getfileinfo_(luin, &filetype, &numsurfs, &numbsurfs, &numseries, &qsettings);
  
  if (reportlevel > 1) {
    printf(" Back with the info on the data file\n" " #time series = %ld\n", numseries);
  }
  
  /*** Now select the index of the series we want to look at.
    If we have a tsdf file, then there is only one time series.  ***/
  
  if (sinputlist->datafiletype == TSDF_FILE || numseries == 1) {
    seriesnum = 0;
  }
  else {
    seriesnum = sinputlist->timeseries;
  }
  
  framestart = sinputlist->ts_start;
  frameend = sinputlist->ts_end;
  framestep = sinputlist->ts_sample_step;
  
  /*** If we have not specified a valid set of time series to read,
    then get it interactively from the user.   We have two ways to 
    get this info, TCL and Formslib, depending on options.
    ***/
  
  if (seriesnum < 0) {
    if (reportlevel > 1)
      printf(" We picked the seriesnum as #%ld\n", seriesnum + 1);
    if (seriesnum >= numseries) {
      seriesnum = numseries - 1;
      fprintf(stderr, " Series number returned too high so set it" " at #%ld\n", seriesnum + 1);
    }
  }
  
  /*** Or else we have specified the series in the calling arguments. ***/
  
  else {
    if (reportlevel > 1)
      printf(" Passed index value is #%ld\n", seriesnum + 1);
    
    if (seriesnum < 0) {
      fprintf(stderr, "+++ In readdatafile passed index = %ld is < 1\n" "     So we set it to 1\n", seriesnum + 1);
      seriesnum = 0;
    }
    else if (seriesnum >= numseries) {
      fprintf(stderr, "+++ In readdatafile passed index = %ld is > max"
              " number in file = %ld\n" "     So we set it to %ld\n", seriesnum + 1, numseries, numseries);
      seriesnum = numseries - 1;
    }
  }
  error = settimeseriesindex_(luin, seriesnum + 1);
  if (reportlevel > 2)
    printf(" The result of settimeseriesindex is %ld\n", error);
  if (error < 0)
    return (NULL);
  
  /*** Scaling of potentials ***/
  
  potscale = sinputlist->potscale;
  
  /*** Units ***/
  
  error = gettimeseriesunits_(luin, &localunits);
  if (error < 0) {
    fprintf(stderr, " No unit available for data\n");
    localunits = 0;
    
  }
  else {
    if (localunits > 0 && localunits <= 5) {
      if (map3d_info.reportlevel > 1)
        printf(" Units of the original data series are %s\n", units_strings[localunits - 1]);
      
      /*** Now see if we have applied a user scale factor that we can
      figure out, and see if that affects the units we believe the data
      are in. 
      ***/
      if (fabs(((float)fabs(potscale) - 1.0)) > .0001) {
        /*** If pot scale == 1000. ***/
        if (potscale > 999 && potscale < 1001) {
          if (localunits == UNIT_MVOLTS) {
            localunits = UNIT_UVOLTS;
            fprintf(stderr, " Units changed from milli to micro" "volts by scaling value of %f\n", potscale);
          }
          else if (localunits == UNIT_VOLTS) {
            localunits = UNIT_MVOLTS;
            fprintf(stderr, " Units changed from volts to" "millivolts by scaling value of %f\n", potscale);
          }
          /*** Else if potscale == .001 ***/
        }
        else if (potscale > .00099 && potscale < .0011) {
          if (localunits == UNIT_UVOLTS) {
            localunits = UNIT_MVOLTS;
            fprintf(stderr, " Units changed from micro to milli" "volts by scaling value of %f\n", potscale);
          }
          else if (localunits == UNIT_MVOLTS) {
            localunits = UNIT_VOLTS;
            fprintf(stderr, " Units changed from millivolts to" "volts by scaling value of %f\n", potscale);
          }
        }
      }
    }
    else {
      fprintf(stderr, " Illegal or unknown unit value (%ld)\n", localunits);
      fprintf(stderr, " Set to millivolts\n");
      localunits = UNIT_MVOLTS;
    }
  }
  
  /***  Frame Selection
    
    Set up the frame numbers here.  We may specify them explicitly
    desire to set them interactively based on the power curve. 
    Note that internally, frames begin at 0, while externally,
    they start at 1.
    ***/
  
  error = gettimeseriesspecs_(luin, &numfileleads, &numfileframes);
  if (error < 0)
    return (NULL);
  if (reportlevel > 1)
    printf(" In the data file, number of leads = %ld \n" " number of frames = %ld\n", numfileleads, numfileframes);
  
  /*** If we only have one frame, then start and end frames are
    decided for us. If we have zero or less, we have a problem.
    ***/
  if (numfileframes == 1) {
    framestart = frameend = 0;
    framestep = 1;
  }
  else if (numfileframes <= 0) {
    fprintf(stderr, "*** (readdatafile.c)\n" "*** There are no frames in this datafile\n" "*** So have to quit\n");
    return 0;
  }
  
  /*** Make sure the last frame does not go beyond the end of the file.
    Then make sure frame numbers are OK.
    ***/
  if (frameend >= numfileframes) {
    if (reportlevel)
      fprintf(stderr, "+++ ReadDataFile: Selected last frame = %ld "
              "is too large\n"
              "    The last one in the file is %ld\n" "    so we update here\n", frameend + 1, numfileframes);
    frameend = numfileframes - 1;
  }
  else if (frameend < framestart){
    framestart = 0;
    frameend = numfileframes - 1;
  }
  
  if (framestep <= 0)
    framestep = 1;
  
  if (framestart >= numfileframes) {
    if (reportlevel)
      fprintf(stderr, "+++ ReadDataFile: Selected last frame = %ld "
              "is too large\n"
              "    The last one in the file is %ld\n" "    so we update here\n", frameend + 1, numfileframes);
    framestart = numfileframes - 1;
  }
  
  if (reportlevel > 1)
    printf(" After selecting, first frame is %ld and last is  %ld" " with inc %ld\n", framestart, frameend, framestep);
  
  if (framestart > frameend || framestep < 0) {
    fprintf(stderr, "**** After selecting, first frame is %ld"
            " and last is  %ld"
            " with inc %ld\n" "**** Start frame must be smaller than finish frame\n", framestart, frameend, framestep);
    return NULL;
  }
  
  /*** Now see where we are with all this. ***/
  
  numreadframes = (long) floor ( (double) (frameend - framestart + 1) / framestep + .999 );
  //numreadframes = frameend - framestart + 1;
  //sinputlist->ts_start = framestart;
  //sinputlist->ts_end = frameend;
  //sinputlist->ts_step = framestep;
  
  /*** Allocate the memeory we need.  ***/
  /*** First, the data storage buffer. ***/
  
  databuff = (float *)calloc((size_t) (numfileleads * numfileframes), sizeof(float));
  if (databuff == NULL) {
    fprintf(stderr, "*** In readdatafile.c I cannot get enough" " dynamic memory to buffer the data\n");
    exit(ERR_MEM);
  }
  
  /*** Now set up the memory for the potential data array.
    ***/
  
  if (insurfnum > 0)
    insurfnum--;
  
  surfstart = insurfnum;
  surfend = insurfnum + numsurfsread - 1;
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    if (numreadframes > map3d_info.maxnumframes)
      map3d_info.maxnumframes = numreadframes;
    /*  numleads = pnt_end[displaysurfnum] - pnt_start[displaysurfnum] + 1; */
    numleads = map3d_geom[displaysurfnum].numpts;
    if ((surfdata = Surf_Data::AddASurfData(surfdata, displaysurfnum, numreadframes, numleads)) == NULL)
      return (NULL);
    
    /*** Set up some parameters that we know already. ***/
    
    surfdata[displaysurfnum].units = localunits;
    surfdata[displaysurfnum].ts_start = framestart;
    surfdata[displaysurfnum].ts_end = frameend;
    surfdata[displaysurfnum].ts_sample_step = framestep;
    surfdata[displaysurfnum].user_step = framestep;
    surfdata[displaysurfnum].userpotmin = sinputlist->potusermin;
    surfdata[displaysurfnum].userpotmax = sinputlist->potusermax;
    surfdata[displaysurfnum].potscale = potscale;
    surfdata[displaysurfnum].seriesnum = seriesnum;
    surfdata[displaysurfnum].numseries = numseries;
    
    /******************** Fiducials ***********************/
    
    error = LoadGlobalFids(luin, reportlevel > 1, &surfdata[displaysurfnum].globalfids);
    surfdata[displaysurfnum].fids = NULL;
    
    //    if(sinputlist->containerfilename&&(strlen(sinputlist->containerfilename) != 0)){
    //      surfdata[displaysurfnum].fids = ReadTsdfcFile(sinputlist->containerfilename,
    //						    sinputlist->potfilename,
    //						    seriesnum,&surfdata[displaysurfnum].globalfids,
    //						    surfdata[displaysurfnum].fids,
    //						    &surfdata[displaysurfnum].numfs,
    //						    &surfdata[displaysurfnum].fidmin,
    //						    &surfdata[displaysurfnum].fidmax);
    //
    //    }
    
    /***	
      long fidsetnum = 0;
	surfdata[displaysurfnum].fids = 
      ReadDfileFids( luin, seriesnum, reportlevel, 
                     surfdata[displaysurfnum].fids, 
                     &fidsetnum );
    ***/
  }
  
  /*** Series Label (used for display later) ***/
  
  gettimeseriesfile_(luin, pakfilename);
  sprintf(cseries, "Series %ld: ", seriesnum + 1);
  if (gettimeserieslabel_(luin, label) != 0 && Trulen(pakfilename) > 2) {
    strcat(label, "Pak File: ");
    strcat(label, pakfilename);
  }
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    strcpy(surfdata[displaysurfnum].label, cseries);
    sprintf(csurface, ": Surface %ld", surfdata[displaysurfnum].surfnum + 1);
    strcat(surfdata[displaysurfnum].label, label);
    strcat(surfdata[displaysurfnum].label, csurface);
    if (reportlevel > 2)
      fprintf(stderr, " For surface #%ld the label is %s\n",
              surfdata[displaysurfnum].surfnum + 1, surfdata[displaysurfnum].label);
    if (sinputlist->parent->datapathname)
      strcpy(surfdata[displaysurfnum].filepath, sinputlist->parent->datapathname);
    strcpy(surfdata[displaysurfnum].potfilename, sinputlist->potfilename);
  }
  
  /*** Get the data buffer. ***/
  
  error = gettimeseriesdata_(luin, databuff);
  if (error < 0) {
    fprintf(stderr, "In readdatafile.c the result of getting" " time series data is %ld\n", error);
    exit(ERR_FILE);
  }
  
  if (reportlevel > 2) {
    printf(" Back with the data in the buffer\n");
    printf(" #leads = %ld #frames = %ld \n", numfileleads, numfileframes);
    printf(" #frames selected to run from %ld to %ld\n", framestart, frameend);
  }
  
  /*** Now move the data from the buffer to the proper locations in the
    data array, using indirection via the channels array. 
    ***/
  
  maxindex = numfileleads * numfileframes - 1;
  surfcount = 0;
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    numsurfnodes = map3d_geom[displaysurfnum].numpts;
    
    for (framenum = framestart, modelframenum = 0; framenum <= frameend;
         framenum += framestep, modelframenum++) {
      for (nodenum = 0; nodenum < numsurfnodes; nodenum++) {
        if (map3d_geom[displaysurfnum].channels[nodenum] > -1) {
          index = framenum * numfileleads + map3d_geom[displaysurfnum].channels[nodenum];
          if (index > maxindex || index < 0) {
            fprintf(stderr, "*** MAP3D ERROR: In readdatafile\n"
                    "    For surface #%ld \n"
                    "    Data buffer index points to %ld "
                    "in data buffer\n"
                    "    but max size is %ld\n",
                    surfdata[displaysurfnum].surfnum + 1, index, maxindex);
            fprintf(stderr, " framenum = %ld, numberofframes = %ld\n"
                    " numberofleads = %ld, channelnum = %ld\n"
                    " numberofnodes = %ld, nodenum = %ld\n",
                    framenum, numfileframes, numfileleads,
                    map3d_geom[displaysurfnum].channels[nodenum], numsurfnodes, nodenum);
            return (NULL);
          }
          surfdata[displaysurfnum].potvals[modelframenum][nodenum] = databuff[index] * potscale;
        }
        else {
          surfdata[displaysurfnum].potvals[modelframenum][nodenum] = UNUSED_DATA;
        }
      }
    }
    surfcount++;
  }
  
  /*** Clean up and get back to calling routine.  ***/
  
  free(databuff);
  closefile_(luin);
  return (surfdata);
}

/*=====================================================================

NOTE: This function right now is in a state to only open 1 matlab time series at a time
i.e., numsurfsread = 1.
This is fine, as right now map3d only reads one series at a time.  To change this
later, you'll need to move the majority of the code into one for loop (instead of all of the
                                                                       for (displaysurfnum ...) loops right before the retrieval of units. */
Surf_Data *ReadMatlabDataFile(Surf_Input * sinputlist, Surf_Data * surfdata, long numsurfsread, long insurfnum)
{
  /***
Input:
  sinputlist	    structure of all the surface information we need
  surfdata	    the surfdata array for all the surfaces
  numsurfsread    number of surfaces for which we have geometry
  and hence, expect to find data.
  insurfnum	    surface number in which to start storing data
  
Output:	    Loads of externals
Return:	    pointer to updated surf_data array or NULL for error
  
  
  ***********************************************************************/
  long errorlevel;
  long numseries = 1, numfileframes = 0;
  long framenum, modelframenum;
  long framestart, frameend, framestep, numreadframes;
  long displaysurfnum, surfstart, surfend, surfcount;
  long nodenum, numsurfnodes;
  long numleads, numfileleads = 0;
  long index, maxindex;
  long localunits = 0;
  long reportlevel;
  float potscale;
  float *databuff;
  char  clabel[100], csurface[20], cseries[20];
  
  clabel[0] = '\0'; // init it in case there is no label in the matlab file
  matlabarray unit;
  matlabarray label;
  matlabarray potvals;
  matlabarray cell;
  matlabarray fids;

  /*********************************************************************/
  reportlevel = map3d_info.reportlevel;
  errorlevel = 1;
  
  
  
  if (reportlevel)
    fprintf(stderr, "Reading matlab file %s, series #%ld\n", sinputlist->potfilename, insurfnum+1);
  
  if (insurfnum < 0) {
    fprintf(stderr, "*** In ReadMatlabDataFile: Error since insurfnum = %ld\n" " Exit here\n", insurfnum);
    return NULL;
  }
  
  // Try an open the data file.  If it fails to find it in the current path,
  // prepend the MAP3D_DATAPATH environment variable
  
  char datafilename[256];
  char* filename = (char *)sinputlist->potfilename;
  
  if (!getFileWithPath(filename,getenv("MAP3D_DATAPATH"),sinputlist->parent->datapathname,datafilename)) {
    fprintf(stderr, "Could not locate file: %s", filename);
    return NULL;
  }
  
  matlabfile mf;
  matlabarray ma;
  std::string name;
  
  // open the matlab file and get the root array
  mf.open(datafilename, "r");
  ma = mf.getmatlabarray(0);
  name = ma.getname();
  
  /*** Set the datapath if we have one for the data files. ***/
  
  if (sinputlist->parent->datapathname) {
    if (reportlevel > 1)
      fprintf(stderr, " We have an alternate data path!\n" " Path: %s\n", sinputlist->parent->datapathname);
  }
  
  /*** Get the basic info from the file.  ***/
  
  if (ma.isstruct()) {
    numseries = ma.getnumelements();
    if (ma.isfieldCI("unit")) unit = ma.getfieldCI(insurfnum,"unit");
    if (ma.isfieldCI("label")) label = ma.getfieldCI(insurfnum,"label");
    if (ma.isfieldCI("potvals")) potvals = ma.getfieldCI(insurfnum,"potvals");
    if (ma.isfieldCI("data")) potvals = ma.getfieldCI(insurfnum,"data");
    if (ma.isfieldCI("field")) potvals = ma.getfieldCI(insurfnum,"field");
    if (ma.isfieldCI("scalarfield")) potvals = ma.getfieldCI(insurfnum,"scalarfield");
    if (ma.isfieldCI("fids")) fids = ma.getfieldCI(insurfnum,"fids");

  }
  else if (ma.iscell()) {
    numseries = ma.getnumelements();
    cell = ma.getcell(insurfnum);
    if (cell.isfieldCI("unit")) unit = cell.getfieldCI(0,"unit");
    if (cell.isfieldCI("label")) label = cell.getfieldCI(0,"label");
    if (cell.isfieldCI("potvals")) potvals = cell.getfieldCI(0,"potvals");
    if (cell.isfieldCI("data")) potvals = cell.getfieldCI(0,"data");
    if (cell.isfieldCI("field")) potvals = cell.getfieldCI(0,"field");
    if (cell.isfieldCI("scalarfield")) potvals = cell.getfieldCI(0,"scalarfield");
    if (cell.isfieldCI("fids")) fids = cell.getfieldCI(0,"fids");
  }
  else if (ma.isdense()) { 
    potvals = ma;
  }
  
  if (reportlevel) {
    printf(" Back with the info on the data file\n" " number of time series available = %ld\n", numseries);
  }
  
  /*** Now select the index of the series we want to look at.
    If we have a tsdf file, then there is only one time series.  ***/
  
  framestart = sinputlist->ts_start;
  frameend = sinputlist->ts_end;
  framestep = sinputlist->ts_sample_step;
  
  if (reportlevel > 1)
    printf(" Passed index value is #%ld\n", insurfnum);
  
  /*** Scaling of potentials ***/
  
  potscale = sinputlist->potscale;
  
  if (!unit.isempty() && unit.isstring())
  {
    localunits = 0;
    if (unit.compareCI("mv")) localunits = UNIT_MVOLTS;
    if (unit.compareCI("uv")) localunits = UNIT_UVOLTS;
    if (unit.compareCI("V")) localunits = UNIT_VOLTS;
  }
  
  if (!label.isempty() && label.isstring()) {
    strcpy(clabel, label.getstring().c_str());
  }
  
  
  
  /***  Frame Selection
    
    Set up the frame numbers here.  We may specify them explicitly
    desire to set them interactively based on the power curve. 
    Note that internally, frames begin at 0, while externally,
    they start at 1.
    ***/
  surfstart = insurfnum;
  surfend = insurfnum + numsurfsread - 1; 
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    numfileframes = 0;
    numfileleads = 0;
    
    if (!potvals.isempty()) {
      numfileleads = potvals.getm();
      numfileframes = potvals.getn();
      
      if((numfileleads != map3d_geom[displaysurfnum].numpts)&&
         (numfileframes == map3d_geom[displaysurfnum].numpts)){
        potvals.transpose();
        numfileleads = potvals.getm();
        numfileframes = potvals.getn();
      }
    }
    else {
      fprintf(stderr, "Cannot find potential data in file %s\n", sinputlist->potfilename);
      fprintf(stderr, "Use matlab matrix with name 'potvals', 'data', 'field', or 'scalarfield'\n");
      return 0;
    }
  }
  if (reportlevel > 1)
    printf(" In the data file, number of leads = %ld \n" " number of frames = %ld\n", numfileleads, numfileframes);
  
  /*** If we only have one frame, then start and end frames are
    decided for us. If we have zero or less, we have a problem.
    ***/
  if (numfileframes == 1) {
    framestart = frameend = 0;
    framestep = 1;
  }
  else if (numfileframes <= 0) {
    fprintf(stderr, "*** (ReadMatlabDataFile)\n" "*** There are no frames in this datafile\n" "*** So have to quit\n");
    return 0;
  }
  else if (frameend < framestart){
    framestart = 0;
    frameend = numfileframes - 1;
  }
  
  if (framestep <= 0)
    framestep = 1;
  
  
  /*** Make sure the last frame does not go beyond the end of the file.
    Then make sure frame numbers are OK.
    ***/
  if (frameend >= numfileframes) {
    if (reportlevel)
      fprintf(stderr, "+++ ReadMatlabDataFile: Selected last frame = %ld "
              "is too large\n"
              "    The last one in the file is %ld\n" "    so we update here\n", frameend + 1, numfileframes);
    frameend = numfileframes - 1;
  }
  
  if (framestart >= numfileframes) {
    if (reportlevel)
      fprintf(stderr, "+++ ReadMatlabDataFile: Selected last frame = %ld "
              "is too large\n"
              "    The last one in the file is %ld\n" "    so we update here\n", frameend + 1, numfileframes);
    framestart = numfileframes - 1;
  }
  
  if (reportlevel > 1)
    printf(" After selecting, first frame is %ld and last is  %ld" " with inc %ld\n", framestart, frameend, framestep);
  
  if (framestart > frameend || framestep < 0) {
    fprintf(stderr, "**** After selecting, first frame is %ld"
            " and last is  %ld"
            " with inc %ld\n" "**** Start frame must be smaller than finish frame\n", framestart, frameend, framestep);
    return 0;
  }
  
  /*** Now see where we are with all this. ***/
  //numreadframes = frameend - framestart + 1;
  numreadframes = (long) floor ( (double) (frameend - framestart + 1) / framestep + .999 );
  
  /*** Allocate the memeory we need.  ***/
  /*** First, the data storage buffer. ***/
  databuff = (float *)calloc((size_t) (numfileleads * numfileframes), sizeof(float));
  
  if (databuff == NULL) {
    fprintf(stderr, "*** In ReadMatlabDataFile I cannot get enough" " dynamic memory to buffer the data\n");
    return 0;
  }
  
  /*** Now set up the memory for the potential data array.
    ***/
  
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    if (numreadframes > map3d_info.maxnumframes)
      map3d_info.maxnumframes = numreadframes;
    /*  numleads = pnt_end[displaysurfnum] - pnt_start[displaysurfnum] + 1; */
    numleads = map3d_geom[displaysurfnum].numpts;
    if ((surfdata = Surf_Data::AddASurfData(surfdata, displaysurfnum, numreadframes, numleads)) == NULL) {
      fprintf(stderr, "Failed to allocate memory for data file: ", datafilename);
      return (NULL);
    }
    
    /*** Set up some parameters that we know already. ***/
    
    surfdata[displaysurfnum].setUnits(localunits);
    surfdata[displaysurfnum].ts_start = framestart;
    surfdata[displaysurfnum].ts_end = frameend;
    surfdata[displaysurfnum].ts_sample_step = framestep;
    surfdata[displaysurfnum].user_step = framestep;
    surfdata[displaysurfnum].userpotmin = sinputlist->potusermin;
    surfdata[displaysurfnum].userpotmax = sinputlist->potusermax;
    surfdata[displaysurfnum].potscale = potscale;
    surfdata[displaysurfnum].seriesnum = insurfnum;
    surfdata[displaysurfnum].numseries = numseries;
    
#pragma mark Matlab Fiducials
    /******************** Fiducials ***********************/
    if (!fids.isempty())
    {
      //printf("Start reading matlab fiducials\n");
      float qtime = 1, stime = 1, ttime = 1, pontime = 1, pofftime = 1, rpeaktime = 1;
      float tpeaktime = 1, stofftime = 1;
      long numgfids;
      int numfidsets = 0;
      surfdata[displaysurfnum].numfs = 1;
      numsurfnodes = map3d_geom[displaysurfnum].numpts;


      //set up global fids memory
      numgfids = 8;
      if (( surfdata[displaysurfnum].globalfids.fidtypes = (short *) calloc( (size_t) numgfids, sizeof( short ))) == NULL)
      {
        ReportError( "ReadMatlabDatafile", "error getting basic fid mem", 
                     ERR_MEM, "");
        return( NULL );
      }
      if (( surfdata[displaysurfnum].globalfids.fidvals = (float *) calloc( (size_t) numgfids, sizeof( float ))) == NULL)
      {
        ReportError( "ReadMatlabDatafile", "error getting basic fid mem", 
                     ERR_MEM, "");
        return( NULL );
      }
      
      long numfids = 1;// fids.getnumelements();
      if ( surfdata[displaysurfnum].fids == NULL )
      {
        if (( surfdata[displaysurfnum].fids = (Series_Fids *) calloc( (size_t) numfids, 
                                                   sizeof( Series_Fids ) )) == NULL ) 
        {
          char errtext[80];
          sprintf(errtext," Tried to open %d fidsets", numfids);
          ReportError( "ReadMatlabDataFile", "error getting big memory file",
                       ERR_MEM, errtext );
          return( NULL );
        }
      }
      else 
      { 
        if (( surfdata[displaysurfnum].fids = (Series_Fids *) realloc( surfdata[displaysurfnum].fids, (size_t) 
                                                    (numfids) * sizeof( Series_Fids ) )) == NULL ) 
        {
          ReportError( "ReadMatlabDataFile", "error expanding big memory file",
                       ERR_MEM, "" );
          return( NULL );
        }
      }
      
      //set up series fids memory
      if (( surfdata[displaysurfnum].fids[0].leadfids = (Lead_Fids *) 
            calloc( (size_t)  numsurfnodes, sizeof( Lead_Fids ) )) == NULL )
      {
        ReportError( "ReadMatlabDatafile", 
                     "error getting leadfids memory", ERR_MEM, "" );
        return( NULL );
      }
      
      //load the seriesfids structure
      surfdata[displaysurfnum].fids[0].tsnum = insurfnum;
      surfdata[displaysurfnum].fids[0].pakfilenum = -1;
      surfdata[displaysurfnum].fids[0].numfidleads = numsurfnodes;
      surfdata[displaysurfnum].fids[0].numfidtypes = fids.getnumelements();
      //strcpy( surfdata[displaysurfnum].fids[0].fidlabel, myFS->name.c_str());
      
      //set up series fidnames memory
      if (( surfdata[displaysurfnum].fids[0].fidnames = (char**) 
            calloc( (size_t) fids.getnumelements(), sizeof( char* ) )) == NULL )
      {
        ReportError( "ReadMatlabDatafile", 
                     "error getting fidnames memory", ERR_MEM, "" );
        return( NULL );
      }
      if (( surfdata[displaysurfnum].fids[0].fidtypes =
            (short *) calloc( (size_t) fids.getnumelements(), sizeof( short )) ) == NULL )
      {
        ReportError( "ReadMatlabDatafile", "error getting fidtypes memory",
                     ERR_MEM, "" );
        return( NULL );
      }
      
      
      matlabarray value;
      matlabarray type;
      FI_Init(1);
      
      for(int numFidTypes = 0; numFidTypes < fids.getnumelements(); numFidTypes++){
        FiducialInfo* tmp = (FiducialInfo*) malloc(sizeof(FiducialInfo));
        
        type = fids.getfieldCI(numFidTypes,"type");
        std::vector<short> vtype;
        type.getnumericarray(vtype);
        
        tmp->type = vtype[0];

        surfdata[displaysurfnum].fids[0].fidtypes[numFidTypes] = vtype[0];
        if(FI_GetInfoByType(tmp)){
          if ((surfdata[displaysurfnum].fids[0].fidnames[numFidTypes] =
               (char *)calloc((size_t) strlen(tmp->name)+1, sizeof(char))) == NULL) {
            ReportError("ReadMatlabDatafile", "error getting fidnames memory", ERR_MEM, "");
            return (NULL);
          }
          strcpy( surfdata[displaysurfnum].fids[0].fidnames[numFidTypes],tmp->name);
        }

      }
      
      
      for(long leadnum = 0; leadnum <numsurfnodes; leadnum++){
        index = map3d_geom[displaysurfnum].channels[leadnum];
        maxindex = numfileleads - 1;
        if (index > maxindex || index < 0) {
          fprintf(stderr, "*** In ReadMatlabDataFile(fiducials)\n"
                  "    For surface #%ld \n"
                  "    Data buffer index points to %ld "
                  "in data buffer\n" "    but max size is %ld\n", displaysurfnum + 1, index, maxindex);
          fprintf(stderr, " numberofnodes = %ld, nodenum = #%ld"
                  " channel = #%ld\n", numsurfnodes, nodenum + 1, map3d_geom[displaysurfnum].channels[nodenum] + 1);
          return (0);
        }

        //printf("index %d leadnum %d\n", index, leadnum);
        surfdata[displaysurfnum].fids[0].leadfids[leadnum].leadnum = leadnum;
        surfdata[displaysurfnum].fids[0].leadfids[leadnum].numfids = 0;
        std::vector<int> whichfid;

        for(int numFidTypes = 0; numFidTypes < fids.getnumelements(); numFidTypes++){
          value = fids.getfieldCI(numFidTypes,"value");
          std::vector<double> vvalue;
          value.getnumericarray(vvalue);
          if(index <= vvalue.size()){
            whichfid.push_back(numFidTypes);
            surfdata[displaysurfnum].fids[0].leadfids[leadnum].numfids++;
          }
        }
        //printf("Lead %d numfids %d\n", leadnum,surfdata[displaysurfnum].fids[0].leadfids[leadnum].numfids);
        
        if (( surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidtypes =
              (short *) calloc( (size_t) surfdata[displaysurfnum].fids[0].leadfids[leadnum].numfids, sizeof( short )) ) == NULL )
        {
          ReportError( "ReadMatlabDatafile", "error getting fidtypes memory",
                       ERR_MEM, "" );
          return( NULL );
        }
        
        if (( surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidvals =
              (float *) calloc( (size_t) surfdata[displaysurfnum].fids[0].leadfids[leadnum].numfids, sizeof( float )) )== NULL )
        {
          ReportError( "ReadMatlabDatafile", "error getting fidvals memory",
                       ERR_MEM, "" );
          return( NULL );
        }
        
        for(int fidnum = 0; fidnum < surfdata[displaysurfnum].fids[0].leadfids[leadnum].numfids; fidnum++){
          value = fids.getfieldCI(whichfid[fidnum],"value");
          type = fids.getfieldCI(whichfid[fidnum],"type");
          std::vector<long> vtype;
          std::vector<double> vvalue;

          type.getnumericarray(vtype);
          value.getnumericarray(vvalue);
          
          //series fids
          surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidtypes[fidnum] = vtype[0];
          surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidvals[fidnum] = vvalue[leadnum];
          
//          printf("fid type %d fid value %f\n", surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidtypes[fidnum],
//                 surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidvals[fidnum]);
            
          //get min-max fids
          if( surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidvals[fidnum] > surfdata[displaysurfnum].fidmax)
            surfdata[displaysurfnum].fidmax =  surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidvals[fidnum];
          if( surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidvals[fidnum] < surfdata[displaysurfnum].fidmin)
            surfdata[displaysurfnum].fidmin =  surfdata[displaysurfnum].fids[0].leadfids[leadnum].fidvals[fidnum];
          
          //global fids
          switch(vtype[0]){
            case 0:
              pontime = vvalue[leadnum];
              break;
            case 1:
              pofftime = vvalue[leadnum];
              break;
            case 2:
              qtime = vvalue[leadnum];
              break;
            case 3:
              rpeaktime = vvalue[leadnum];
              break;
            case 4:
              stime = vvalue[leadnum];
              break;
            case 5:
              stofftime = vvalue[leadnum];
              break;
            case 6:
              tpeaktime = vvalue[leadnum];
              break;
            case 7:
              ttime = vvalue[leadnum];
              break;
          }
        }
      }
      
      
      surfdata[displaysurfnum].globalfids.numfids = numgfids;
      surfdata[displaysurfnum].globalfids.leadnum = -1;
      surfdata[displaysurfnum].globalfids.fidtypes[0] = FI_PON;
      surfdata[displaysurfnum].globalfids.fidvals[0] = (float) pontime - 1.f;
      surfdata[displaysurfnum].globalfids.fidtypes[1] = FI_POFF;
      surfdata[displaysurfnum].globalfids.fidvals[1] = (float) pofftime - 1.f;
      surfdata[displaysurfnum].globalfids.fidtypes[2] = FI_QON;
      surfdata[displaysurfnum].globalfids.fidvals[2]   = (float) qtime - 1.f;
      surfdata[displaysurfnum].globalfids.fidtypes[3] = FI_RPEAK;
      surfdata[displaysurfnum].globalfids.fidvals[3]    = (float) rpeaktime - 1.f;
      surfdata[displaysurfnum].globalfids.fidtypes[4] = FI_SOFF;
      surfdata[displaysurfnum].globalfids.fidvals[4]   = (float) stime - 1.f;
      surfdata[displaysurfnum].globalfids.fidtypes[6] = FI_STOFF;
      surfdata[displaysurfnum].globalfids.fidvals[6]    = (float) stofftime - 1.f;
      surfdata[displaysurfnum].globalfids.fidtypes[6] = FI_TPEAK;
      surfdata[displaysurfnum].globalfids.fidvals[6]    = (float) tpeaktime - 1.f;
      surfdata[displaysurfnum].globalfids.fidtypes[7] = FI_TOFF;
      surfdata[displaysurfnum].globalfids.fidvals[7]   = (float) ttime - 1.f;
      
      if ( map3d_info.reportlevel > 1 ){
        printf(" Some global fidicials from the file\n"
               " qframe   = %f\n"
               " ponframe = %f\n"
               " rpeak    = %f\n",
               surfdata[displaysurfnum].globalfids.fidvals[2],
               surfdata[displaysurfnum].globalfids.fidvals[0],
             surfdata[displaysurfnum].globalfids.fidvals[3]);
      }
    }
      
//      if (!fids.isfieldCI("value"))
//      {  return; /* ERROR INPROPER FORMAT*/}
//      if (!fids.isfieldCI("type"))
//      {  return; /* ERROR INPROPER FORMAT*/}
//      
//      matlabarray value;
//      matlabarray type;
//      
//      std::vector<double> vvalue;
//      std::vector<long> vtype;
//      
//      for (long p=0;p<numfids;p++)
//      {
//        value = fids.getfieldCI(p,"value");
//        type = fids.getfieldCI(p,"type");
//        
//        type.getnumericarray(vtype);
//        value.getnumericarray(vvalue);
//                    
//        if (vtype.size() < 1) continue;
//        switch(vtype[0])
//        {
//          case 0 :
//          {	// P-onset
//            //Do something with vvalue which has all the fiducials				
//          }
//          case 1 :
//          {
//            
//            
//          }
//            
//        }
//        
//      }
//      
//    }
    
    // error = LoadGlobalFids(luin, reportlevel, &surfdata[displaysurfnum].globalfids);
    //     surfdata[displaysurfnum].fids = NULL;
    //     /***	
    // 	long fidsetnum = 0;
    // 	surfdata[displaysurfnum].fids = 
    // 	ReadDfileFids( luin, seriesnum, reportlevel, 
    // 	surfdata[displaysurfnum].fids, 
    // 	&fidsetnum );
    //     ***/
  }
  
  /*** Series Label (used for display later) ***/
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    sprintf(cseries, "Series %ld: ", insurfnum + 1);
    strcpy(surfdata[displaysurfnum].label, cseries);
    sprintf(csurface, ": Surface %ld", surfdata[displaysurfnum].surfnum + 1);
    strcat(surfdata[displaysurfnum].label, clabel);
    strcat(surfdata[displaysurfnum].label, csurface);
    
    // short display name - put as series number if label is not in file
    if (strlen(clabel) == 0)
      sprintf(clabel, "%d", insurfnum+1);
    strcpy(surfdata[displaysurfnum].shortlabel, clabel);
    
    if (reportlevel > 2)
      fprintf(stderr, " For surface #%ld the label is %s\n",
              surfdata[displaysurfnum].surfnum + 1, surfdata[displaysurfnum].label);
    if (sinputlist->parent->datapathname)
      strcpy(surfdata[displaysurfnum].filepath, sinputlist->parent->datapathname);
    strcpy(surfdata[displaysurfnum].potfilename, sinputlist->potfilename);
  }
  
  /*** Get the data buffer. ***/
  if (!potvals.isempty()) potvals.getnumericarray(databuff, potvals.getnumelements());
  
  
  if (reportlevel > 2) {
    printf(" Back with the data in the buffer\n");
    printf(" #leads = %ld #frames = %ld \n", numfileleads, numfileframes);
    printf(" #frames selected to run from %ld to %ld\n", framestart, frameend);
  }
  
  /*** Now move the data from the buffer to the proper locations in the
    data array, using indirection via the channels array. 
    ***/
  
  maxindex = numfileleads * numfileframes -1;
  surfcount = 0;
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    numsurfnodes = map3d_geom[displaysurfnum].numpts;
    
    for (framenum = framestart, modelframenum = 0; framenum <= frameend;
         framenum += framestep, modelframenum++) {
      for (nodenum = 0; nodenum < numsurfnodes; nodenum++) {
        if (map3d_geom[displaysurfnum].channels[nodenum] > -1) {
          index = framenum * numfileleads + map3d_geom[displaysurfnum].channels[nodenum];
          if (index > maxindex || index < 0) {
            fprintf(stderr, "*** MAP3D ERROR: In ReadMatlabDataFile\n"
                    "    For surface #%ld \n"
                    "    Data buffer index points to %ld "
                    "in data buffer\n"
                    "    but max size is %ld\n",
                    surfdata[displaysurfnum].surfnum + 1, index, maxindex);
            fprintf(stderr, " framenum = %ld, numberofframes = %ld\n"
                    " numberofleads = %ld, channelnum = %ld\n"
                    " numberofnodes = %ld, nodenum = %ld\n",
                    framenum, numfileframes, numfileleads,
                    map3d_geom[displaysurfnum].channels[nodenum], numsurfnodes, nodenum);
            return (NULL);
          }
          surfdata[displaysurfnum].potvals[modelframenum][nodenum] = databuff[index] * potscale;
        }
        else {
          surfdata[displaysurfnum].potvals[modelframenum][nodenum] = UNUSED_DATA;
        }
      }
    }
    surfcount++;
  }
  
  /*** Clean up and get back to calling routine.  ***/
  
  free(databuff);
  return (surfdata);
  
  
}

/*======================================================================*/



Surf_Data *ReadGradFiles(Surf_Input * sinputlist, Surf_Data * surfdata,
                         Map3d_Geom * map3dgeom, long numsurfsread, long insurfnum)
{
  /***
  A routine to read one or more gradient files, which contain vectors.
  
Input:
  sinputlist   	the structure of information for this input
  or entry surface. We get lots of stuff from here.
  surfdata	    	the surfdata array for all the surfaces
  numsurfsread 	the number of display surfaces that were read for 
  this entry surface.  If this is more than 1,
  we have to re-read the same grad files
  insurfnum	number of display surfaces at which to start
  loading new gradients.
  ***/
  long surfcount;
  long framenum;
  long numframes;
  long numleads;
  long displaysurfnum; /*** Display surface into which we put grad data ***/
  long numgradsurfs; /*** NUmber of surfaces worth of gradient data read ***/
  long filenum, filenum1, filenum2, filestep;
  char filename[80], basefilename[80];
  FILE *luin_p;
  /********************************************************************/
  if (map3d_info.reportlevel > 2)
    fprintf(stderr, "In ReadGradFiles\n");
  
  filenum1 = sinputlist->ts_start;
  filenum2 = sinputlist->ts_end;
  filestep = sinputlist->ts_sample_step;
  if (filenum2 < filenum1)
    filenum2 = filenum1;
  if (filestep < 0)
    filestep = 1;
  strcpy(basefilename, sinputlist->potfilename);
  StripExtension(basefilename);
  
  /*** Loop through all the surfaces read for this entry surface. 
    ***/
  
  numgradsurfs = 0;
  for (surfcount = 0; surfcount < numsurfsread; surfcount++) {
    displaysurfnum = insurfnum + surfcount;
    
    /*** Set up the sequence of files that we want to read in for this
    surface.
    ***/
    
    framenum = 0;
    for (filenum = filenum1; filenum <= filenum2; filenum += filestep) {
      
      /*** Make the .grad filename and open the file. ***/
      
      if (filenum2 > filenum1) {
        sprintf(filename, "%s%-3.3ld", basefilename, filenum);
      }
      else {
        strcpy(filename, basefilename);
      }
      strcat(filename, ".grad");
      
      if ((luin_p = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "*** Grad file %s not " "found ***\n", filename);
        return (NULL);
      }
      
      /*** Go read some data from the file. ***/
      
      /*** Allocate some memory for the new data. ***/
      
      if (filenum == filenum1) {
        
        /*** See if the surface already exists - if not, we have to add one. ***/
        
        if (&surfdata[displaysurfnum]) {
          numleads = surfdata[displaysurfnum].numleads;
          numframes = surfdata[displaysurfnum].numframes;
        }
        else {
          numleads = map3dgeom[displaysurfnum].numpts;
          numframes = (long)((filenum2 - filenum1) / filestep + 1);
          if (numframes > map3d_info.maxnumframes)
            map3d_info.maxnumframes = numframes;
          if ((surfdata = Surf_Data::AddASurfData(surfdata, displaysurfnum, numframes, numleads)) == NULL)
            return (NULL);
        }
        
        /*** Now get the memory for the gradframes for this surface. ***/
        
        if (surfdata[displaysurfnum].gradframes != NULL)
          free(surfdata[displaysurfnum].gradframes);
        if ((surfdata[displaysurfnum].gradframes = (Grad_Frame *)
             calloc(numframes, sizeof(Grad_Frame))) == NULL) {
          fprintf(stderr, "ReadGradFiles: No memory available\n");
          exit(ERR_MEM);
        }
      }
      
      if (map3d_info.reportlevel > 2)
        fprintf(stderr, " Reading gradients into display" " surface #%ld\n", displaysurfnum + 1);
      ReadOneGradFile(luin_p, &surfdata[displaysurfnum], framenum);
      fclose(luin_p);
      framenum++;
    }
    surfdata[displaysurfnum].numframes = framenum;
    surfdata[displaysurfnum].qgotgrads = true;
    numgradsurfs++;
    if (strlen(sinputlist->parent->datapathname) > 1)
      strcpy(surfdata[displaysurfnum].filepath, sinputlist->parent->datapathname);
    strcpy(surfdata[displaysurfnum].potfilename, sinputlist->potfilename);
  }
  return surfdata;
}

long ReadOneGradFile(FILE * luin_p, Surf_Data * onesurfdata, long lframenum)
/*** Read the current (potential gradient) vector from
an already open file. The first point is also the location of 
the centroid of the finite element, the second is the length of
the vector's components. So the second endpoint is always the first
point + second point.
So read in 6 real values: x1, y2, z2, diffx, diffy, diffz.

Input:
luin_p	pointer to open file
onesurfdata	a pointer to the a single surf data structure
lframenum   local frame number

***/
{
  long i;
  long nfgrads, ngradread;
  long maxnumgrads;
  Vector *gradp_p, *gradvec_p;
  Arrow *arrows_p;
  /*****************************************************************/
  if (lframenum < 0) {
    fprintf(stderr, "In ReadGrad, error since frame is < 0\n");
    exit(ERR_MISC);
  }
  
  maxnumgrads = onesurfdata->maxnumgrads;
  onesurfdata->gradframes[lframenum].framenum = lframenum;
  
  /*** See how many values the file has. ***/
  
  nfgrads = GetFileLength(luin_p);
  rewind(luin_p);
  
  /*** And allocate some memory based on that information. ***/
  
  if ((gradp_p = (Vector *) calloc((size_t) nfgrads, sizeof(Vector))) == NULL) {
    fprintf(stderr, "ReadOneGradFile: No memory available fo gradp_p\n");
    exit(ERR_MEM);
  }
  onesurfdata->gradframes[lframenum].gradp = gradp_p;
  
  if ((gradvec_p = (Vector *) calloc((size_t) nfgrads, sizeof(Vector))) == NULL) {
    fprintf(stderr, "ReadOneGradFile: No memory available for gradvec_p\n");
    exit(ERR_MEM);
  }
  onesurfdata->gradframes[lframenum].gradvec = gradvec_p;
  
  if ((arrows_p = (Arrow *) calloc((size_t) nfgrads, sizeof(Arrow))) == NULL) {
    fprintf(stderr, "ReadOneGradFile: No memory available for arrows_p\n");
    exit(ERR_MEM);
  }
  onesurfdata->gradframes[lframenum].arrows = arrows_p;
  
  /*** Read the data finally.  ***/
  
  ngradread = 0;
  for (i = 0; i < nfgrads; i++, gradp_p++, gradvec_p++) {
    if (fscanf(luin_p, "%f %f %f %f %f %f\n",
               &gradp_p->x, &gradp_p->y, &gradp_p->z, &gradvec_p->x, &gradvec_p->y, &gradvec_p->z) != 6) {
      fprintf(stderr, "*** In ReadGraden problems because the file"
              " yielded less data than expected\n"
              "     We expect %ld values but are only at #%ld\n", nfgrads, ngradread + 1);
      exit(ERR_FILE);
    }
    ngradread++;
  }
  onesurfdata->gradframes[lframenum].numgrads = ngradread;
  if (lframenum == 0) {
    onesurfdata->maxnumgrads = ngradread;
  }
  else {
    if (ngradread != maxnumgrads) {
      fprintf(stderr, "Warning-ReadOneGradFile: number of grads changed"
              " from one frame to the next from %ld to %ld\n", maxnumgrads, ngradread);
      onesurfdata->maxnumgrads = cjmax(maxnumgrads, ngradread);
    }
  }
  return 0;
}

long ReadMap3dFidfile(Surf_Input * sinputlist, Surf_Data * surfdata, long numsurfsread, long insurfnum)
{
  /***
Input:
  sinputlist	    structure of all the surface information we need
  surfdata	    the surfdata array for all the surfaces
  numsurfsread    number of surfaces for which we have geometry
  and hence, expect to find data.
  insurfnum	    surface number in which to start storing data
  fidfilename     the place to find the fids, a .fid file.
  
Output:	    surfdata gets updated
Return:	    fidinfo (allocated and set up here--updated laer.
                     
                     
                     ***********************************************************************/
  long error = 0;
  long reportlevel;
  long numfseriesinfile, index, maxindex, displaysurfnum;
  long nodenum, numsurfnodes;
  long surfstart, surfend;
  Series_Fids *infidseries = NULL, *outfidseries = NULL;
  /*********************************************************************/
  reportlevel = map3d_info.reportlevel;
  if (reportlevel)
    fprintf(stderr, "Reading fid file %s\n", sinputlist->fidfilename);
  
  /*** Initialize the fid library ***/
  
  FI_Init(reportlevel);
  
  if (insurfnum < 0) {
    fprintf(stderr, "*** In ReadMap3dFidfile: Error since insurfnum = %ld\n" " Exit here\n", insurfnum);
    exit(ERR_MISC);
  }
  
  /*** See how many series are in the fid file. ***/
  
  numfseriesinfile = GetNumFidfileSeries(sinputlist->fidfilename);
  if (numfseriesinfile < 1) {
    fprintf(stderr, "*** In ReadMap3dFidfile: Error since no fids in file\n" " Exit here\n");
    return ERR_FILE;
  }
  
  /*** Now see if the numbers all match.  This will be primitive, but
    we can refine it later if need be. ***/
  
  if (sinputlist->fidseriesnum >= numfseriesinfile) {
    fprintf(stderr, "*** In ReadMap3dFidfile: Error since there are %ld"
            " fids in file\n" "*** But you want number %ld\n", numfseriesinfile, sinputlist->fidseriesnum + 1);
    return (ERR_MISC);
  }
  
  /*** Now get the series we want from the file. ***/
  
  infidseries = (Series_Fids *) calloc((size_t) 1, sizeof(Series_Fids));
  
  ReadFidfileSeries(reportlevel, sinputlist->fidfilename, sinputlist->fidseriesnum, 0, infidseries);
  
  /*** Now we have to sort these into the right order to match the 
    potentials, or at least to map to the right geometry nodes.
    Note that one input fid series gets parcelled out to a set of
    output fid series.
    ***/
  
  surfstart = insurfnum;
  surfend = insurfnum + numsurfsread - 1;
  maxindex = infidseries->numfidleads - 1;
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    numsurfnodes = map3d_geom[displaysurfnum].numpts;
    outfidseries = (Series_Fids *) calloc((size_t) 1, sizeof(Series_Fids));
    outfidseries->tsnum = infidseries->tsnum;
    outfidseries->pakfilenum = infidseries->pakfilenum;
    outfidseries->numfidtypes = infidseries->numfidtypes;
    CopyFidTypes(outfidseries, infidseries);
    CopyFidNames(outfidseries, infidseries);
    strcpy(outfidseries->fidlabel, infidseries->fidlabel);
    outfidseries->numfidleads = numsurfnodes;
    outfidseries->leadfids = (Lead_Fids *) calloc((size_t) numsurfnodes, sizeof(Lead_Fids));
    
    for (nodenum = 0; nodenum < numsurfnodes; nodenum++) {
      index = map3d_geom[displaysurfnum].channels[nodenum];
      if (index > maxindex || index < 0) {
        fprintf(stderr, "*** In ReadMap3dFidfile\n"
                "    For surface #%ld \n"
                "    Data buffer index points to %ld "
                "in data buffer\n" "    but max size is %ld\n", displaysurfnum + 1, index, maxindex);
        fprintf(stderr, " numberofnodes = %ld, nodenum = #%ld"
                " channel = #%ld\n", numsurfnodes, nodenum + 1, map3d_geom[displaysurfnum].channels[nodenum] + 1);
        return (0);
      }
      outfidseries->leadfids[nodenum].leadnum = nodenum;
      outfidseries->leadfids[nodenum].numfids = infidseries->leadfids[index].numfids;
      
      int numfidtypes = infidseries->leadfids[index].numfids;
      outfidseries->leadfids[nodenum].fidtypes = 
        (short *) calloc( (size_t) numfidtypes, sizeof( short ) );
      outfidseries->leadfids[nodenum].fidvals = 
        (float *) calloc( (size_t) numfidtypes, sizeof( float ) );
      
      for ( int fnum=0; fnum < numfidtypes; fnum++ ) {
        outfidseries->leadfids[nodenum].fidtypes[fnum] = infidseries->leadfids[nodenum].fidtypes[fnum];
        outfidseries->leadfids[nodenum].fidvals[fnum] = infidseries->leadfids[nodenum].fidvals[fnum];
      }
    }
    surfdata[displaysurfnum].fids = outfidseries;
    surfdata[displaysurfnum].numfs = 1;
  }
  
  /*** Clean up and get back to calling routine.  ***/
  
  free(infidseries);
  return (error);
}

/*======================================================================*/
long CopyFidTypes(Series_Fids * outfidseries, Series_Fids * infidseries)
{
  /*** Copy all the fid types from one struct to the other ***/
  
  long numfidtypes, fidnum, error = 0;
  /**********************************************************************/
  numfidtypes = infidseries->numfidtypes;
  outfidseries->fidtypes = (short *)calloc((size_t) numfidtypes, sizeof(short));
  for (fidnum = 0; fidnum < numfidtypes; fidnum++) {
    outfidseries->fidtypes[fidnum] = infidseries->fidtypes[fidnum];
  }
  return (error);
}

/*======================================================================*/
long CopyFidNames(Series_Fids * outfidseries, Series_Fids * infidseries)
{
  /*** Copy all the fid type names from one struct to the other ***/
  
  long numfidtypes, fidnum, error = 0;
  /**********************************************************************/
  numfidtypes = infidseries->numfidtypes;
  outfidseries->fidnames = (char **)calloc((size_t) numfidtypes, sizeof(char *));
  for (fidnum = 0; fidnum < numfidtypes; fidnum++) {
    if ((outfidseries->fidnames[fidnum] =
         (char *)calloc((size_t) strlen(infidseries->fidnames[fidnum]) + 1, sizeof(char))) == NULL) {
      ReportError("CopyFidNames", "error getting fidnames memory", ERR_MEM, "");
      return (ERR_MEM);
    }
    strcpy(outfidseries->fidnames[fidnum], infidseries->fidnames[fidnum]);
  }
  return (error);
}

long ReadMap3dGeomFile(Map3d_Geom* onemap3dgeom, long insurfnum)
{
  
  /*** 
  A routine to read a single surface from a .geom file for the map3d program.
  
  Input/Output:
  *onemap3dgeom   	the map3d_geom structure for one surfaces.
  *onedrawinfo          draw info structure for this surface
Returns:
  the number of surfaces read, < 0 for error case.
  ***/
  long numsurfsread = 0;
  long startsurf = 0, endsurf = 0;
  char filename[100], basefilename[100];
  
  Surf_Geom *surfgeom = NULL;
  strcpy(basefilename, onemap3dgeom->basefilename);
  StripExtension(basefilename);
  strcpy(filename, basefilename);
  strcat(filename, ".geom");
  /*************************************************************************/
  
  if ( map3d_info.reportlevel > 0 ) { 
    if (insurfnum == -1)
      fprintf(stderr, "Reading geom file %s with all surfaces\n", filename);
    else
      fprintf(stderr, "Reading geom file %s, surface# %d\n", filename, insurfnum);
  }
  
  /*** If there is a desired surface, then read it, otherwise read all the 
    surfaces in the file.
    ***/
  
  if (insurfnum < 1) {
    startsurf = -1;             // will call ReadGeomFile to read all surfaces (BJW)
  }
  else {
    startsurf = insurfnum - 1;
    endsurf = startsurf;
  }
  
  if (map3d_info.reportlevel > 1)
    fprintf(stderr, "Readmap3dgeom: we will search the geometry"
            " file for surfaces" " #%ld to #%ld\n", startsurf + 1, endsurf + 1);
  
  /*** Use the standard routine to read the geometry info. ***/
  
  surfgeom = ReadGeomFile(filename, startsurf, endsurf, &numsurfsread, map3d_info.reportlevel > 1);
  if (surfgeom == NULL)
    return (-1);
  if (numsurfsread < 1) {
    fprintf(stderr, "Readmap3dgeom: error because numsurfsread = %ld\n", numsurfsread);
    return (ERR_FILE);
  }
  if (map3d_info.reportlevel > 1)
    printf(" ReadGeomFile reports %ld surfaces read\n", numsurfsread);
  
  /*** Now transfer the information from the standard structure to the 
    Special Map3d structure. ***/
  
  int i;
  for (i = 0; i < numsurfsread; i++) {
    Map3d_Geom *trace = &onemap3dgeom[i];
    trace->points[trace->geom_index] = surfgeom[i].nodes;
    trace->numpts = surfgeom[i].numpts;
    trace->channels = surfgeom[i].channels;
    trace->numelements = surfgeom[i].numelements;
    trace->elements = surfgeom[i].elements;
    trace->elementsize = surfgeom[i].elementsize;
    strcpy(trace->label, surfgeom[i].label);
    strcpy(trace->filepath, surfgeom[i].filepath);
    //strcpy(trace->basefilename, basefilename);
    
  }
  /*** And here, update other map3dgeom structure stuff. ***/
  if (map3d_info.reportlevel > 1)
    fprintf(stderr, "   Finished reading %ld surfaces\n", numsurfsread);
  return (numsurfsread);        //instead of 1
}

void WriteMap3dGeomFile(char *filename, vector<Map3d_Geom *> map3dgeoms)
{
  Surf_Geom *surfgeom = new Surf_Geom[map3dgeoms.size()];
  unsigned i;
  for (i = 0; i < map3dgeoms.size(); i++) {
    surfgeom[i].nodes = map3dgeoms[i]->points[map3dgeoms[i]->geom_index];
    surfgeom[i].numpts = map3dgeoms[i]->numpts;
    surfgeom[i].channels = map3dgeoms[i]->channels;
    surfgeom[i].surfnum = i;
    
    strcpy(surfgeom[i].label, map3dgeoms[i]->label);
    strcpy(surfgeom[i].filepath, map3dgeoms[i]->filepath);

    surfgeom[i].numelements = map3dgeoms[i]->numelements;
    surfgeom[i].elements = map3dgeoms[i]->elements;
    surfgeom[i].elementsize = map3dgeoms[i]->elementsize;    
  }
  
  WriteGeomFile(filename, surfgeom, -1, -1, map3dgeoms.size(), 0);
  delete [] surfgeom;
  
}

long ReadMatlabGeomFile(Map3d_Geom* geom, long insurfnum)
{
  if ( map3d_info.reportlevel > 0 ) { 
    if (insurfnum == -1)
      fprintf(stderr, "Reading geom file %s with all surfaces\n", geom->basefilename);
    else
      fprintf(stderr, "Reading geom file %s, surface# %d\n", geom->basefilename, insurfnum);
  }
  
  matlabfile mf(geom->basefilename, "r");
  int numArrays = mf.getnummatlabarrays();
  if (numArrays != 1) {
    printf("%s - invalid file format\n", geom->basefilename);
    return 0;
  }
  
  matlabarray cell;
  
  matlabarray ma = mf.getmatlabarray(0);  // Get the first matlab array from the file
  std::string name = ma.getname();		// Get the name of this array
  int num = ma.getnumelements();
  if (insurfnum > num) {
    printf("%s does not contain %d surfaces\n", geom->basefilename, insurfnum);
    return 0;
  }
  
  int index = insurfnum > 0 ? insurfnum-1 : 0;
  
  matlabarray pts;
  matlabarray fac;
  matlabarray tet;
  matlabarray seg;
  matlabarray cha;
  
  if (ma.isstruct()) {
    if (ma.isfieldCI("pts")) pts = ma.getfieldCI(index,"pts");
    if (ma.isfieldCI("node")) pts = ma.getfieldCI(index,"node");

    if (ma.isfieldCI("fac")) fac = ma.getfieldCI(index,"fac");
    if (ma.isfieldCI("face")) fac = ma.getfieldCI(index,"face");
    
    if (ma.isfieldCI("cell")) tet = ma.getfieldCI(index,"cell"); 
    if (ma.isfieldCI("tet")) tet = ma.getfieldCI(index,"tet");
    if (ma.isfieldCI("tetra")) tet = ma.getfieldCI(index,"tetra");
    
    if (ma.isfieldCI("channels")) cha = ma.getfieldCI(index,"channels");
    
    if (ma.isfieldCI("edge")) seg = ma.getfieldCI(index,"edge");
    if (ma.isfieldCI("seg")) seg = ma.getfieldCI(index,"seg");
  }
  else if (ma.iscell()) {
    
    cell = ma.getcell(index);
	
    if(cell.isstruct()) {
      if (cell.isfieldCI("pts")) pts = cell.getfieldCI(0,"pts");
      if (cell.isfieldCI("node")) pts = cell.getfieldCI(0,"node");
      
      if (cell.isfieldCI("fac")) fac = cell.getfieldCI(0,"fac");
      if (cell.isfieldCI("face")) fac = cell.getfieldCI(0,"face");
      
      if (cell.isfieldCI("cell")) tet = cell.getfieldCI(0,"cell"); 
      if (cell.isfieldCI("tet")) tet = cell.getfieldCI(0,"tet");
      if (cell.isfieldCI("tetra")) tet = cell.getfieldCI(0,"tetra");
      
      if (cell.isfieldCI("channels")) cha = cell.getfieldCI(0,"channels");
      
      if (cell.isfieldCI("edge")) seg = cell.getfieldCI(0,"edge");
      if (cell.isfieldCI("seg")) seg = cell.getfieldCI(0,"seg");
    }
	
    
  }
  
  /* BUILD IN SOME MORE ROBUSTNESS */
  /* if matrix define the other way around, just transpose the matrix
    also check if arrays have correct dimensions */
 
  if (!pts.isempty()) {
    if ((pts.getm() != 3)&&(pts.getn()==3)) pts.transpose();
    if (pts.getm() != 3) {
      printf("Pts matrix must have dimensions 3xn\n");
      return 0;
    }
  }
  
  if (!fac.isempty()) {
    if ((fac.getm() != 3)&&(fac.getn()==3)) fac.transpose();
    if (fac.getm() != 3) {
      printf("Fac matrix must have dimensions 3xn\n");
      return 0;
    }
  }
  
  if (!tet.isempty()) {
    if ((tet.getm() != 4)&&(tet.getn()==4)) tet.transpose();
    if (tet.getm() != 4) {
      printf("Tetra matrix must have dimensions 4xn\n");
      return 0;
    }
  }
  
  if (!seg.isempty()) {
    if ((seg.getm() != 2)&&(seg.getn()==2)) seg.transpose();
    if (seg.getm() != 2) {
      printf("Seg matrix must have dimensions 2xn\n");
      return 0;
    }
  }
  
  if (!pts.isempty()) {
    if (!fac.isempty()) {
      if (!seg.isempty() || !tet.isempty()) {
        printf("Can only specify one of pts, fac, or tetra\n");
        return 0;
      }
      // load fac data
      geom->SetupMap3dSurfElements(fac.getnumelements()/3, 3);
      long* elements = new long[geom->numelements*3];
      fac.getnumericarray(elements, geom->numelements*3);
      for (int i = 0; i < geom->numelements; i++) {
        geom->elements[i][0] = elements[i*3+0]-1;
        geom->elements[i][1] = elements[i*3+1]-1;
        geom->elements[i][2] = elements[i*3+2]-1;
      }

      delete[] elements;
    }
    else if (!seg.isempty()) {
      if (!tet.isempty()) {
        printf("Can only specify one of pts, fac, or tetra\n");
        return 0;
      }
      // load seg data
      geom->numelements = seg.getnumelements()/2;
      geom->SetupMap3dSurfElements(geom->numelements, 2);
      long* elements = new long[geom->numelements*2];
      seg.getnumericarray(elements, geom->numelements*2);
      for (int i = 0; i < geom->numelements; i++) {
        geom->elements[i][0] = elements[i*2+0]-1;
        geom->elements[i][1] = elements[i*2+1]-1;
      }
      delete[] elements;
    }
    else if (!tet.isempty()) {
      // load tetra data
      geom->SetupMap3dSurfElements(geom->numelements, 4);
      long* elements = new long[geom->numelements*4];
      tet.getnumericarray(elements, geom->numelements*4);
      for (int i = 0; i < geom->numelements; i++) {
        geom->elements[i][0] = elements[i*4+0]-1;
        geom->elements[i][1] = elements[i*4+1]-1;
        geom->elements[i][2] = elements[i*4+2]-1;
        geom->elements[i][3] = elements[i*4+3]-1;
      }
      delete[] elements;
      
    }
    
    // load pts data
    geom->numpts = pts.getnumelements()/3;
    geom->points[geom->geom_index] = Alloc_fmatrix(geom->numpts, 3);
    float** points = geom->points[geom->geom_index];
    float* elements = new float[geom->numpts*3];
    pts.getnumericarray(elements, geom->numpts*3);
    for (int i = 0; i < geom->numpts; i++) {
      points[i][0] = elements[i*3+0];
      points[i][1] = elements[i*3+1];
      points[i][2] = elements[i*3+2];
    }
    delete[] elements;
    
  }
  else {
    printf("Empty or non-existent pts array\n");
    return 0;
  }
  
  geom->channels = (long*) calloc(sizeof(long), geom->numpts);
  if (!cha.isempty()) {
    long* elements = new long[geom->numpts];
    int numchannels = cha.getnumelements();
    cha.getnumericarray(elements, numchannels);
    
    // make sure we don't seg fault if there are more channels in the file
    // than there are points
    for (int i = 0; i < numchannels && i < geom->numpts; i++) {
      geom->channels[i] = elements[i] -1;
    }
    
    // initialize the rest of the channels if there aren't enough 
    // channels in the file
    for (int i = numchannels; i < geom->numpts; i++) {
      geom->channels[i] = -1;
    }
    delete[] elements;
  }
  else {
    for (int i = 0; i < geom->numpts; i++) {
      geom->channels[i] = i;
    }
  }
  return 1;
}

long ReadLandmarkGeomFile(Map3d_Geom* onemap3dgeom)
{
  if (map3d_info.reportlevel)
    fprintf(stderr, "Reading landmark geom file %s\n", onemap3dgeom->basefilename);
  Land_Mark* lm = DefALandMarkSurf(onemap3dgeom->surfnum);
  ReadLandMarkFile(onemap3dgeom->basefilename, lm, map3d_info.reportlevel);
  
  // figure out how many points to create
  
  const int slices = 8;
  int connections = 0;
  for (int i = 0; i < lm->numsegs; i++) {
    connections += lm->segs[i].numpts - 1;
  }
  
  // double the non-end points
  onemap3dgeom->numpts = connections*slices + lm->numsegs*slices;
  onemap3dgeom->points[onemap3dgeom->geom_index] = Alloc_fmatrix(onemap3dgeom->numpts, 3);
  // two triangles per slice, try to connect the nodes with another set
  onemap3dgeom->SetupMap3dSurfElements(connections*slices*2, 3);
  
  onemap3dgeom->channels = (long*) calloc(sizeof(long), onemap3dgeom->numpts);
  
  int ptnum = 0;
  int channelnum = 0;
  int trinum = 0;
  
  // calculate these here to not have to create the same points 
  // a million times
  float circle_points[slices][3];
  for (int i = 0; i < slices; i++) {
    float radian = 6.2831853f*(((float)i)/slices);
    circle_points[i][0] = cos(radian);
    circle_points[i][1] = sin(radian);
    circle_points[i][2] = 0;
  }
  
  // normal vector of circle for the points
  float vec1[3] = {0,1,0};  //default for only 1 point in the seg  
  float vec2[3];
  float avgvec[3];
  float matrix[9];
  float seg_top[slices][3];
  float seg_bot[slices][3];
  
  for (int i = 0; i < lm->numsegs; i++) {
    LandMarkSeg seg = lm->segs[i];
    for (int j = 0; j < seg.numpts - 1; j++) {
      float *pt = seg.pts[j];
      float *nextpt = seg.pts[j+1];
      float rad = seg.rad[j];
      float nextrad = seg.rad[j+1];
      
      // first segment
      if (j == 0) {
        // compute the top points of the segment, and rotate.
        // the next iterations will use these points as the bottom
        // points
        vec1[0] = nextpt[0] - pt[0];
        vec1[1] = nextpt[1] - pt[1];
        vec1[2] = nextpt[2] - pt[2];
        if (vectorLength(vec1) < .00001) break;
        
        float sin_alpha, cos_alpha;
        float cross[3]; // pseudo-cross product
                        // yz length
        float vecprojYZ = sqrt(vec1[1]*vec1[1] + vec1[2]*vec1[2]);
        if (vecprojYZ > .00001) {
          sin_alpha = vec1[1] / vecprojYZ;
          cos_alpha = vec1[2] / vecprojYZ;
        }
        else {
          sin_alpha = 0;
          cos_alpha = 1;
        }
        cross[0] = vec1[0];
        cross[1] = vec1[1] * cos_alpha - vec1[2] * sin_alpha;
        cross[2] = vec1[Y] * sin_alpha + vec1[Z] * cos_alpha;
        float cross_length = vectorLength(cross);
        float sin_beta = cross[0] / cross_length;
        float cos_beta = cross[2] / cross_length;
        
        matrix[0] = cos_beta;
        matrix[1] = -sin_alpha*sin_beta;
        matrix[2] = -sin_beta*cos_alpha;
        matrix[3] = 0;
        matrix[4] = cos_alpha;
        matrix[5] = -sin_alpha;
        matrix[6] = -sin_beta;
        matrix[7] = sin_alpha*cos_beta;
        matrix[8] = cos_alpha*cos_beta;
        
        float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
        for (int k = 0; k < slices; k++) {
          float newpt[3];
          MultMatrix9x3(matrix, circle_points[k], newpt);
          seg_top[k][0] = pt[0] + newpt[0]*rad;
          seg_top[k][1] = pt[1] + newpt[1]*rad;
          seg_top[k][2] = pt[2] + newpt[2]*rad;
          pts[ptnum+k][0] = seg_top[k][0];
          pts[ptnum+k][1] = seg_top[k][1];
          pts[ptnum+k][2] = seg_top[k][2];
          onemap3dgeom->channels[ptnum+k] = channelnum;
        }
        
        ptnum += slices;
        channelnum++;
        // if there are >2 points
        if (seg.numpts > 2) {
          vec2[0] = seg.pts[2][0] - seg.pts[1][0];
          vec2[1] = seg.pts[2][1] - seg.pts[1][1];
          vec2[2] = seg.pts[2][2] - seg.pts[1][2];
        }
        else {
          vec2[0] = vec1[0];
          vec2[1] = vec1[1];
          vec2[2] = vec1[2];
        }
        avgvec[0] = (vec1[0] + vec2[0]) / 2.0f;
        avgvec[1] = (vec1[1] + vec2[1]) / 2.0f;
        avgvec[2] = (vec1[2] + vec2[2]) / 2.0f;
      }
      // any in-between segment 
      else if (j > 0 && j < seg.numpts-2) {
        float* nextnextpt = seg.pts[j+2];
        for (int k = 0; k < slices; k++) {
          seg_top[k][0] = seg_bot[k][0];
          seg_top[k][1] = seg_bot[k][1];
          seg_top[k][2] = seg_bot[k][2];
        }
        vec1[0] = vec2[0];
        vec1[1] = vec2[1];
        vec1[2] = vec2[2];
        
        vec2[0] = nextnextpt[0] - nextpt[0];
        vec2[1] = nextnextpt[1] - nextpt[1];
        vec2[2] = nextnextpt[2] - nextpt[2];
        avgvec[0] = (vec1[0] + vec2[0]) / 2.0f;
        avgvec[1] = (vec1[1] + vec2[1]) / 2.0f;
        avgvec[2] = (vec1[2] + vec2[2]) / 2.0f;
      }
      // last segment
      else if (j == seg.numpts-2) {
        for (int k = 0; k < slices; k++) {
          seg_top[k][0] = seg_bot[k][0];
          seg_top[k][1] = seg_bot[k][1];
          seg_top[k][2] = seg_bot[k][2];
        }
        vec1[0] = vec2[0];
        vec1[1] = vec2[1];
        vec1[2] = vec2[2];
        avgvec[0] = vec2[0] = nextpt[0]-pt[0];
        avgvec[1] = vec2[1] = nextpt[1]-pt[1];
        avgvec[2] = vec2[2] = nextpt[2]-pt[2];
      }
      
      // prepare the next point
      float sin_alpha, cos_alpha;
      float cross[3]; // pseudo-cross product
                      // yz length
      float vecprojYZ = sqrt(avgvec[1]*avgvec[1] + avgvec[2]*avgvec[2]);
      if (vecprojYZ > .00001) {
        sin_alpha = avgvec[1] / vecprojYZ;
        cos_alpha = avgvec[2] / vecprojYZ;
      }
      else {
        sin_alpha = 0;
        cos_alpha = 1;
      }
      cross[0] = avgvec[0];
      cross[1] = avgvec[1] * cos_alpha - avgvec[2] * sin_alpha;
      cross[2] = avgvec[Y] * sin_alpha + avgvec[Z] * cos_alpha;
      float cross_length = vectorLength(cross);
      float sin_beta = cross[0] / cross_length;
      float cos_beta = cross[2] / cross_length;
      
      matrix[0] = cos_beta;
      matrix[1] = -sin_alpha*sin_beta;
      matrix[2] = -sin_beta*cos_alpha;
      matrix[3] = 0;
      matrix[4] = cos_alpha;
      matrix[5] = -sin_alpha;
      matrix[6] = -sin_beta;
      matrix[7] = sin_alpha*cos_beta;
      matrix[8] = cos_alpha*cos_beta;
      
      float circ_bottom[slices][3];
      for (int k = 0; k < slices; k++) {
        float newpt[3];
        MultMatrix9x3(matrix, circle_points[k], newpt);
        circ_bottom[k][0] = nextpt[0] + newpt[0]*nextrad;
        circ_bottom[k][1] = nextpt[1] + newpt[1]*nextrad;
        circ_bottom[k][2] = nextpt[2] + newpt[2]*nextrad;
      }
      
      // find the best (nontwisted) configuration of points
      // vec1 SHOULD be between this point and the next...
      int bestconfig = 0;
#if 0
      float dist = vectorLength(vec1)/2;
      float mincircum = 1.E10;
      float segvec[slices][3];
      float midp[slices][3];
      for(int k=0; k < slices; k++ )
      {
        int pnum = k;
        float circum = 0;
        for( int l=0; l<slices; l++ )
        {
          segvec[l][0] = circ_bottom[pnum][0] - seg_top[l][0];
          segvec[l][1] = circ_bottom[pnum][1] - seg_top[l][1];
          segvec[l][2] = circ_bottom[pnum][2] - seg_top[l][2];
          float segvecmag = vectorLength(segvec[j]);
          midp[l][0] = segvec[l][0] / segvecmag * dist;
          midp[l][1] = segvec[l][1] / segvecmag * dist;
          midp[l][2] = segvec[l][2] / segvecmag * dist;
          pnum++;
          if ( pnum >= slices) pnum = 0;
          if (l > 0) {
            float temp[3] = {midp[l][0] - midp[l-1][0], midp[l][1] - midp[l-1][1],midp[l][2] - midp[l-1][2]};
            circum += vectorLength(temp);
          }
        }
        float temp[3] = {midp[0][0] - midp[slices-1][0], midp[0][1] - midp[slices-1][1],midp[0][2] - midp[slices-1][2]};
        circum +=  vectorLength(temp);
        if ( circum < mincircum )
        {
          mincircum = circum;
          bestconfig = k;
        }
      }
#endif 
      float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
      for(int k=0; k<slices; k++ )
      {
        seg_bot[k][0] = circ_bottom[(bestconfig+k)%slices][0];
        seg_bot[k][1] = circ_bottom[(bestconfig+k)%slices][1];
        seg_bot[k][2] = circ_bottom[(bestconfig+k)%slices][2];
        pts[ptnum+k][0] = seg_bot[k][0];
        pts[ptnum+k][1] = seg_bot[k][1];
        pts[ptnum+k][2] = seg_bot[k][2];
        onemap3dgeom->channels[ptnum+k] = channelnum;
        
        onemap3dgeom->elements[trinum][0] = ptnum+k;
        onemap3dgeom->elements[trinum][1] = ptnum+((k+1)%slices);
        onemap3dgeom->elements[trinum][2] = ptnum+k-slices;
        onemap3dgeom->elements[trinum+1][0] = ptnum+k-slices;
        onemap3dgeom->elements[trinum+1][1] = ptnum+((k+1)%slices)-slices;
        onemap3dgeom->elements[trinum+1][2] = ptnum+((k+1)%slices);
        trinum += 2;
      }
      
      // increment here so we can refer to the correct point in the triangles
      ptnum += slices;
      channelnum++;
    }
  }
  return 0;
}


void WriteMatlabGeomFile(char * filename, vector<Map3d_Geom *> geoms)
{
  matlabarray data;
  vector<long> dims(2);
  dims[0] = geoms.size();
  dims[1] = 1;
  // create a cell array so we can store different stuff (if necessary) for each geom
  data.createcellarray(dims);

  for (unsigned i = 0; i < geoms.size(); i++) {
    Map3d_Geom* geom = geoms[i];
    matlabarray cell;
    cell.createstructarray();

    cell.addfieldname("pts");
    matlabarray pts;
    pts.createdensearray(3, geom->numpts, matlabarray::miDOUBLE);
    // TODO - save all geom indices
    pts.setnumericarray(geom->points[geom->geom_index], 3, geom->numpts);
    cell.setfield(0, "pts", pts);
    if (geom->elements) {
      // tri case
      long** offset = Alloc_lmatrix(geom->numelements, 3);
      for (int j = 0; j < geom->numelements; j++)
        for (int k = 0; k < 3; k++)
          offset[j][k] = geom->elements[j][k]+1; // make it one-based
      cell.addfieldname("fac");
      matlabarray fac;
      fac.createdensearray(3, geom->numelements, matlabarray::miINT32);
      fac.setnumericarray(offset, 3, geom->numelements);
      cell.setfield(0, "fac", fac);
      Free_lmatrix(offset, geom->numelements);
    }
    else if (geom->elementsize == 2) {
      // seg case
      long** offset = Alloc_lmatrix(geom->numelements, 2);
      for (int j = 0; j < geom->numelements; j++)
        for (int k = 0; k < 2; k++)
          offset[j][k] = geom->elements[j][k]+1; // make it one-based
      cell.addfieldname("seg");
      matlabarray seg;
      seg.createdensearray(2, geom->numelements, matlabarray::miINT32);
      seg.setnumericarray(offset, 2, geom->numelements);
      cell.setfield(0, "seg", seg);
      Free_lmatrix(offset, geom->numelements);
    }
    else if (geom->elements) {
      // tetra case
      long** offset = Alloc_lmatrix(geom->numelements, 4);
      for (int j = 0; j < geom->numelements; j++)
        for (int k = 0; k < 4; k++)
          offset[j][k] = geom->elements[j][k]+1; // make it one-based
      cell.addfieldname("tetra");
      matlabarray tet;
      tet.createdensearray(4, geom->numelements, matlabarray::miINT32);
      tet.setnumericarray(offset, 4, geom->numelements);
      cell.setfield(0, "tetra", tet);
      Free_lmatrix(offset, geom->numelements);
    }
    if (geom->channels) {
      // if there are channels
      long* offset = new long[geom->numpts];
      for (int j = 0; j < geom->numpts; j++)
        offset[j] = geom->channels[j]+1; // make it one-based
      cell.addfieldname("channels");
      matlabarray chan;
      chan.createdensearray(1, geom->numpts, matlabarray::miINT32);
      chan.setnumericarray(geom->channels, geom->numpts);
      cell.setfield(0, "channels", chan);
      delete [] offset;
    }
    data.setcell(i, cell);
  }

  matlabfile file(filename, "w");
  file.putmatlabarray(data, "geom");
}

Surf_Data *ReadPotFiles(Surf_Input * sinputlist, Surf_Data * surfdata,
                        Map3d_Geom * map3dgeom, long numsurfsread, long insurfnum)
{
  /*** Read a set of pot files for a single entry surface in map3d.
  The idea is that for each entry surface there can be more than
  one display surface, which is all set up so that a geometry
  file can have more then one surface in it, i.e. a single entry
  surface has info for a single .geom file, which can have more
  than one surface in it.  Then we get more display surfaces than
  input or entry surfaces.
  
  Here we read one pot file, or one set of pot files, for each
  display surface.  This is a weird way to use pot files
  but is legal if we either use the same set of pots over and over 
  again, or have channels files that select portions of the pot file
  data.
  
Input:
  sinputlist   	the structure of information for this input
  or entry surface. We get lots of stuff from here.
  surfdata	    	the surfdata array for all the surfaces
  numsurfsread 	the number of display surfaces that were read for 
  this entry surface.  If this is more than 1,
  we have to re-read the same pot files
  insurfnum	        number of display surfaces at which to start
  loading new potentials.
  
  ***/
  long surfcount;
  long displaysurfnum; /*** Surface into which we place data. ***/
  long framenum, numpotsurfs;
  long numframes, numleads;
  long filenum, filenum1, filenum2, filestep;
  long *channels_p;  /*** Pointer to one surface worth of channels ***/
  char filename[200], basefilename[200];
  char datafilename[256];
  float potscale;
  /********************************************************************/
  
  
  filenum1 = sinputlist->ts_start;
  filenum2 = sinputlist->ts_end;
  filestep = sinputlist->ts_sample_step;
  
  
  if (filenum2 < filenum1)
    filenum2 = filenum1;
  if (filestep < 0)
    filestep = 1;
  strcpy(basefilename, sinputlist->potfilename);
  StripExtension(basefilename);
  if (filenum2 > filenum1)
    basefilename[strlen(basefilename) - 3] = 0;

  if (map3d_info.reportlevel > 0) {
    if (filenum2 > filenum1) {
      fprintf(stderr, "Reading pot files starting with %s%-3.3ld\n", basefilename, filenum1+1);
      printf("   (basefilename is %s from %s)\n", basefilename, sinputlist->potfilename);
    }
    else {
      printf("Reading pot file: %s\n", sinputlist->potfilename);
    }
  }

  potscale = sinputlist->potscale;
  numpotsurfs = 0;
  
  /*** Loop through all the surfaces read for this entry surface. 
    ***/
  
  for (surfcount = 0; surfcount < numsurfsread; surfcount++) {
    
    displaysurfnum = insurfnum + surfcount;
    numleads = map3dgeom[displaysurfnum].numpts;
    channels_p = map3dgeom[displaysurfnum].channels;
    
    /*** Set up the sequence of files that we want to read in for this
      surface.
      ***/
    framenum = 0;
    if (map3d_info.reportlevel > 1)
      fprintf(stderr, " Loop of pot files for display surface %ld "
              "from %ld to %ld step %ld\n", displaysurfnum + 1, filenum1 + 1, filenum2 + 1, filestep);
    for (filenum = filenum1; filenum <= filenum2; filenum+=filestep) {
      
      /*** Make the .pot filename and open the file. ***/
      
      if (filenum2 > filenum1) {
        sprintf(filename, "%s%-3.3ld", basefilename, filenum + 1);
      }
      else {
        strcpy(filename, basefilename);
      }
      strcat(filename, ".pot");
      
      if (!getFileWithPath(filename,getenv("MAP3D_DATAPATH"),sinputlist->parent->datapathname,datafilename))
        return surfdata;
      
      /*** Now go read the file into this surface's arrays.  ***/     
      /*** Allocate memory for the new surface of data in the surfdata array. ***/
      
      if (filenum == filenum1) {
        numframes = (long) floor ( (double) (filenum2-filenum1 + 1) / filestep + .999 );
        //numframes = (long)((filenum2 - filenum1) + 1);
        if (numframes > map3d_info.maxnumframes)
          map3d_info.maxnumframes = numframes;
        if ((surfdata = Surf_Data::AddASurfData(surfdata, displaysurfnum, numframes, numleads)) == NULL)
          return (NULL);
        surfdata->ts_end = filenum2;
        surfdata->ts_start = filenum1;
        surfdata->ts_sample_step = filestep;
        surfdata->numframes = (filenum2 - filenum1) + 1;
        
      }
      
      if (map3d_info.reportlevel > 1)
        fprintf(stderr, " Reading potentials from \n" "file: %s\n", filename);
      FILE* luin_p = fopen(datafilename,"r");
      ReadOnePotFile(luin_p, potscale, &surfdata[displaysurfnum], framenum, channels_p);
      fclose(luin_p);
      framenum++;
    }
    
    numpotsurfs++;
    if (sinputlist->parent->datapathname && strlen(sinputlist->parent->datapathname) > 1)
      strcpy(surfdata[displaysurfnum].filepath, sinputlist->parent->datapathname);
    strcpy(surfdata[displaysurfnum].potfilename, sinputlist->potfilename);
  }
  return surfdata;
}

/*================================================================*/
void ReadOnePotFile(FILE * luin_p, float potscale, Surf_Data * onesurfdata, long lframenum, long *onechannels)
/*** Read the potentials values from the already open file 
Dynamic allocation version.

Input:
potscale	      float value multiplied by each value in the file
as it is read in.
onesurfdata     one structure to load up with data
lsurfnum        local surface number
onechannels     pointer to channels for this surface.

***/
{
  long i;
  long npotread, ninpots, index;
  long numleads, nfields;
  float *potbuff;   /*** Buffer to hold a set of potentials ***/
  /******************************************************************/
  
  numleads = onesurfdata->numleads;
  
  /*** Set up the buffer used to store one set of potentials. ***/
  
  ninpots = GetFileLength(luin_p);
  rewind(luin_p);
  
  if ((potbuff = (float *)calloc((size_t) ninpots, sizeof(float))) == NULL) {
    fprintf(stderr, "*** In ReadOnePotFile error getting memory"
            " for pot data\n" "    in frame #%ld\n", lframenum + 1);
    fprintf(stderr, "   num_values = %ld\n", ninpots);
    exit(ERR_MEM);
  }
  
  /*** Read the data finally, first into a buffer.  ***/
  
  npotread = 0;
  for (i = 0; i < ninpots; i++) {
    if ((nfields = fscanf(luin_p, "%f\n", &potbuff[i])) != 1) {
      fprintf(stderr, "*** In ReadOnePotFile problems because the file\n"
              "     yielded wrong number of fields or less"
              " data than expected\n"
              "     We expect %ld values but are only at %ld\n"
              "     and have a line with %ld fields\n", ninpots, npotread + 1, nfields);
      exit(ERR_FILE);
    }
    npotread++;
  }
  
  /*** Now transfer the data to the final location, using the channels
    buffer to redirect the effort. ***/
  
  for (i = 0; i < numleads; i++) {
    index = onechannels[i];
    if (index < 0) {
      /*** Mark as unused data ***/
      onesurfdata->potvals[lframenum][i] = UNUSED_DATA;
      
      /*** Check if we have valid index pointer. ***/
      
    }
    else if (index > ninpots - 1) {
      fprintf(stderr, "*** In ReadOnePotFile  we have an invalid"
              " redirection\n"
              "    The value of the channel index is %ld but"
              " we have %ld values\n"
              "    The index must always be less than the number" " of values in the file\n", index, ninpots);
      exit(ERR_MEM);
    }
    else {
      onesurfdata->potvals[lframenum][i] = potbuff[index] * potscale;
    }
  }
  
  /*** Free up the space needed for the buffer. ***/
  
  free((float *)potbuff);
  potbuff = NULL;
  
  /*** Since pot files don't have units, set a 0 in theunits. ***/
  
  onesurfdata->units = 0;
  
  if (map3d_info.reportlevel > 3) {
    fprintf(stderr, " Potentials for frame %ld read\n", lframenum);
    fprintf(stderr, " First values are\n");
    for (i = 0; i < cjmin(ninpots, 10); i++)
      fprintf(stderr, " %ld  %f\n", i, onesurfdata->potvals[lframenum][i]);
  }
}


Series_Fids *ReadTsdfcFile(char* filename, char* potfilename, long timeseriesnum,
                           Lead_Fids *globalfids, Series_Fids *seriesfids, int *numfs, float *fidmin, float *fidmax){
  string containerFileName(filename);
  // FIX string entryName(shorten_filename(potfilename));
  string entryName(potfilename);
  
  float qtime = 1, stime = 1, ttime = 1, pontime = 1, pofftime = 1, rpeaktime = 1;
  float tpeaktime = 1, stofftime = 1;
  long numgfids;
  int numfidsets = 0;
  
  Container *c = file_cache.readContainerFile(containerFileName);
  if(!c){
    return 0;
  }
  
  FSList *fsl;
  if(c->entryMap.count(entryName)){
    if(c->entryMap[entryName]->paramList != NULL){
      // Get FSList pointer
      fsl = (FSList *)c->entryMap[entryName]->paramList;
    }
    else{
      fprintf(stderr, "Time series %s has 0 parameter sets\n",entryName.c_str());
      return NULL;
    }
  }
  else{
    fprintf(stderr,"ERROR: Time series file not found\n");
    return NULL;
  }
  
  // Get FidSet pointer
  FidSet *myFS;
  
  //set up global fids memory
  numgfids = 8;
  if (( globalfids->fidtypes = (short *) calloc( (size_t) numgfids, sizeof( short ))) == NULL)
  {
    ReportError( "ReadTsdfcFile", "error getting basic fid mem", 
                 ERR_MEM, "");
    return( NULL );
  }
  if (( globalfids->fidvals = (float *) calloc( (size_t) numgfids, sizeof( float ))) == NULL)
  {
    ReportError( "ReadTsdfcFile", "error getting basic fid mem", 
                 ERR_MEM, "");
    return( NULL );
  }
  
  
  /*** Get some fid memory. ***/
  numfidsets = fsl->totalNumPS;
  if ( seriesfids == NULL )
  {
    if (( seriesfids = (Series_Fids *) calloc( (size_t) numfidsets, 
                                               sizeof( Series_Fids ) )) == NULL ) 
    {
      char errtext[80];
      sprintf(errtext," Tried to open %d fidsets", numfidsets);
      ReportError( "ReadDfileFids", "error getting big memory file",
                   ERR_MEM, errtext );
      return( NULL );
    }
  }
  else 
  { 
    if (( seriesfids = (Series_Fids *) realloc( seriesfids, (size_t) 
                                                (numfidsets) * sizeof( Series_Fids ) )) == NULL ) 
    {
      ReportError( "ReadDfileFids", "error expanding big memory file",
                   ERR_MEM, "" );
      return( NULL );
    }
  }
  
  *numfs = fsl->numPS;
  
  for(int i = 0; i < fsl->numPS; i++){
    myFS = fsl->fidSetsArray[i];
    
    //set up series fids memory
    if (( seriesfids[i].leadfids = (Lead_Fids *) 
          calloc( (size_t) myFS->numfidleads, sizeof( Lead_Fids ) )) == NULL )
    {
      ReportError( "ReadtsdfcfileFids", 
                   "error getting leadfids memory", ERR_MEM, "" );
      return( NULL );
    }
    
    //load the seriesfids structure
    seriesfids[i].tsnum = timeseriesnum;
    seriesfids[i].pakfilenum = -1;
    seriesfids[i].numfidleads = myFS->numfidleads;
    seriesfids[i].numfidtypes = myFS->numFidTypesUnique;
    strcpy( seriesfids[i].fidlabel, myFS->name.c_str());
    
    //set up series fidnames memory
    if (( seriesfids[i].fidnames = (char**) 
          calloc( (size_t) myFS->numFidTypesUnique, sizeof( char* ) )) == NULL )
    {
      ReportError( "ReadTsdfcfile", 
                   "error getting fidnames memory", ERR_MEM, "" );
      return( NULL );
    }
    if (( seriesfids[i].fidtypes =
          (short *) calloc( (size_t) myFS->numFidTypesUnique, sizeof( short )) ) == NULL )
    {
      ReportError( "Readtsdfcfile", "error getting fidtypes memory",
                   ERR_MEM, "" );
      return( NULL );
    }
    
    for(int numFidTypes = 0; numFidTypes < myFS->numFidTypesUnique; numFidTypes++){
      if ((seriesfids[i].fidnames[numFidTypes] =
           (char *)calloc((size_t) strlen(myFS->fidNamesUniqueArray[numFidTypes].c_str()) + 1, sizeof(char))) == NULL) {
        ReportError("ReadTsdfcfile", "error getting fidnames memory", ERR_MEM, "");
        return (NULL);
      }
      
      strcpy( seriesfids[i].fidnames[numFidTypes], myFS->fidNamesUniqueArray[numFidTypes].c_str());
      seriesfids[i].fidtypes[numFidTypes] = (short) myFS->GetFidType(myFS->fidNamesUniqueArray[numFidTypes]);
    }
    
    for(int leadnum = 0; leadnum <myFS->numfidleads; leadnum++){
      
      seriesfids[i].leadfids[leadnum].leadnum = leadnum;
      seriesfids[i].leadfids[leadnum].numfids = myFS->fidDescArray[leadnum];
      
      
      if (( seriesfids[i].leadfids[leadnum].fidtypes =
            (short *) calloc( (size_t) seriesfids[i].leadfids[leadnum].numfids, sizeof( short )) ) == NULL )
      {
        ReportError( "ReadtsdfcfileFids", "error getting fidtypes memory",
                     ERR_MEM, "" );
        return( NULL );
      }
      
      if (( seriesfids[i].leadfids[leadnum].fidvals =
            (float *) calloc( (size_t) seriesfids[i].leadfids[leadnum].numfids, sizeof( float )) )== NULL )
      {
        ReportError( "ReadtsdfcfileFids", "error getting fidvals memory",
                     ERR_MEM, "" );
        return( NULL );
      }
      
      for(int fidnum = 0; fidnum< myFS->fidDescArray[leadnum]; fidnum++){
        //series fids
        seriesfids[i].leadfids[leadnum].fidtypes[fidnum] = myFS->leadsArray[leadnum][fidnum]->type;
        seriesfids[i].leadfids[leadnum].fidvals[fidnum] = myFS->leadsArray[leadnum][fidnum]->value;
        
        //get min-max fids
        if(seriesfids[i].leadfids[leadnum].fidvals[fidnum] > *fidmax)
          *fidmax = seriesfids[i].leadfids[leadnum].fidvals[fidnum];
        if(seriesfids[i].leadfids[leadnum].fidvals[fidnum] < *fidmin)
          *fidmin = seriesfids[i].leadfids[leadnum].fidvals[fidnum];
        
        //global fids
        switch(myFS->leadsArray[leadnum][fidnum]->type){
          case 0:
            pontime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
          case 1:
            pofftime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
          case 2:
            qtime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
          case 3:
            rpeaktime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
          case 4:
            stime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
          case 5:
            stofftime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
          case 6:
            tpeaktime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
          case 7:
            ttime = myFS->leadsArray[leadnum][fidnum]->value;
            break;
        }
      }
    }
  }
  
  globalfids->numfids = numgfids;
  globalfids->leadnum = -1;
  globalfids->fidtypes[0] = FI_PON;
  globalfids->fidvals[0] = (float) pontime - 1.f;
  globalfids->fidtypes[1] = FI_POFF;
  globalfids->fidvals[1] = (float) pofftime - 1.f;
  globalfids->fidtypes[2] = FI_QON;
  globalfids->fidvals[2]   = (float) qtime - 1.f;
  globalfids->fidtypes[3] = FI_RPEAK;
  globalfids->fidvals[3]    = (float) rpeaktime - 1.f;
  globalfids->fidtypes[4] = FI_SOFF;
  globalfids->fidvals[4]   = (float) stime - 1.f;
  globalfids->fidtypes[6] = FI_STOFF;
  globalfids->fidvals[6]    = (float) stofftime - 1.f;
  globalfids->fidtypes[6] = FI_TPEAK;
  globalfids->fidvals[6]    = (float) tpeaktime - 1.f;
  globalfids->fidtypes[7] = FI_TOFF;
  globalfids->fidvals[7]   = (float) ttime - 1.f;
  
  if ( map3d_info.reportlevel > 1 ){
    printf(" Some global fidicials from the file\n"
	       " qframe   = %f\n"
	       " ponframe = %f\n"
	       " rpeak    = %f\n",
	       globalfids->fidvals[2],
	       globalfids->fidvals[0],
	       globalfids->fidvals[3]);
  }
  return seriesfids;
}




void ReadChannelsFile(FILE * luin_p, Map3d_Geom * onemap3dgeom)
/*** Read the channels file, which is already open.

The channels file contains a column which links the nodes of the  
geometry with entries in the data file.  The 'channels'
array points, for each node, to the number of the location in the
data array where the data associated with that node is located.
This is used mostly with multiplexed data that contains data that
is either stored in a different order to the points in the geometry
to which it is associated, or contains multiple surfaces of data
which are not otherwise divided.  Examples of the latter are the 
data files that come from the tank since there are not real surfaces
in the tank (aside from the tank surface itself).  By setting up
'channel' arrays, we can functionally connect data values to match
the connections made by linking the points in triangles.

***/
{
  long i, j, nfnodes, nptssurf, nodenum;
  long pointnum;
  long surfnum;
  char instring[100];
  /******************************************************************/
  
  /*** Read the first line and make sure we are OK with this file. ***/
  surfnum = onemap3dgeom->surfnum;
  nptssurf = onemap3dgeom->numpts;
  fscanf(luin_p, "%ld %s \n", &nfnodes, instring);
  if (map3d_info.reportlevel > 1) {
    fprintf(stderr, "In ReadChannelFile for surface #%ld\n", surfnum + 1);
    fprintf(stderr, " First line read as %ld %s\n", nfnodes, instring);
  }
  if (nfnodes != nptssurf) {
    fprintf(stderr, "*** Warning: we have %ld nodes in this surface\n"
            "    but the channel files has info on %ld nodes\n", nptssurf, nfnodes);
  }
  
  /*** Make sure we have a place to put these ***/
  
  if (onemap3dgeom->channels == NULL) {
    printf("*** ReadChannelsFile: no space for channels\n");
    exit(ERR_MEM);
  }
  
  /*** Now go through and read all the channels entries, one per each
    line, and per node in the geometry. ***/
  
  int maxx = 0;

  for (nodenum = 0; nodenum < nfnodes; nodenum++) {
    if (fscanf(luin_p, "%ld %ld \n", &i, &j) != 2) {
      fprintf(stderr, "*** ReadChannelFile, error reading link #%ld\n", nodenum);
      exit(ERR_FILE);
    }
    
    if (nodenum == 312){
      int a;
      a = 5;
    }

    if (i - 1 != nodenum) {
      fprintf(stderr, "Node number read as %ld while counter" " says it should be %ld\n", i, nodenum + 1);
      exit(ERR_FILE);
    }
    onemap3dgeom->channels[nodenum] = j - 1;
    
    if (onemap3dgeom->channels[nodenum] > maxx) {
      maxx = onemap3dgeom->channels[nodenum];
    }

    /*** Make sure we have a legal point being referred to in the channels ***/
    // note: map3d allows '0' or '-1' entries to be "unused" nodes.
    
    if (onemap3dgeom->channels[nodenum] < -2) {
      fprintf(stderr, "*** Warning: in ReadChannelsFile\n" "    node %ld points to a data element < 0\n", nodenum);
    }
  }

  // if there are more points in the surf than the file, pad the rest to be 'unused'
  for (; nodenum < nptssurf; nodenum++)
    onemap3dgeom->channels[nodenum] = -1;
  
  /*** Now look for doubles or any other funky stuff. Don't worry about unused values though ***/
  
  for (nodenum = 0; nodenum < nfnodes; nodenum++) {
    pointnum = onemap3dgeom->channels[nodenum];
    for (i = nodenum + 1; i < nfnodes; i++) {
      if (onemap3dgeom->channels[i] == pointnum && pointnum >= 0) {
        fprintf(stderr, "+++ Warning in ReadChannelsFile\n"
                "    datapoint #%ld is pointed at by node #%ld" " and node #%ld\n", pointnum, nodenum, i);
      }
    }
  }
  return;
}

/*================================================================*/

long ReadChannelLinksFile(FILE * luin_p, Map3d_Geom * onemap3dgeom)
/*** Read the channellinks file, which is already open.

The purpose of the channelinks file is to mark subsets of leads by 
channel number.  So an entry in this file would be:

channelnum labelnum 

where channelnum indicates the channel that should be marked
with the value "labelnum".

Once read in, we have to scan the channelnums for their
associated points and set up the leadlinks array accordingly.

NOTE: TO WORK PROPERLY, WE MUST RUN THIS FUNCTION ONLY AFTER THE CHANNELS
INFORMATION IS AVAILABLE!!!!
***/
{
  long i, nclinks, nvalidlinks, noutleads, leadnum;
  long chlink;
  long pointnum;
  long *leadlinks = NULL;
  char instring[100];
  char chlabel[100];
  char **leadlinklabels = NULL;
  /******************************************************************/
  /*** Read in the file , setting up the channel links array.
    ***/
  
  fscanf(luin_p, "%ld %s \n", &nclinks, instring);
  fprintf(stderr, "In ReadChannelLinksFile\n");
  fprintf(stderr, " First line read as %ld %s\n", nclinks, instring);
  
  /*** Get some local memory for all the links. ***/
  
  if ((leadlinks = (long *)calloc((size_t) nclinks, sizeof(long))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for local leadlinks\n");
    return (ERR_MEM);
  }
  if ((leadlinklabels = (char **)calloc((size_t) nclinks, sizeof(char *))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting local memory" " for leadlinklabels\n");
    return (ERR_MEM);
  }
  
  /*** Now do the reading of the lead link file. ***/
  
  nvalidlinks = 0;
  for (leadnum = 0; leadnum < nclinks; leadnum++) {
    if (fscanf(luin_p, "%ld %s \n", &chlink, chlabel) != 2) {
      fprintf(stderr, "*** ReadChannelLinksFile: error reading" " link #%ld\n", leadnum + 1);
      exit(ERR_FILE);
    }
    
    /*** Load the values into the structures.  ***/
    
    leadlinks[leadnum] = ChanneltoNode(chlink, onemap3dgeom->channels, onemap3dgeom->numpts);
    if (leadlinks[leadnum] >= 0)
      nvalidlinks++;
    if ((leadlinklabels[leadnum] = (char *)calloc((size_t) LEADLABELSIZE + 1, sizeof(char))) == NULL) {
      fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for leadlinklabels lead #%ld\n", leadnum + 1);
      return (ERR_MEM);
    }
    if (strlen(chlabel) <= LEADLABELSIZE) {
      strcpy(leadlinklabels[leadnum], chlabel);
    }
    else {
      strncpy(leadlinklabels[leadnum], chlabel, LEADLABELSIZE);
      leadlinklabels[leadnum][LEADLABELSIZE] = '\0';
      printf("+++ ReadChannelLinksFile: terminated leadlink label\n");
    }
  }
  /*** Now look for doubles or any other funky stuff. ***/
  
  for (leadnum = 0; leadnum < nclinks; leadnum++) {
    pointnum = leadlinks[leadnum];
    if (pointnum >= 0) {
      for (i = leadnum + 1; i < nclinks; i++) {
        if (leadlinks[i] == pointnum) {
          fprintf(stderr, "+++ Warning in ReadChannelLinksFile\n"
                  "    point #%ld is pointed at by lead #%ld" " and lead #%ld\n", pointnum + 1, leadnum + 1, i + 1);
        }
      }
      strcpy(chlabel, leadlinklabels[leadnum]);
      for (i = leadnum + 1; i < nclinks; i++) {
        if (!strcmp(leadlinklabels[i], chlabel)) {
          fprintf(stderr, "+++ Warning in ReadChannelLinksFile\n"
                  "+++ leadlabel %s is found in entry #%ld"
                  " and entry #%ld of channel links file\n", chlabel, leadnum + 1, i + 1);
        }
      }
    }
  }
  
  /*** Now load the subset of links that were actually found in this
    surface and load just those into the data structure. ***/
  
  onemap3dgeom->numleadlinks = nvalidlinks;
  
  if ((onemap3dgeom->leadlinks = (long *)calloc((size_t) nvalidlinks, sizeof(long))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for leadlinks\n");
    return (ERR_MEM);
  }
  if ((onemap3dgeom->leadlinklabels = (char **)calloc((size_t) nvalidlinks, sizeof(char *))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for leadlinklabels\n");
    return (ERR_MEM);
  }
  noutleads = 0;
  for (leadnum = 0; leadnum < nclinks; leadnum++) {
    pointnum = leadlinks[leadnum];
    if (pointnum >= 0) {
      onemap3dgeom->leadlinks[noutleads] = pointnum;
      if ((onemap3dgeom->leadlinklabels[noutleads] = (char *)
           calloc((size_t) strlen(leadlinklabels[leadnum]) + 1, sizeof(char))) == NULL) {
        fprintf(stderr, "*** ReadChannelLinksFile: error getting memory"
                " for leadlinklabels lead #%ld\n", leadnum + 1);
        return (ERR_MEM);
      }
      strcpy(onemap3dgeom->leadlinklabels[noutleads], leadlinklabels[leadnum]);
      noutleads++;
    }
  }
  
  free(leadlinklabels);
  free(leadlinks);
  
  if (noutleads != nvalidlinks) {
    printf("ReadChannelLinks error because noutleads = %ld and" " nvalidlinks = %ld\n", noutleads, nvalidlinks);
    return (ERR_FILE);
  }
  else {
    printf(" For this surface there are %ld valid channel links\n", nvalidlinks);
  }
  
  /*** If we get to here, we have leadlinks, so have then displayed ***/
  
  //    if ( map3d_info.reportlevel > 2 )
  printf(" Setting qshowleadlabels = true\n");
  
  //draw_info[onemap3dgeom->surfnum].qshowleadlabels = true;
  return 0;
}


/*================================================================*/
long ChanneltoNode(long chlink, long *channels, long numpts)
{
  /*** Convert a channel number  to its assoicated node number. ***/
  
  long nodenum;
  chlink--;
  for (nodenum = 0; nodenum < numpts; nodenum++) {
    if (chlink == channels[nodenum]) {
      return (nodenum);
    }
  }
  return (-1);
}

/*======================================================================*/

long NodetoLeadlink(long nodenum, Map3d_Geom * onemap3dgeom, char *leadlabel)
{
  
  /*** Convert a node number to a lead link number, if there is one
  for this node.
  
Input:
  nodenum	    number of the node we want to convert
  onemap3dgeom the complete geometry for this surface
  *leadlabel   poiner to the label that this lead has (NULL is no match)
  
Returns:
  the number of the leadlink that we found to match
  -1 if there is no match.
  ***/
  
  long linknum;
  /**********************************************************************/
  if (onemap3dgeom->numleadlinks <= 0) {
    leadlabel = NULL;
    return (-1);
  }
  
  for (linknum = 0; linknum < onemap3dgeom->numleadlinks; linknum++) {
    if (onemap3dgeom->leadlinks[linknum] == nodenum && linknum != nodenum) {
      leadlabel = onemap3dgeom->leadlinklabels[linknum];
      return (linknum);
    }
  }
  leadlabel = NULL;
  return (-1);
}

/*================================================================*/

long ReadLeadFile(FILE * luin_p, Map3d_Geom * onemap3dgeom)
/*** Read the leadlinkfile file, which is already open.

Note the use of the leadlinks array, which contains the pointers for all the
leads in each surface. So to see which point corresponds to a lead, simply
look the value up in leadlinks that corresponds to the lead number. 

The actual number assigned to each lead link is stored in a separate
array leadlinklabels, and this determines the labels applied to the
leads.  So lead #4 could point to node 343 and be labelled V6.
***/
{
  long i, nfleads, leadnum;
  long pointnum, leadlinknum;
  char instring[100], chlabel[100];
  char **leadlinklabels = NULL;
  int base = 1; // default is to do 1-based files, but if there is a 0, make it 0-based.
  /******************************************************************/
  /*** Read in the file , setting up the lead array.
    ***/
  
  fscanf(luin_p, "%ld %s \n", &nfleads, instring);
  fprintf(stderr, "In ReadLeadFile\n");
  fprintf(stderr, " First line read as %ld %s\n", nfleads, instring);
  
  /*** Get some memory for these links. ***/
  
  if ((onemap3dgeom->leadlinks = (long *)calloc((size_t) nfleads, sizeof(long))) == NULL) {
    fprintf(stderr, "*** ReadLeadFile: error getting memory" " for leadlinks\n");
    return (ERR_MEM);
  }
  if ((leadlinklabels = (char **)calloc((size_t) nfleads, sizeof(char *))) == NULL) {
    fprintf(stderr, "*** ReadLeadFile: error getting memory" " for leadlinklabels\n");
    return (ERR_MEM);
  }
  
  /*** Now do the reading of the lead link file. ***/
  
  for (leadnum = 0; leadnum < nfleads; leadnum++) {
    if (ReadLine(instring, luin_p) == NULL) {
      fprintf(stderr, "*** ReadLeadFile: error reading link #%ld\n", leadnum + 1);
      exit(ERR_FILE);
    }
    /*** Scan the line and check how many values are on each.  We expect
    two values for each line: the label of the leads, and the node
    it should be drawn at.  Eg., V1 234 ***/
    
    if (sscanf(instring, "%s %ld \n", chlabel, &leadlinknum) != 2) {
      fprintf(stderr, "*** ReadLeadFile: error scanning line\n %s\n", instring);
      exit(ERR_FILE);
    }
    
    /*** Load the values into the structures.  ***/
    
    if (leadlinknum == 0)
      base = 0;
    onemap3dgeom->leadlinks[leadnum] = leadlinknum - 1;
    if ((leadlinklabels[leadnum] = (char *)calloc((size_t) LEADLABELSIZE + 1, sizeof(char))) == NULL) {
      fprintf(stderr, "*** ReadLeadLinksFile: error getting memory" " for leadlinklabels lead #%ld\n", leadnum + 1);
      return (ERR_MEM);
    }
    if (strlen(chlabel) <= LEADLABELSIZE) {
      strcpy(leadlinklabels[leadnum], chlabel);
    }
    else {
      strncpy(leadlinklabels[leadnum], chlabel, LEADLABELSIZE);
      leadlinklabels[leadnum][LEADLABELSIZE] = '\0';
      printf("+++ ReadChannelLinksFile: terminated leadlink label\n");
    }
  }
  onemap3dgeom->numleadlinks = nfleads;
  
  /*** Now look for doubles or any other funky stuff. ***/
  
  for (leadnum = 0; leadnum < nfleads; leadnum++) {
    // add 1 to make the file 0-based (as we already subtracted 1) if it is 0-based.
    if (base == 0)
      onemap3dgeom->leadlinks[leadnum]++;
    
    /*** Make sure we have a legal point being referred to in the leadlink ***/
    if (onemap3dgeom->leadlinks[leadnum] > onemap3dgeom->numpts - 1) {
      fprintf(stderr, "+++Warning: in ReadLeadFile\n"
              "+++lead %ld points to a node that is\n"
              "+++larger than what has been read for this surface\n"
              "+++The node expected is %ld, while we have only read"
              " %ld nodes in\n"
              "+++ Setting leadlink pointers to -1\n", leadnum, onemap3dgeom->leadlinks[leadnum], onemap3dgeom->numpts);
      onemap3dgeom->leadlinks[leadnum] = -1;
    }
    
    /*** If there is a zero entry in the leadlink file, flag it by a -1
      for internal checking in the program.
      ***/
    
    else if (onemap3dgeom->leadlinks[leadnum] < 0) {
      onemap3dgeom->leadlinks[leadnum] = -1;
      strcpy(leadlinklabels[leadnum], "");
      fprintf(stderr, "***In ReadLeadFile\n" "   Leadlink #%ld points to a zero point number\n", leadnum + 1);
    }
    pointnum = onemap3dgeom->leadlinks[leadnum];
    if (pointnum >= 0) {
      for (i = leadnum + 1; i < nfleads; i++) {
        if (onemap3dgeom->leadlinks[i] == pointnum) {
          fprintf(stderr, "+++ Warning in ReadLeadFile\n"
                  "    point #%ld is pointed at by lead #%ld" " and lead #%ld\n", pointnum + 1, leadnum + 1, i + 1);
        }
      }
    }
    strcpy(chlabel, leadlinklabels[leadnum]);
    for (i = leadnum + 1; i < nfleads; i++) {
      if (!strcmp(leadlinklabels[i], chlabel)) {
        fprintf(stderr, "+++ Warning in ReadLeadFile\n"
                "+++ leadlebel %s is found in entry #%ld" " and entry #%ld of leadlinks\n", chlabel, leadnum + 1,
                i + 1);
      }
    }
  }
  
  /*** If we get to here, we have leadlinks, so have them displayed ***/
  
  onemap3dgeom->leadlinklabels = leadlinklabels;
  
  if (map3d_info.reportlevel > 2) {
    for (i = 1; i < cjmin(10, nfleads); i++) {
      printf(" Lead #%ld has label %s\n", onemap3dgeom->leadlinks[i], onemap3dgeom->leadlinklabels[i]);
    }
    fprintf(stderr, " Setting qshowleadlabels = true\n");
  }
  //    draw_info[onemap3dgeom->surfnum].qshowleadlabels = true;
  return 0;
}
