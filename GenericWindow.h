/* GenericWindow.h */

#ifndef GENERICWINDOW_H
#define GENERICWINDOW_H

#define GEOMWINDOW 1
#define LEGENDWINDOW 2
#define TIMEWINDOW 3
#define RMSWINDOW 4

typedef struct _GtkWidget GtkWidget;
typedef struct _GdkGLContext GdkGLContext;
typedef struct _GdkGLDrawable GdkGLDrawable;
typedef struct _GdkEventMotion GdkEventMotion;

#define HORIZ (map3d_info.borderWidth + map3d_info.borderWidth)
#define VERTICAL (map3d_info.borderWidth + map3d_info.borderWidth + map3d_info.titleHeight)


class GenericWindow
{
public:
  GenericWindow();
  ~GenericWindow();
  void genericInit();
  void destroy();
  virtual void setupEventHandlers() = 0;
  int parentid;                 /* the id of this window's parent */
  int winid;                    /* this window's id */
  int x, y;                     /* upper-left corner in screen coordinates */
  int width, height;            /* dimensions */
  float bgcolor[4];             /* background color */
  float fgcolor[4];             /* foreground color */
  char showinfotext;            /* show window content information */
  char showfullinfotext;        /* show full window content information */

  int curx, cury;               /* the current mouse coordinates */
  int relx, rely;               /* the mouse coordinates relative to the window that started the movement */
  int button;                   /* the current mouse button */
  int state;                    /* the current mouse button state */
  int modifiers;                /* the current modifier keys which aee pressed */

  bool initialized;  // has the physical window been created?
  bool ready;        // are we actually using the window (putting stuff in it)?
  

  int startx, starty; // for moving, sizing windows
  int wintype;

  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *drawarea;
  GtkWidget *menu;

};

class GLWindow : public GenericWindow {
public:
  GLWindow(int type, const char* title, int min_width, int min_height, bool rms=false);
  bool standardInitialize();
  void popWindow();
  void setPosAndShow();
  void setPopLevel();
  void moveWindow(GdkEventMotion * event);
  void sizeWindow(GdkEventMotion * event);
  void positionWindow(int _width, int _height, int _x, int _y, int def_width, int def_height);
  void setMinSize(int width, int height);
  void getCommandLineCoordinates(int& _width, int& _height, int& _x, int& _y);
  GdkGLContext *glcontext;
  GdkGLDrawable *gldrawable;
  int poplevel; // for saving windows - smaller numbers are "on top" of bigger ones
  int min_width, min_height;

  // this so far is only for legend windows, when you specify -slw 0
  bool startHidden;
};

void makeGLShareContext();

#endif
