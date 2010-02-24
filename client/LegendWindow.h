/* LegendWindow.h */

#ifndef LEGENDWINDOW_H
#define LEGENDWINDOW_H

#include "GenericWindow.h"
#include "GeomWindowMenu.h" // for MenuGroup

class ColorMap;
class Surf_Data;
class Mesh_Info;

class LegendWindow:public Map3dGLWidget
{
public:
  LegendWindow(QWidget* parent, Mesh_Info* mesh);
  static LegendWindow* LegendWindowCreate(Mesh_Info* mesh, int _width, int _height, int _x, int _y, bool hidden);
  ColorMap **map;
  int orientation;              /* 0 = horizontal, 1 = vertical */
  bool matchContours;           /* 0 = does not match 1 = does match */
  int nticks;                   /* number of tick marks on legend */

  Surf_Data *surf;
  Mesh_Info *mesh;

  // if we start hidden and have -al coordinates set, set to true
  bool specifiedCoordinates;

  void initializeGL();
  void paintGL();
  virtual void keyPressEvent ( QKeyEvent * event );
  virtual void mouseMoveEvent ( QMouseEvent * event );
  virtual void mousePressEvent ( QMouseEvent * event );
  void LegendWindowMenu(int entry);
  void MapOrientation(menu_data * data);
  void MapTicks(menu_data * data);

  int OpenMenu(QPoint point);
  void MenuEvent(int menu_data);
};

#endif
