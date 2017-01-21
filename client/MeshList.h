/* MeshList.h */

#ifndef MESHLIST_H
#define MESHLIST_H

class Contour_Info;
class Draw_Info;
class Map3d_Geom;
class Surf_Data;
class Transforms;
class ColorMap;

#define RENDER_MESH_NONE 0
#define RENDER_MESH_PTS 1
#define RENDER_MESH_ELTS 2
#define RENDER_MESH_CONN 3
#define RENDER_MESH_ELTS_CONN 4
#define RENDER_MESH_PTS_CONN 5
#define RENDER_MESH_NONDATA_ELTS 6

#define SHADE_NONE       0
#define SHADE_FLAT       1 /*** Flat shading for shadingtype ***/
#define SHADE_GOURAUD    2 /*** Gourard shading for shadingtype ***/
#define SHADE_BANDED     3 /*** shadingtype for banded shading ***/
#define SHADE_TEXTURED   4 /*** textured version of gouraud shading, interpolating through our colormap ***/

// set up a bunch of defaults here, so we can see if a 
// global or local command line argument is used
#define DEF_MESH_RED 255
#define DEF_MESH_GREEN 0
#define DEF_MESH_BLUE 0
#define DEF_SHADINGMODEL SHADE_NONE
#define DEF_DRAWMESH RENDER_MESH_CONN
#define DEF_CONTOURSPACING 0.0
#define DEF_INVERT false
#define DEF_NEGCONTDASHED false
#define DEF_DRAWCONT true
#define DEF_SHOWLEGEND true
#define DEF_LIGHTING false
#define DEF_FOGGING false
#define DEF_NUMCONTS 10
#define DEF_VFOV 29.0f
#define DEF_AXES false
#define DEF_AXES_COLOR_RGB 178

#include <vector>
#include "map3d-struct.h"

class GeomWindow;
class LegendWindow;
struct PickInfo;
using std::vector;

class Mesh_Info
{
public:
  Mesh_Info();
  ~Mesh_Info();

  GeomWindow *gpriv;            /* geom window where mesh is present */
  Map3d_Geom *geom;             /* the mesh */
  Surf_Data *data;              /* potential/scalar data */
  Contour_Info *cont;           /* contours lines and bands */
  vector < Contour_Info * > fidConts; /* fiducial contours lines and bands */
  vector <bool> drawFidConts;    /* whether to draw each individual fid contour */
  vector <Contour_Info *> fidMaps;          /* fiducial contours lines and bands */
  int drawfidmap;

//  Contour_Info *fidactcont;        /* fiducial contours lines and bands */
//  Contour_Info *fidreccont;        /* fiducial contours lines and bands */
//  Contour_Info *fidactmapcont;        /* fiducial contours lines and bands */
//  Contour_Info *fidrecmapcont;        /* fiducial contours lines and bands */
  Transforms *tran;             /* user transformations (mousings) */
  ColorMap *cmap;               /* the data-to-color map */
  bool invert;                  /* indicates whether mapping should be inverted or not */
  LegendWindow *legendwin;      /* the id of this mesh's legend window */
  bool showlegend;              /* indicates whether this mesh's color map legend should be visible */
  bool lighting;                /* indicates whether to render with lighting */
  bool fogging;                 /* indicates whether to render with depth cue */
  //bool drawsurf;                /* indicates whether to render the color mapped surface */
  int drawmesh;                 /* indicates how to render the geometry (mesh) */
  bool qshowpnts;               /* Draw node marks */
  long shadingmodel;            /* GOURAUD, FLAT, or SHADE_BANDED, or SHADE_NONE */
  bool shadefids;               /* if true, shade fids instead of pots */
  long gouraudstyle;            /* Use real_gouraud or textures ***/
  Landmark_Draw landmarkdraw;   /* Info for drawing landmarks (colors, toggling, etc.)*/

  bool drawcont;                /* indicates whether to render contour lines */
  bool drawfids;                /* indicates whether to render fiducials in general */
  
  bool drawactcont;                /* indicates whether to render fiducial contour lines */
  bool drawreccont;                /* indicates whether to render fiducial contour lines */
  int drawfidmapcont;                /* indicates whether to render fiducial map contour lines */

  bool drawisosurf;             /* indicates whether to render an isosurface */
  int colorMapIndex;            /* index into colormap menu switch statement */
  bool negcontdashed;           /* indicates whether contour lines should be dashed when negative */
  float meshsize;               /* line and point size for the mesh */
  float contsize;               /* line and point size for the contours */
  float meshcolor[4];           /* mesh color */
  float secondarycolor[4];      /* secondary mesh color */
  bool draw_marks_as_spheres;   /* Whether to use textures to draw dots or to draw boxes */
  float mark_all_color[4];      /* color of all node marking */
  float mark_all_size;          /* color of all node marking */
  bool mark_all_sphere;         /* mark all with sphere */
  bool mark_all_sphere_value;   /* whether the spheres are mapped to the data value */
  char mark_all_number;         /* mark all with node, channel or value */
  float mark_extrema_color[4];  /* color of extrema node marking */
  float mark_extrema_size;      /* color of extrema node marking */
  bool mark_extrema_sphere;     /* mark extrema with sphere */
  char mark_extrema_number;     /* mark extrema with node, channel or value */
  float mark_ts_color[4];       /* color of time signal node marking */
  float mark_ts_size;           /* color of time signal node marking */
  bool mark_ts_sphere;          /* mark time signal with sphere */
  char mark_ts_number;          /* mark time signal with node, channel or value */

  float mark_lead_color[4];     /* color of lead node marking */
  float mark_lead_size;         /* color of lead node marking */
  bool mark_lead_sphere;        /* mark lead with sphere */
  char mark_lead_number;        /* mark lead with node, channel or value */

  float mark_triangulate_size;  /* size of node that will mark in the process of triangulation */

  float axescolor[4];           /* axes color */

  bool axes;                    /* whether or not to display axes */

  float contourspacing;         /* user desired voltage gradient between neighboring contours */
  float fidcontourspacing;         /* user desired voltage gradient between neighboring contours */

  //bool use_spacing;             /* last contour spacing value issued */
  //bool use_spacing_fid;             /* last contour spacing value issued */

  vector < Scalar_Input > scalarInputs; /* scalars that are brought up on the command line */
  int w_xmin, w_xmax;           /* the -as option arguments */
  int w_ymin, w_ymax;

  int lw_xmin, lw_xmax;         /* the -al option arguments */
  int lw_ymin, lw_ymax;

  // selected points are used for triangulation and node editing
  int num_selected_pts;
  int selected_pts[3];

  // Use +/- to select stuff
  float *current_size_selector;
  float current_size_increment;
  float current_size_midpoint;
  bool current_size_mesh_based;

  Surf_Input *mysurf;
  int groupid;                  // number of the group this surf belongs to.

  // pick stuff for this mesh
  PickInfo *pickstack[40];
  int pickstacktop;
  int curpicknode;


};

typedef vector < Mesh_Info * >Mesh_List;

#endif
