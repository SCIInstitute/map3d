/* PickWindow.h */

#ifndef PICKWINDOW_H
#define PICKWINDOW_H

#include "GenericWindow.h"

struct PickInfo;
struct FilesDialogRowData;
class Mesh_Info;

class PickWindow:public GLWindow
{
public:
  PickWindow(bool rms);
  static PickWindow* PickWindowCreate(int _width, int _height, int _x, int _y, int def_width, int def_height);
  virtual void setupEventHandlers();

  PickInfo *pick;
  Mesh_Info *mesh;          // mesh that "owns" this pick window
  bool click;

  float axiscolor[4];           //in addition to fg and bg defined in GenericWindow
  float graphcolor[4];
  float leftoffset, rightoffset, bottomoffset, topoffset;
  float graph_width;

  // info for the rms window (inside the FilesDialog) only
  FilesDialogRowData *dialogRowData;
  int window_line; // when we're dragging, we need to know if it's the line or right marker
};

enum pickmenu
{ axes_color, graph_color, full_screen, graph_width_menu};

#ifdef __cplusplus
extern "C"
{
#endif

  struct mouse_button_data;
  struct mouse_motion_data;
  struct menu_data;
  struct special_key_data;
  struct key_data;
  typedef union _GdkEvent GdkEvent;
  typedef struct _GdkEventCrossing GdkEventCrossing;
  typedef struct _GdkEventMotion GdkEventMotion;
  typedef struct _GdkEventButton GdkEventButton;
  typedef struct _GdkEventKey GdkEventKey;
  typedef void* gpointer;

  void PickWindowInit(GtkWidget * widget, GdkEvent * event, gpointer data);
  void PickWindowDestroy(PickWindow * priv);
  void PickWindowRepaint(GtkWidget * widget, GdkEvent * event, gpointer data);
  void PickWindowReshape(GtkWidget * widget, GdkEvent * event, gpointer data);
  void PickWindowButtonPress(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void PickWindowButtonRelease(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void PickWindowMotion(GtkWidget * widget, GdkEventMotion * event, gpointer data);
  void RMSPickWindowButtonPress(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void RMSPickWindowButtonRelease(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void RMSPickWindowMotion(GtkWidget * widget, GdkEventMotion * event, gpointer data);
  void PickWindowMenu(menu_data * data);
  void PickWindowKeyboardPress(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void PickWindowKeyboardRelease(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void PickWindowHandleKeyboard(key_data * data);
  void PickWindowEnter(GtkWidget * widget, GdkEventCrossing * event, gpointer data);
  void PickWindowLeave(GtkWidget * widget, GdkEventCrossing * event, gpointer data);
  void PickWindowDrawNode(GtkWidget * widget, GdkEvent * event, gpointer data);
  void PickWindowStyle(PickWindow * p, int x);

#ifdef __cplusplus
}
#endif

#endif
