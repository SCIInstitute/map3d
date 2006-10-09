/* LegendWindow.h */

#ifndef LEGENDWINDOW_H
#define LEGENDWINDOW_H

#include "GenericWindow.h"
#include "GeomWindowMenu.h" // for MenuGroup

class ColorMap;
class Surf_Data;
class Mesh_Info;

class LegendWindow:public GLWindow
{
public:
  LegendWindow();
  static LegendWindow* LegendWindowCreate(int _width, int _height, int _x, int _y, int def_width, int def_height, bool hidden);
  virtual void setupEventHandlers();
  ColorMap **map;
  int orientation;              /* 0 = horizontal, 1 = vertical */
  bool matchContours;           /* 0 = does not match 1 = does match */
  bool is_displayed;            /* 0 - hide, 1 - display */
  int nticks;                   /* number of tick marks on legend */

  Surf_Data *surf;
  Mesh_Info *mesh;
  GtkWidget *vert_orient, *horiz_orient;
  MenuGroup ticks;

  // if we start hidden and have -al coordinates set, set to true
  bool specifiedCoordinates;
};

#ifdef __cplusplus
extern "C"
{
#endif

  struct key_data;
  struct menu_data;

  void LegendWindowRepaint(GtkWidget * widget, GdkEvent * event, gpointer data);
  void LegendWindowReshape(GtkWidget * widget, GdkEvent * event, gpointer data);
  void LegendWindowKeyboardPress(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void LegendWindowKeyboardRelease(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void LegendWindowHandleKeyboard(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void LegendWindowInit(GtkWidget * widget, GdkEvent * event, gpointer data);
  void LegendWindowButtonPress(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void LegendWindowButtonRelease(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void LegendWindowMouseMotion(GtkWidget * widget, GdkEventMotion * event, gpointer data);
  void LegendWindowMenu(int entry);
  void MapOrientation(menu_data * data);
  void MapTicks(menu_data * data);

#ifdef __cplusplus
}
#endif

#endif
