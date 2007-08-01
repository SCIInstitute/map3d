/* GeomWindow.h */

#ifndef GEOMWINDOW_H
#define GEOMWINDOW_H


#ifdef _WIN32
#define MAP3D_SLEEP Sleep
#define MULTIPLIER 1
#else
#define MAP3D_SLEEP usleep
#define MULTIPLIER 1000
#endif

#include "GenericWindow.h"
#include "MeshList.h"
#include "GeomWindowMenu.h"

class Surf_Data;
struct PickInfo;
struct Clip_Planes;


class GeomWindow:public GLWindow
{
public:
  GeomWindow();
  static GeomWindow* GeomWindowCreate(int _width, int _height, int _x, int _y, int def_width, int def_height);
  virtual void setupEventHandlers();
  void addMesh(Mesh_Info* mesh);
  void removeMesh(Mesh_Info* mesh);
  Mesh_List findMeshesFromSameInput(Mesh_Info* mesh);
  void recalcMinMax();
  void setInitialMenuChecks();
  float vfov;                   /* vertical field of view */
  Mesh_List meshes;             /* info for all the meshes in this 
                                   window: geom, data, contours, colormaps,... */
  float l2norm;                 /* the "fit" info for this window */
  float xcenter, ycenter, zcenter;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  float fog1,fog2;

  float light_position[4];


  Clip_Planes *clip;
#ifdef ROTATING_LIGHT
  BallData light_pos;
#endif

  long dominantsurf; /*** Dominant surface number ***/
  long secondarysurf; /*** Surf when frames are unlocked and all displayed
			   in same window ***/

  char all_axes;                /* to display all meshes' axes or only one */
  bool rgb_axes;

  char useonepickwindow;        /* when picking, show only one pick at a time */
  char showlocks;
  int idle_id;                  // gtk id of idle loop

  float large_font;                // size of font for the large, medium, small fonts
  float med_font;                  //   use MAP3D_FONT_SIZE_* from glprintf.h
  float small_font;                //   these are floats to appease dynamic changing ( +/- by size, which are float*)

  MenuGroup surf_color, surf_render, scaling_range, scaling_function, scaling_map;
  MenuGroup picking;
  MenuGroup mesh_render, mesh_select;
  MenuGroup cont_draw_style, cont_num;
  MenuGroup node_all_sphere, node_ext_sphere, node_lead_sphere, node_pick_sphere;
  MenuGroup node_all, node_ext, node_lead, node_pick;
  MenuGroup frame_num;
  MenuGroup lighting;
  MenuGroup surf_invert;
  GtkWidget *front_plane, *back_plane, *c_together, *c_with_object;
  GtkWidget *noFidCont, *actFidCont, *recFidCont,
    *noFidMap, *actFidMap, *recFidMap;
};

#ifdef __cplusplus
extern "C"
{
#endif

  struct mouse_button_data;
  struct mouse_motion_data;
  struct special_key_data;
  struct key_data;

  // to avoid including gtk.h here
  typedef union _GdkEvent GdkEvent;
  typedef struct _GdkEventButton GdkEventButton;
  typedef struct _GdkEventMotion GdkEventMotion;
  typedef struct _GdkEventKey GdkEventKey;
  typedef void* gpointer;

  void GeomWindowBPress(GtkWidget * widget, GdkEventButton * event, gpointer data, float xn, float yn);
  void GeomWindowBRelease(GtkWidget * widget, GdkEventButton * event, gpointer data, float xn, float yn);
  void GeomWindowButtonPress(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void GeomWindowButtonRelease(GtkWidget * widget, GdkEventButton * event, gpointer data);
  //void GeomWindowInit(GeomWindow * priv);
  void GeomWindowInit(GtkWidget * widget, GdkEvent * event, gpointer data);
  //void GeomWindowRepaint();
  void GeomWindowRepaint(GtkWidget * widget, GdkEvent * event, gpointer data);
  //void GeomWindowReshape(int width, int height);
  void GeomWindowReshape(GtkWidget * widget, GdkEvent * event, gpointer data);
  void GeomWindowMouseButton(int button, int state, int x, int y);
  void GeomWindowHandleMouseButton(mouse_button_data * data);
  void GeomWindowMouseMotion(GtkWidget * widget, GdkEventMotion * event, gpointer data);
//  void GeomWindowMouseMotion(int x, int y);
  void GeomWindowHandleMouseMotion(GtkWidget * widget, GdkEventMotion * event, gpointer data, float xn, float yn);
//  void GeomWindowHandleMouseMotion(mouse_motion_data * data);
  bool testKeyboardRepeat(bool active);
  void GeomWindowUpdateAndRedraw(GeomWindow* priv);
  bool GeomWindowHandleFrameAdvances(gpointer data);
  void GeomWindowKeyboard(unsigned char key, int x, int y);
  void GeomWindowKPress(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void GeomWindowKRelease(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void GeomWindowKeyboardRelease(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void GeomWindowKeyboardPress(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void GeomWindowHandleKeyboard(key_data * data);
  void GeomWindowSpecial(int key, int x, int y);
  void GeomWindowSpecialUp(int key, int x, int y);
  void GeomWindowHandleSpecial(special_key_data * data);
  void GeomWindowIdleFunc(gpointer data);
  void GeomWindowTransformKeyboard(Mesh_Info * curmesh, GdkEventKey* event);
  void GeomWindowMenu(menu_data * data);
  void DrawBGImage(GeomWindow* priv);
  void DrawSurf(Mesh_Info * curmesh);
  void DrawMesh(Mesh_Info * curmesh, bool secondary);
  void DrawCont(Mesh_Info * curmesh);
  void DrawFidCont(Mesh_Info * curmesh, Contour_Info *cont);
  void DrawFidMapCont(Mesh_Info * curmesh, Contour_Info *cont);
  void DrawFidMapSurf(Mesh_Info * curmesh,Contour_Info *cont);
  void Transform(Mesh_Info * curmesh, float factor);
  bool Pick(GeomWindow * priv, int meshnum, int x, int y, bool del = false);
  void DrawPicks(Mesh_Info * curmesh);
  void DrawNodes(Mesh_Info * curmesh);
  void DrawLeads(Mesh_Info * curmesh);
  void DrawNodePick(PickInfo * pick);
  void DrawExtrema(Mesh_Info * curmesh);
  void DrawInfo(GeomWindow * priv);
  void DrawLockSymbol(GeomWindow * priv, int which, bool full);
  void DrawDot(float x, float y, float z, float size);
  void DrawAxes(Mesh_Info * curmesh);
  void GeneratePick(PickInfo * pick);
  void DelTriangle(Mesh_Info * curmesh, int nodenum);
  void DelNode(Mesh_Info * curmesh, int nodenum);
  void Triangulate(Mesh_Info * curmesh, int nodenum);
  void SaveGeomToDisk(Mesh_Info * mesh, bool trans);
  void SaveMeshes(Mesh_List& ml, vector<bool> transforms, char* filename);
#ifdef __cplusplus
}
#endif

#endif
