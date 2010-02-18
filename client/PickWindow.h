/* PickWindow.h */

#ifndef PICKWINDOW_H
#define PICKWINDOW_H

#include "GenericWindow.h"

struct PickInfo;
struct FilesDialogRowData;
class Mesh_Info;

class PickWindow: public Map3dGLWidget
{
public:
  PickWindow(QWidget* parent, bool rms);
  ~PickWindow();
  static PickWindow* PickWindowCreate(int _width, int _height, int _x, int _y);

  void initializeGL();
  void paintGL();
  void Destroy();

  virtual void keyPressEvent ( QKeyEvent * event );
  virtual void keyReleaseEvent ( QKeyEvent * event );
  virtual void mouseMoveEvent ( QMouseEvent * event );
  virtual void mousePressEvent ( QMouseEvent * event );
  virtual void mouseReleaseEvent ( QMouseEvent * event );
  virtual void enterEvent( QEvent* event ); 
  virtual void leaveEvent( QEvent* event ); 

  void RMSButtonPress(QMouseEvent* event);
  void RMSButtonRelease(QMouseEvent* event);
  void RMSMotion(QMouseEvent* event);

  int OpenMenu(QPoint point);
  void MenuEvent(int menu_data);

  void DrawNode();
  void SetStyle(int x);


  bool rms;
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


#endif
