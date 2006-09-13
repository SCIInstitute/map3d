#include "GenericWindow.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "MainWindow.h"
#include "WindowManager.h"
#include "eventdata.h"
#include "map3d-struct.h"
#include "glprintf.h"

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <stdio.h>

extern MainWindow *masterWindow;
extern GdkGLConfig *glconfig;
extern Map3d_Info map3d_info;


GenericWindow::GenericWindow()
{
  winid = -1;
}

GenericWindow::~GenericWindow()
{
  
}

// set the original position/size and show
void GLWindow::setPosAndShow()
{
  if (startHidden)
    return;
  if (masterWindow) {
    gtk_fixed_move(GTK_FIXED(masterWindow->fixed), drawarea, x, y);
    gtk_widget_set_size_request(drawarea, width, height);
    ready = true;
    gtk_container_resize_children(GTK_CONTAINER(masterWindow->fixed));
  }
  else {
    gtk_window_move(GTK_WINDOW(window),x, y);
    gtk_window_resize(GTK_WINDOW(window),width, height);
    ready = true;
    gtk_widget_show_now((window));
    gtk_window_present(GTK_WINDOW(window));
  }
}
void GenericWindow::setupEventHandlers()
{
  printf("ABS VIRT\n");
}

GLWindow::GLWindow(int type, const char* title, int min_width, int min_height, bool rms) 
  : GenericWindow(), min_width(min_width), min_height(min_height), startHidden(false)
{
  wintype = type;
  width=0; height=0;
  x=0; y=0;
  poplevel = 9999;
  
  initialized = false;
  ready = false;

  drawarea = gtk_drawing_area_new();
  if (map3d_info.share_widget == NULL) {
    makeGLShareContext();
  }
  gtk_widget_set_gl_capability(drawarea, glconfig, gtk_widget_get_gl_context(map3d_info.share_widget),
                               TRUE, GDK_GL_RGBA_TYPE);

  if (masterWindow && !rms) {
    gtk_fixed_put(GTK_FIXED(masterWindow->fixed), drawarea, x, y);
  }
  else
    if(!rms){
      window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title(GTK_WINDOW(window), title);
      box = gtk_vbox_new(FALSE, 0);
      gtk_container_add(GTK_CONTAINER(window), box);
      gtk_widget_show(box);

      gtk_widget_set_size_request(drawarea, min_width, min_height);
      gtk_window_set_default_size(GTK_WINDOW(window), width, height);
      gtk_window_move((GtkWindow *) window, x, y);
      gtk_box_pack_start(GTK_BOX(box), drawarea, TRUE, TRUE, 0);
      gtk_widget_show(drawarea);
    }
  // gtk wants to call the reshape event once we call show.  That will need some stuff
  // from the opengl context, done in init.  However, if we just call init, the opengl
  // context won't be created yet and we'll get errors.  So we have Reshape call init.
}

// gets called at top of specific window's Init function, common init stuff
// returns true if it did its job
bool GLWindow::standardInitialize()
{
  if (initialized)
    return false;
  
  if (winid == -1 && wintype != RMSWINDOW) {
    winid = AssociateWindow(this);
    if (wintype == GEOMWINDOW)
      AssociateGeomWindow((GeomWindow*)this);
  }
  if (wintype != RMSWINDOW)
    setPopLevel();

  initialized = true;

  glcontext = gtk_widget_get_gl_context(drawarea);
  gldrawable = gtk_widget_get_gl_drawable(drawarea);
  return true;
}

