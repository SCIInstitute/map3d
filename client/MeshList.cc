/* MeshList.cxx */

#ifdef _WIN32
#pragma warning(disable:4172 4514)  /* quiet visual c++ */
#endif

#include <float.h>
#include "MeshList.h"
#include "Contour_Info.h"
#include "Map3d_Geom.h"
#include "Transforms.h"
#include "colormaps.h"
#include "Surf_Data.h"
#include "pickinfo.h"
#include "reportstate.h"
#include "WindowManager.h"
#include "GeomWindow.h"

Mesh_Info::Mesh_Info()
{
  geom = new Map3d_Geom;
  tran = new Transforms;
  data = 0;                     // leave as 0 until it gets used
  cont = 0;
  fidConts.clear();
  fidMaps.clear();

//  fidactcont = 0;
//  fidreccont = 0;
//  fidactmapcont = 0;
//  fidrecmapcont = 0;

  mysurf = 0;
  groupid = 0;

  cmap = &Jet;
  invert = 0;
  legendwin = 0;
  showlegend = 0;
  lighting = 0;
  fogging = 0;
  
  gpriv = 0;
  drawmesh = RENDER_MESH_CONN;
  drawcont = 1;
  drawfids = 1;
  drawactcont = false;
  drawreccont = false;
  drawfidmapcont = -1;
  drawfidmap = 0;
  //drawrecmapcont = false;
  drawisosurf = 0;
  colorMapIndex = 2;
  negcontdashed = 0;
  meshsize = 1;
  contsize = 1;
  meshcolor[0] = 1;
  meshcolor[1] = meshcolor[2] = 0;
  secondarycolor[0] = secondarycolor[1] = 0;
  secondarycolor[2] = 1;
  draw_marks_as_spheres = true;
  mark_all_color[0] = mark_all_color[1] = mark_all_color[2] = 1;
  mark_extrema_color[0] = mark_extrema_color[2] = 0;
  mark_extrema_color[1] = 1;
  mark_ts_color[0] = mark_ts_color[1] = 1;
  mark_ts_color[2] = 0;
  mark_all_size = 2;
  mark_extrema_size = 3;
  mark_lead_size = 3;
  mark_ts_size = 4;
  mark_all_sphere = 0;
  mark_all_sphere_value = 0;
  mark_all_number = 0;
  mark_extrema_sphere = 0;
  mark_extrema_number = 0;
  mark_ts_sphere = 1;
  mark_ts_number = 0;
  mark_lead_sphere = 1;
  mark_lead_number = 4;
  mark_triangulate_size = 5;
  contourspacing = 0.0;
  fidcontourspacing = 0.0;
  //use_spacing = false;
  //use_spacing_fid = false;
  scalarInputs.clear();
  axes = 0;
  axescolor[0] = axescolor[1] = axescolor[2] = (float).7;

  num_selected_pts = 0;

  current_size_selector = 0;
  current_size_increment = 0;

  pickstacktop = -1;
  curpicknode = 0;
  this->qshowpnts = 1;
  shadingmodel = SHADE_NONE;
  shadefids = false;
  gouraudstyle = SHADE_TEXTURED;
}

Mesh_Info::~Mesh_Info()
{
  if (geom)
    delete geom;
  if (data)
    delete data;
  if (cont)
    delete cont;
//  if (fidactcont)
//    delete fidactcont;
//  if (fidreccont)
//    delete fidreccont;
//  if (fidactmapcont)
//    delete fidactmapcont;
//    if (fidrecmapcont)
//    delete fidrecmapcont;
  if (tran)
    delete tran;

  for (int loop = 0; loop <= pickstacktop; loop++) {
    DestroyWindow(pickstack[loop]->pickwin);
  }


}
