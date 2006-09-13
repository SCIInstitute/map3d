/* ParseCommandLineOptions.cxx */

#ifdef _WIN32
#pragma warning(disable:4172 4514)  /* quiet visual c++ */
#endif

#include <vector>
#include "ParseCommandLineOptions.h"
#include "map3d-struct.h"
#include "usage.h"

extern Map3d_Info map3d_info;

using std::vector;

int ParseCommandLineOptions(int argc, char **argv, Global_Input & globalInput)
{
  Surf_Input *newsurf = 0;
  int error_count = 0;
  
  map3d_info.map3d_executable = argv[0];
  /* initialize the global parse struct */
  Init_Global_Input(&globalInput);
  globalInput.SurfList[MAX_SURFS] = new Surf_Input;
  Init_Surf_Input(globalInput.SurfList[MAX_SURFS]);
  globalInput.SurfList[MAX_SURFS]->parent = &globalInput;

  Scalar_Input si;
  Init_Scalar_Input(&si);

  int si_index = 0;

  if (argc >= 2) {
    /* parse the options */
    OPTION_START()

      /* only one argument */
      OPTION_ONLY(
                  if (strcmp(argv[1], "--help") == 0 ||
                    strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0) {
                      PrintUsageStatement(false);
                      exit(1);
                  }
                  /* if it's -v print map3d version */
                  else if (strcmp(argv[1], "-v") == 0) {
                    PrintUsageStatement(true);
                    exit(1);
                  }
                  else if (strcmp(argv[1], "-b") == 0) {
                    globalInput.qnoborders = 1;
                  }
                  else {
                    /* otherwise assume it's a filename */
                    globalInput.numgeomfiles = 0;
                    newsurf = new Surf_Input;
                    Init_Surf_Input(newsurf);
                    newsurf->parent = &globalInput;
                    newsurf->displaysurfnum = 1;
                    OPTION_COPY(0, newsurf->geomfilename) globalInput.SurfList[0] = newsurf;;

                  }
                  )
      /* GLOBAL OPTIONS */

      /* -ac leave geometry in absolute coordinates */
      OPTION_VALUE("-ac", globalInput.qabsolute, true)
      /* -b borderless windows (place all windows onto a composition palette) */
      OPTION_VALUE("-b", globalInput.qnoborders, true)
      /* -bw borderwidth, titleBarHeight - override the default borderwidth and titleheight for better alignment */
      OPTION_2("-bw", globalInput.borderWidth, globalInput.titleHeight)
      /* -dp datafilepath */
      OPTION_1("-dp", globalInput.datapathname)
      /* -ds make the current surface the dominant surface */
      //OPTION_VALUE("-ds", globalInput.dominantsurf, globalInput.numgeomfiles)
      /* -fs mode - sets the global frame step */
      OPTION_1("-fs", globalInput.framestep)
      /* -gp path name to geometry file */
      OPTION_1("-gp", globalInput.geompathname)
      /* -if output image base filename */
      OPTION_1("-if", globalInput.imagefile)
      /* -bgi image for the background */
      OPTION_1("-bgi", globalInput.bgimage)
      /* -bgp coordinates for plane of image for the background (in geometry space) */
      OPTION_ARRAY("-bgp", 6, globalInput.bgcoords)
      /* -l general rotate frame - 1/0 switches to enable general, rotate, and frame locks */
      OPTION_3("-l", globalInput.lockgeneral, globalInput.lockrotate, globalInput.lockframes)
      /* -nv do not do validity check of geometry */
      OPTION_VALUE("-nv", globalInput.qnovalidity, true)
      /* -nw each surface has it's own window */
      OPTION_VALUE("-nw", globalInput.qsinglewin, false)
      /* -pm mode - sets the initial pick mode to mode */
      OPTION_1("-pm", globalInput.pickmode)
      /* -rl set report level */
      OPTION_1("-rl", globalInput.report_level)
      /* -sc range function map - sets the initial scaling configuration to be range, function, and map */
      OPTION_3("-sc", globalInput.scale_scope, globalInput.scale_model, globalInput.scale_mapping)
      /* -v print the map3d version info */
      OPTION_CODE("-v", 0, printf("Map3D version " VERSION "\n");)

      /* OPTIONS THAT COULD AFFECT ALL MESHES (if done before first -f
          OR THE CURRENT MESH ONLY */

      /* -c set the default or current mesh color index */
      OPTION_ARRAY("-c", 3, globalInput.SurfList[globalInput.numgeomfiles]->colour_mesh)
      /* -bg set the default or current background color index */
      OPTION_ARRAY("-bg", 3, globalInput.SurfList[globalInput.numgeomfiles]->colour_bg)
      /* -fg set the default or current foreground color index */
      OPTION_ARRAY("-fg", 3, globalInput.SurfList[globalInput.numgeomfiles]->colour_fg)
      OPTION_1("-lf", globalInput.SurfList[globalInput.numgeomfiles]->large_font)
      OPTION_1("-mf", globalInput.SurfList[globalInput.numgeomfiles]->med_font)
      OPTION_1("-sf", globalInput.SurfList[globalInput.numgeomfiles]->small_font)

      OPTION_1("-sm", globalInput.SurfList[globalInput.numgeomfiles]->shadingmodel)
      OPTION_1("-rm", globalInput.SurfList[globalInput.numgeomfiles]->drawmesh)
      OPTION_1("-ic", globalInput.SurfList[globalInput.numgeomfiles]->invert)
      //OPTION_1("-nc", globalInput.SurfList[globalInput.numgeomfiles]->numconts)
      OPTION_4("-rq", globalInput.SurfList[globalInput.numgeomfiles]->rotationQuat.w,
                      globalInput.SurfList[globalInput.numgeomfiles]->rotationQuat.x,
                      globalInput.SurfList[globalInput.numgeomfiles]->rotationQuat.y,
                      globalInput.SurfList[globalInput.numgeomfiles]->rotationQuat.z)
      OPTION_ARRAY("-tc", 3, globalInput.SurfList[globalInput.numgeomfiles]->translation)
      OPTION_1("-zf", globalInput.SurfList[globalInput.numgeomfiles]->vfov)
      OPTION_1("-slw", globalInput.SurfList[globalInput.numgeomfiles]->showlegend)
      OPTION_1("-el", globalInput.SurfList[globalInput.numgeomfiles]->lighting)
      OPTION_1("-ef", globalInput.SurfList[globalInput.numgeomfiles]->fogging)
      /* -gn group number for scaling with groups */
      OPTION_1("-gn", globalInput.SurfList[globalInput.numgeomfiles]->groupid)
      OPTION_1("-sco", globalInput.SurfList[globalInput.numgeomfiles]->drawcont)
      OPTION_1("-nc", globalInput.SurfList[globalInput.numgeomfiles]->negcontdashed)
      OPTION_1("-x", globalInput.SurfList[globalInput.numgeomfiles]->axes)
      OPTION_ARRAY("-xc", 3, globalInput.SurfList[globalInput.numgeomfiles]->axes_color)
      /* OPTIONS THAT OPEN FILES - each is matched with the geom file in the preceding -f */

      /* -f geometry filename */
      OPTION_CODE("-f", 1,
                  // the MAX_SURFS index is for global settings (done before any surfaces passed in)
                  if (globalInput.numgeomfiles == MAX_SURFS) globalInput.numgeomfiles = 0;
                  else globalInput.numgeomfiles++;
                  si_index = 0;
                  newsurf = new Surf_Input;
                  Init_Surf_Input(newsurf);
                  newsurf->parent = &globalInput;
                  newsurf->displaysurfnum = globalInput.numgeomfiles + 1;
                  OPTION_COPY(1, newsurf->geomfilename) globalInput.SurfList[globalInput.numgeomfiles] = newsurf;
                  )
      /* -ch channels filename */
      OPTION_1("-ch", globalInput.SurfList[globalInput.numgeomfiles]->chfilename)
      /* -cl channellinks filename */
      OPTION_1("-cl", globalInput.SurfList[globalInput.numgeomfiles]->clfilename)
      /* -ff fidfilename */
      OPTION_1("-ff", globalInput.SurfList[globalInput.numgeomfiles]->fidfilename)
      /* -fn fidseriesnum  */
      OPTION_1("-fn", globalInput.SurfList[globalInput.numgeomfiles]->fidseriesnum)
      /* -ll leadlinks filename */
      OPTION_1("-ll", globalInput.SurfList[globalInput.numgeomfiles]->llfilename)
      /* -lm landmark filename */
      OPTION_1("-lm", globalInput.SurfList[globalInput.numgeomfiles]->lmfilename)
      /* -p pot/grad filename */
      OPTION_CODE("-p", 1, globalInput.SurfList[globalInput.numgeomfiles]->potfilenames.push_back(argv[curargindex + 1]);)

      /* OPTIONS THAT ACT ON THE CURRENT SURFACE ONLY (most recent -f) */

      /* -al location for legend window of current surface */
      OPTION_4("-al", globalInput.SurfList[globalInput.numgeomfiles]->lwxmin,
                globalInput.SurfList[globalInput.numgeomfiles]->lwxmax,
                globalInput.SurfList[globalInput.numgeomfiles]->lwymin,
                globalInput.SurfList[globalInput.numgeomfiles]->lwymax)
      /* -as location for window of current surface */
      OPTION_4("-as", globalInput.SurfList[globalInput.numgeomfiles]->winxmin,
                globalInput.SurfList[globalInput.numgeomfiles]->winxmax,
                globalInput.SurfList[globalInput.numgeomfiles]->winymin,
                globalInput.SurfList[globalInput.numgeomfiles]->winymax)
      /* -at set scalar window location */
      OPTION_CODE("-at", 4, if (globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs[si_index].xmin != -1) {
                  si_index++; globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs.push_back(si);}
                  OPTION_COPY(1, globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs[si_index].xmin)
                  OPTION_COPY(2, globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs[si_index].xmax)
                  OPTION_COPY(3, globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs[si_index].ymin)
                  OPTION_COPY(4, globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs[si_index].ymax))
      /* -cg color for vectors for the current mesh */
      OPTION_1("-cg", globalInput.SurfList[globalInput.numgeomfiles]->colour_grad)
      /* -cs scale step size */
      OPTION_1("-cs", globalInput.SurfList[globalInput.numgeomfiles]->contourstep)
      /* -i increment between frame numbers */
      OPTION_1("-i", globalInput.SurfList[globalInput.numgeomfiles]->ts_sample_step)
      /* -lh - set for a horizontally-oriented colorbar */
      OPTION_VALUE("-lh", globalInput.SurfList[globalInput.numgeomfiles]->lworientation, false)
  //      OPTION_CODE("-pf",3,
  //        globalInput.SurfList[globalInput.numgeomfiles]->potfilenames.push_back(argv[curargindex+1]);)
  //        OPTION_COPY(2,globalInput.SurfList[globalInput.numgeomfiles]->ts_start)
  //        OPTION_COPY(3,globalInput.SurfList[globalInput.numgeomfiles]->ts_end)
      /* -ph maximum data value */
      OPTION_1("-ph", globalInput.SurfList[globalInput.numgeomfiles]->potusermax)
      /* -pl minimum data value */
      OPTION_1("-pl", globalInput.SurfList[globalInput.numgeomfiles]->potusermin)
      /* -ps scale value applied to all data */
      OPTION_1("-ps", globalInput.SurfList[globalInput.numgeomfiles]->potscale)
      /* -s first and last framenumbers */
      OPTION_CODE("-s", 2, OPTION_COPY(1, globalInput.SurfList[globalInput.numgeomfiles]->ts_start)
                  OPTION_COPY(2, globalInput.SurfList[globalInput.numgeomfiles]->ts_end)
                  globalInput.SurfList[globalInput.numgeomfiles]->ts_start--;
                  globalInput.SurfList[globalInput.numgeomfiles]->ts_end--;
                  )
      /* -sl surfnum to lock to another surface */
      OPTION_1("-sl", globalInput.SurfList[globalInput.numgeomfiles]->scale_lock)
      /* -t trace lead number */
      OPTION_CODE("-t", 1, if (globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs[si_index].scalarnum != -1) {
                  si_index++; globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs.push_back(si);}
                  OPTION_COPY(1, globalInput.SurfList[globalInput.numgeomfiles]->scalarInputs[si_index].scalarnum))
                    
      /* -w start a new window with current surface */
      OPTION_VALUE("-w", globalInput.SurfList[globalInput.numgeomfiles]->newwindow, true)

      /* doesn't match the valid options listed above */
      OPTION_END(printf("MAP3D ERROR: invalid option \"%s\"\n", OPTION_CUR);
                error_count++;
                )
  }
  /*** Set some counters back to be zero-based ***/
  // we start numgeomfiles at MAX_SURFS then go to 0, so that the surflist array will be 0-based.
  // increment here to be the number of geom files so loops will be intuitive
  if (globalInput.numgeomfiles == MAX_SURFS) 
    globalInput.numgeomfiles = 0;
  else 
    globalInput.numgeomfiles++;
  if (error_count < 1)
    return 1;

  printf("MAP3D ERROR: %d parsing error(s) detected.  exiting.\n", error_count);

  return 0;
}

void option_copy(char *a, char *&s)
{
  if (s)
    delete[]s;
  s = new char[256];
  strcpy(s, a);
}

void option_copy(char *a, char &c)
{
  c = (char)atoi(a);
}

void option_copy(char *a, unsigned char &c)
{
  c = (char)atoi(a);
}

void option_copy(char *a, short &s)
{
  s = (short)atoi(a);
}

void option_copy(char *a, int &i)
{
  i = atoi(a);
}

void option_copy(char *a, bool &b)
{
  b = atoi(a)!=0;
}

void option_copy(char *a, long &l)
{
  l = atol(a);
}

void option_copy(char *a, float &f)
{
  f = (float)atof(a);
}

void option_copy(char *a, double &d)
{
  d = atof(a);
}

void option_error(char *s, int i)
{
  printf("MAP3D ERROR: %s needs %d argument(s)", s, i);
  exit(-1);
}
