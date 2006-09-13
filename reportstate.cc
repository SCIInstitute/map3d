/* reportstate.cxx */
 /*** Collection of functions that report the state of the rendering
      and display features of map3d 
      Last update: Sat Sep 23 12:36:05 2000 by Rob MacLeod
        - created
***/

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#include "map3d-struct.h"
#include "Map3d_Geom.h"
#include "reportstate.h"
#include "GeomWindow.h"

extern "C" Map3d_Geom * map3d_geom;
extern "C" Map3d_Info map3d_info;

void ReportMeshRender(int drawmesh)
{
  switch (drawmesh) {
  case RENDER_MESH_NONE:
    printf(" Render nothing from the mesh\n");
    break;
  case RENDER_MESH_ELTS:
    printf(" Render mesh elements\n");
    break;
  case RENDER_MESH_CONN:
    printf(" Render mesh connectivity\n");
    break;
  case RENDER_MESH_ELTS_CONN:
    printf(" Render mesh as elements for front face\n");
    break;
  case RENDER_MESH_PTS:
    printf(" Render mesh points only\n");
    break;
  case RENDER_MESH_PTS_CONN:
    printf(" Render mesh points and connectivity\n");
    break;
  default:
    printf(" Illegal drawmesh value of %d\n", drawmesh);
  }
}


void ReportShading(int shadingmodel)
{
  switch (shadingmodel) {
  case SHADE_FLAT:
    printf(" Flat shading model\n");
    break;
  case SHADE_GOURAUD:
    printf(" Gouraud shading model\n");
    break;
  case SHADE_BANDED:
    printf(" Banded shading model\n");
    break;
  case -1:
    printf(" Shading off\n");
    break;
  default:
    printf(" Illegal shading model value of %d\n", shadingmodel);
  }
}
