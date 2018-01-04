/* map3d-struct.cxx */

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif

#include "map3d-struct.h"
#include "pickinfo.h"
#include "scalesubs.h"
#include "glprintf.h"
#include "MeshList.h"
#include "texture.h"
#include <float.h>
#include <limits.h>
#include <string.h>
#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

void Init_Global_Input(Global_Input * s)
{
  s->numgeomfiles = -1;
  s->qnoborders = false;
  s->qsinglewin = true;
  s->qnovalidity = false;
  s->imagefile = NULL;
  s->bgimage = NULL;
  // position at (0,0,0),(1,1,0)
  s->bgcoords[0] = s->bgcoords[1] = s->bgcoords[2] = s->bgcoords[5] = 0;
  s->bgcoords[3] = s->bgcoords[4] = 1;
  s->report_level = 0;
  //s->dominantsurf = 0;
  s->qabsolute = false;
  s->borderWidth = 0;
  s->titleHeight = 0;
  s->geompathname = NULL;
  s->datapathname = NULL;

  s->scale_scope = LOCAL_SCALE;
  s->scale_model = LINEAR;
  s->scale_mapping = TRUE_MAP;
  s->lockgeneral = LOCK_FULL;
  s->lockrotate = LOCK_FULL;
  s->lockframes = LOCK_FULL;
  s->pickmode = NEW_WINDOW_PICK_MODE;
  s->framestep = 1;
  s->frameloop = false;
  s->report_level = 1;
  s->time_unit = 0;
  s->frames_per_time_unit = 1;
  s->same_scale = 0;
}

void Init_Scalar_Input(Scalar_Input * s)
{
  s->scalarnum = -1;
  s->channelnum = -1;
  s->xmin = -1;
  s->xmax = -1;
  s->ymin = -1;
  s->ymax = -1;
}

void Init_Surf_Input(Surf_Input * s)
{
  s->geomfilename = NULL;
  s->fidfilename = NULL;
  s->chfilename = NULL;
  s->clfilename = NULL;
  s->llfilename = NULL;
  s->lmfilename = NULL;
  s->tmpfilename = NULL;
  s->potfilename = NULL;
  s->geomfiletype = -1;
  s->datafiletype = -1;
  s->geomsurfnum = -1;
  s->colour_grad = -1;
  s->numscalars = -1;
  s->scale_lock = -1;
  s->winxmin = 0;
  s->winxmax = 0;
  s->winymin = 0;
  s->winymax = 0;

  s->lwxmin = 0;
  s->lwxmax = 0;
  s->lwymin = 0;
  s->lwymax = 0;

  s->lworientation = 1;
  s->newwindow = false;
  s->numsurfsread = 0;
  s->displaysurfnum = -1;
  s->potscale = 1.0;
  s->potusermax = 0.0f;
  s->potusermin = 0.0f;
  s->fidseriesnum = 0;
  s->ts_start = 0;
  s->ts_end = -1;
  s->ts_sample_step = 1;
  s->timestep = 1.0;
  s->timestart = 0;
  s->groupid = 0;
  Scalar_Input si;
  Init_Scalar_Input(&si);
  s->scalarInputs.push_back(si);

  // defaults that can be overriden either globally or per-surface on the command-line
  s->colour_mesh[0] = -1;
  s->colour_mesh[1] = -1;
  s->colour_mesh[2] = -1;
  s->colour_ll[0] = -1;
  s->colour_ll[1] = -1;
  s->colour_ll[2] = -1;
  s->colour_bg[0] = -1;
  s->colour_bg[1] = -1;
  s->colour_bg[2] = -1;
  s->colour_fg[0] = -1;
  s->colour_fg[1] = -1;
  s->colour_fg[2] = -1;
  s->large_font = -1;
  s->med_font = -1;
  s->small_font = -1;
  s->contourstep = 0;
  s->colormap = -1;
  s->shadingmodel = -1;
  s->drawmesh = -1;
  s->invert = -1;
  s->negcontdashed = -1;
  s->drawcont = -1;
  s->drawfids = -1;
  s->showlegend = -1;
  s->legendticks = -1;
  s->lighting = -1;
  s->fogging = -1;
  s->numconts = DEF_NUMCONTS; // -1 -> when we get this on the command line
  s->vfov = -FLT_MAX;
  s->axes = -1;
  s->axes_color[0] = s->axes_color[1] = s->axes_color[2] = -1;
  s->translation[0] = 0; s->translation[1] = 0; s->translation[2] = -FLT_MAX;
  s->rotationQuat.w = s->rotationQuat.x = s->rotationQuat.y = s->rotationQuat.z = -FLT_MAX;
  s->all_mark = s->all_sphere = s->all_value = -1;
  s->extrema_mark = s->extrema_sphere = -1;
  s->pick_mark = s->pick_sphere = -1;
  s->lead_mark = s->lead_sphere = -1;

  s->showinfotext = -1;
  s->showlocks = -1;
  s->size_ll = -1;

  s->preloadedDataArray = NULL;
}