// this one is to put one "borderless window" on top of the rest
void GLWindow::popWindow()
{
  // this needs to be in this exact order
  if (masterWindow && wintype != RMSWINDOW) {
    gtk_container_remove(GTK_CONTAINER(masterWindow->fixed), drawarea);
    gtk_widget_destroy(drawarea);
    initialized = false;
    ready = false;
    drawarea = gtk_drawing_area_new();
    gtk_widget_set_gl_capability(drawarea, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
    setupEventHandlers();
    gtk_fixed_put(GTK_FIXED(masterWindow->fixed), drawarea, x, y);
    gtk_widget_show(drawarea);
    ready = true;
    gtk_widget_set_size_request(drawarea, width, height);
  }
}

// set this window's poplevel to 1, and increment the others
// this should generate a unique, incremental poplevel for each window
void GLWindow::setPopLevel()
{
  int old_poplevel = poplevel;
  for (unsigned i = 0; i < numWindows(); i++) {
    GLWindow* win = (GLWindow*) GetWindow(i);
    if (!win || win == this || win->poplevel > old_poplevel)
      continue;
    win->poplevel++;
  }
  poplevel = 1;
//  for (unsigned i = 0; i < numWindows(); i++) {
//    GLWindow* win = (GLWindow*) GetWindow(i);
//    if (win)
//      printf("%d: poplevel: %d\n", i, win->poplevel);
//  }
//  printf("\n");
}

void GLWindow::positionWindow(int _width, int _height, int _x, int _y, int def_width, int def_height)
{

  if (_width > 0 && _height > 0) {
    //to compensate for borders on composition window
    if (map3d_info.qnoborders) {
      _y += map3d_info.globalBorderWidth*2 + map3d_info.globalTitleHeight;
    }
    // these comparisons try to compensate for windows being bigger than the screen
    // or windows that go off the screen
    if (_width > map3d_info.screenWidth) {
      _width -= (_width - map3d_info.screenWidth + HORIZ);
    }
    if (_height > map3d_info.screenHeight) {
      _height -= (_height - map3d_info.screenHeight + VERTICAL);
    }
    if (_x + _width > map3d_info.screenWidth) {
      _x -= (_x + _width - map3d_info.screenWidth);
    }
    if (_y + _height > map3d_info.screenHeight) {
      _y -= (_y + _height - map3d_info.screenHeight);
    }

    this->x = _x + map3d_info.borderWidth;
    this->y = map3d_info.screenHeight - (_y + _height) - map3d_info.borderWidth;  // height had VERTICAL taken out when it was passed in
    this->width = _width + 1;
    this->height = _height + 1;

  }
  else {

    //default position for next window - wrap around the screen if it won't fit
    if (map3d_info.posx + def_width - map3d_info.borderWidth > map3d_info.screenWidth) {
      map3d_info.posx = map3d_info.wrap_flag * (map3d_info.globalBorderWidth + map3d_info.globalTitleHeight);
      map3d_info.posy += def_height;
    }
    if (map3d_info.posy + def_height - map3d_info.titleHeight - map3d_info.borderWidth > map3d_info.screenHeight) {
      // here use absolute border width so the windows cascade even with borderless windows
      map3d_info.wrap_flag++;
      map3d_info.posy = map3d_info.wrap_flag * (map3d_info.globalBorderWidth + map3d_info.globalTitleHeight);
      map3d_info.posx = map3d_info.wrap_flag * (map3d_info.globalBorderWidth + map3d_info.globalTitleHeight);
    }
    this->x = map3d_info.posx + map3d_info.borderWidth;
    this->y = map3d_info.posy + map3d_info.borderWidth + map3d_info.titleHeight;
    this->width = def_width - HORIZ;
    this->height = def_height - VERTICAL;
    map3d_info.posx += def_width;
  }

  startx = x;
  starty = y;
  map3d_info.borderless_xmin = MIN(this->x, map3d_info.borderless_xmin);
  map3d_info.borderless_xmax = MAX(this->x + this->width, map3d_info.borderless_xmax);
  map3d_info.borderless_ymin = MIN(this->y, map3d_info.borderless_ymin);
  map3d_info.borderless_ymax = MAX(this->y + this->height, map3d_info.borderless_ymax);
}

// standardize these...
void GLWindow::moveWindow(GdkEventMotion * event)
{
  if (GetCurrentWindow() != winid)
    return;
  x += (int)event->x_root - startx;
  y += (int)event->y_root - starty;
  startx = (int)event->x_root;
  starty = (int)event->y_root;

  if (masterWindow != NULL) {
    // sometimes if you move over another window, the other window will zoom into oblivion
    // try to compensate for that
    if (x < 0)
      x = 0;
    if (x >= map3d_info.screenWidth-1)
      x = map3d_info.screenWidth-10;
    if (y < 0)
      y = 0;
    if (y >= map3d_info.screenHeight-1)
      y = map3d_info.screenHeight-10;
    gtk_fixed_move(GTK_FIXED(masterWindow->fixed), drawarea, x, y);
    // call to resize children is so the children draw in a logical manner
    gtk_container_resize_children(GTK_CONTAINER(masterWindow->fixed));
  }
  else
    gtk_window_move(GTK_WINDOW(window),x, y);

}

void GLWindow::sizeWindow(GdkEventMotion * event)
{
  if (GetCurrentWindow() != winid)
    return;
  width = width + (int)event->x_root - startx;
  height = height + (int)event->y_root - starty;
  if (width < min_width) width = min_width;
  if (height < min_height) height = min_height;
  startx = (int)event->x_root;
  starty = (int)event->y_root;

  if (masterWindow != NULL) {
    gtk_widget_set_size_request(drawarea, width, height);
    // call to resize children is so the children draw in a logical manner
    gtk_container_resize_children(GTK_CONTAINER(masterWindow->fixed));
  }
  else
    gtk_window_resize(GTK_WINDOW(window),width, height);
}

void GLWindow::setMinSize(int width, int height){
  gtk_widget_set_size_request(drawarea, width, height);
  min_width = width;
  min_height = height;
}


void GLWindow::getCommandLineCoordinates(int& _width, int& _height, int& _x, int& _y)
{
  _width = width+HORIZ-1;
  _x = x - map3d_info.borderWidth;

  _height = height-1;

  _y = map3d_info.screenHeight - y - map3d_info.borderWidth - _height;

  if (map3d_info.qnoborders)
    _y -= (map3d_info.globalBorderWidth*2 + map3d_info.globalTitleHeight);

  _height = _height+VERTICAL;

}

void makeGLShareContext()
{
  GtkWidget* widget = gtk_drawing_area_new();
  gtk_widget_set_gl_capability(widget, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);

  GtkWidget* window = gtk_window_new(GTK_WINDOW_POPUP);

  // the shared widget must be realized before other widgets can share from it.  
  // (and the other widgets must not yet be realized)
  gtk_widget_set_size_request(widget, 0,0);
  gtk_window_set_default_size(GTK_WINDOW(window), 0,0);
  gtk_container_add(GTK_CONTAINER(window), widget);
  gtk_window_move((GtkWindow *) window, 0,0);
  gtk_widget_show(widget);
  gtk_widget_show(window);
  gtk_widget_hide(window);

  map3d_info.share_widget = widget;
  GdkGLContext* context = gtk_widget_get_gl_context(widget);
  GdkGLDrawable* drawable = gtk_widget_get_gl_drawable(widget);
  gdk_gl_drawable_gl_begin(drawable, context);
  initFontsAndTextures();
  gdk_gl_drawable_gl_end(drawable);

}