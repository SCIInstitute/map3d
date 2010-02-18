/* GenericWindow.h */

#ifndef GENERICWINDOW_H
#define GENERICWINDOW_H

#define GEOMWINDOW 1
#define LEGENDWINDOW 2
#define TIMEWINDOW 3
#define RMSWINDOW 4

#define HORIZ (map3d_info.borderWidth + map3d_info.borderWidth)
#define VERTICAL (map3d_info.borderWidth + map3d_info.borderWidth + map3d_info.titleHeight)

#include <QGLWidget>

class Map3dGLWidget : public QGLWidget {
public:
  Map3dGLWidget(QWidget* parent, int type, const char* title, int min_width, int min_height, bool rms=false);
  void setPopLevel();
  void positionWindow(int _width, int _height, int _x, int _y, int def_width, int def_height, bool exactCoords = false);

  // if overridden, call the parent version
  virtual void initializeGL();
  virtual void paintGL() = 0;
  virtual void resizeGL();
  void getCommandLineCoordinates(int& _width, int& _height, int& _x, int& _y);
  int poplevel; // for saving windows - smaller numbers are "on top" of bigger ones
  int min_width, min_height;

  int winid;                    /* this window's id */
  float bgcolor[4];             /* background color */
  float fgcolor[4];             /* foreground color */
  char showinfotext;            /* show window content information */

  int button;                   /* the current mouse button */
  int state;                    /* the current mouse button state */
  int modifiers;                /* the current modifier keys which aee pressed */

  // for moving, sizing windows
  QPoint mouseStart;
  QPoint startPos;
  QSize startSize;
  float last_xn, last_yn; // for moving, percent, for communicating across windows
  int wintype;

  // this so far is only for legend windows, when you specify -slw 0
  bool startHidden;

  //size takes an int from 1-10, or one of the defines below
  //it is a float here because most of the code references are floats - see incrSize in dialogs.cc
  void renderString3f(float x, float y, float z, float size, QString string);
  int getFontHeight(int size_index);
  int getFontWidth(int size_index, QString string);

  void setMoveCoordinates(QMouseEvent*);
  void moveEvent(QMouseEvent*);
  void sizeEvent(QMouseEvent*);
  void showEvent(QShowEvent * event);

private:
  static Map3dGLWidget* sharedWidget;
};

#endif