Landmark_Draw::Landmark_Draw()
{
  qshowlmark = 1;
  qshowlabels = 1;
  qshowpoint = 1;
  qshowrods = 1;
  drawtype = 1;


  qshowocclus = 1;
  qshowstitch = 1;
  qshowstim = 1;
  qshowlead = 1;
  qshowcor = 1;
  qshowplane = 1;
  qtransplane = 1;
  qshowrod = 1;
  qshowrecneedle = 1;
  qshowpaceneedle = 1;
  qshowcath = 1;
  qshowfiber = 1;
  qshowcannula = 1;


  coronarycolor[0] = 1;
  coronarycolor[1] = 0;
  coronarycolor[2] = 0;

  occluscolor[0] = 0;
  occluscolor[1] = 0;
  occluscolor[2] = 1;

  stitchcolor[0] = 0;
  stitchcolor[1] = 0;
  stitchcolor[2] = 1;

  stimcolor[0] = 0;
  stimcolor[1] = 0;
  stimcolor[2] = 1;

  leadcolor[0] = 0;
  leadcolor[1] = 0;
  leadcolor[2] = 1;

  planecolor[0] = 1;
  planecolor[1] = 0;
  planecolor[2] = 1;
  planecolor[3] = .5;

  rodcolor[0] = 0;
  rodcolor[1] = 1;
  rodcolor[2] = 0;

  recneedlecolor[0] = 0;
  recneedlecolor[1] = 1;
  recneedlecolor[2] = 0;

  paceneedlecolor[0] = 0;
  paceneedlecolor[1] = 1;
  paceneedlecolor[2] = 0;

  fibercolor[0] = 0;
  fibercolor[1] = 1;
  fibercolor[2] = 0;

  cathcolor[0] = 0;
  cathcolor[1] = 1;
  cathcolor[2] = 1;

  cannulacolor[0] = 1;
  cannulacolor[1] = 0;
  cannulacolor[2] = 0;

  picked_segnum = -1;
  picked_ptnum = -1;

  modified_lm = false;
}


Map3d_Info::Map3d_Info()
{
  maxnumframes = 0;
  reportlevel = 0;
  singlewin = 0;
  qabscoord = 0;
  qnoborders = 0;
  qnovalidity = 0;
  pickmode = NEW_WINDOW_PICK_MODE;
  useonepickwindow = 0;
  framenum = 0;
  //Def_Vals defvals;
  //long winids[MAXSURF]; /*** pointers to all open windows. ***/
  picksize = 10;
  sizeincrement = .1f;
  transincrement = 5;
  rotincrement = .07f;
  scaleincrement = 24;       // this number is divided to change the vfov

  lockframes = LOCK_FULL;
  lockgeneral = LOCK_FULL;
  lockrotate = LOCK_FULL;

  use_spacing = 0;
  use_spacing_fid = 0;

  gi = 0;
  parentid = 0;
  mainwidth = 640;
  mainheight = 480;

  //initialize window values
  posx = 0;
  posy = 0;

  borderInitialized = false;
  borderWidth = 0;
  titleHeight = 0;

  numPickwins = 0;
  numLegendwins = 0;

  lock_texture = dot_texture = rainbow_texture = jet_texture = bg_texture = 0;
  strcpy(imagefile, "map3d.png");
  imagesuffix = 0;
  saving_animations = false;

  scale_scope = LOCAL_SCALE;
  scale_model = LINEAR;
  scale_mapping = TRUE_MAP;
  scale_frame_set = 0;
  scale_frame_max = 0;
  scale_frame_min = 0;
  global_potmax = 0;
  global_potmin = 0;
  user_fstep = 1;
  frame_loop = false;
  
  time_unit = "ms";
  frames_per_time_unit = 1;

  same_scale = 0;
  contour_antialiasing = true;
}

Map3d_Info::~Map3d_Info()
{
  if (lock_texture)
    delete lock_texture;
  if (dot_texture)
    delete dot_texture;
  if (rainbow_texture)
    delete rainbow_texture;
  if (jet_texture)
    delete jet_texture;
  if (bg_texture)
    delete bg_texture;
}

Clip_Planes::Clip_Planes()
{
  init();
}

void Clip_Planes::init()
{
  front_enabled = back_enabled = lock_together = false;
  lock_with_object = true;
  front = front_init;
  back = back_init;
  glDisable(GL_CLIP_PLANE0);
  glDisable(GL_CLIP_PLANE1);
  Ball_Init(&bd, 1);
  Ball_Place(&bd, qOne, .8);
}

Bandpoly::~Bandpoly()
{
  if (nodes) {
    Free_fmatrix(nodes, numpts);
  }
}

void Init_Surface_Group(Surface_Group * s)
{
  s->potmin = FLT_MAX;
  s->potmax = -FLT_MAX;
  s->framemin = FLT_MAX;
  s->framemax = -FLT_MAX;
}
