/* PickWindow.cxx */

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#ifdef OSX
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <float.h>

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include "PickWindow.h"
#include "Contour_Info.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "WindowManager.h"
#include "dialogs.h"
#include "ContourDialog.h"
#include "eventdata.h"
#include "glprintf.h"
#include "map3d-struct.h"
#include "pickinfo.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "scalesubs.h"
#include "savescreen.h"
#include "MainWindow.h"
#include <gdk/gdkkeysyms.h>

#define PICK_INSIDE  0
#define PICK_OUTSIDE 1
#define PICK_LEFT    2
#define PICK_RIGHT   3

extern Map3d_Info map3d_info;
extern char *units_strings[5];
extern int fstep, cur, fstart, fend;
extern int key_pressed;
extern int pick;
extern int delay;
extern GdkGLConfig *glconfig;
extern MainWindow *masterWindow;
extern FilesDialog *filedialog;
extern SaveDialog *savedialog;

PickInfo *pickstack[100] = { 0 };
int pickstacktop = -1;

#define CHAR_SIZE 70
#define CHAR_BIG CHAR_SIZE * 4
#define CHAR_MED CHAR_SIZE * 3
//#define PROJ_SIZE 1000.f


PickWindow::PickWindow(bool rms) : GLWindow((rms?RMSWINDOW:TIMEWINDOW),"Time Signal", 260, 120, rms)
{
  setupEventHandlers();

  axiscolor[0] = .75;
  axiscolor[1] = .75;
  axiscolor[2] = .1f;
  axiscolor[3] = 1;
  
  graphcolor[0] = .1f;
  graphcolor[1] = .75f;
  graphcolor[2] = .1f;
  graphcolor[3] = 1.0f;
  graph_width = 2;
  
  if (wintype == TIMEWINDOW) {
    PickWindowStyle(this, 1);
  }
  else {
    PickWindowStyle(this, 0);
  }
  
  menu = gtk_menu_new();
  AddMenuEntry(menu, "Axes Color", axes_color, this, PickWindowMenu);
  AddMenuEntry(menu, "Graph Color", graph_color, this, PickWindowMenu);
  AddMenuEntry(menu, "Toggle Display Mode", full_screen, this, PickWindowMenu);
  AddMenuEntry(menu, "Graph Width", graph_width_menu, this, PickWindowMenu);
}

void PickWindow::setupEventHandlers()
{
  if (wintype != RMSWINDOW) {
    if (masterWindow == NULL) {
      g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(PickWindowKeyboardPress), this);
      g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(PickWindowKeyboardRelease), this);
      g_signal_connect_swapped(G_OBJECT(window), "delete_event", G_CALLBACK(PickWindowDestroy),
                               this);
      g_signal_connect(G_OBJECT(window), "window_state_event", G_CALLBACK (window_state_callback), NULL);
    }
    
    gtk_widget_set_events(drawarea, GDK_EXPOSURE_MASK | GDK_BUTTON_MOTION_MASK |
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_VISIBILITY_NOTIFY_MASK |
                          GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    
    
    g_signal_connect(G_OBJECT(drawarea), "expose_event", G_CALLBACK(PickWindowRepaint), this);
    g_signal_connect(G_OBJECT(drawarea), "button_press_event", G_CALLBACK(PickWindowButtonPress), this);
    g_signal_connect(G_OBJECT(drawarea), "button_release_event", G_CALLBACK(PickWindowButtonRelease), this);
    g_signal_connect(G_OBJECT(drawarea), "motion_notify_event", G_CALLBACK(PickWindowMotion), this);
    g_signal_connect(G_OBJECT(drawarea), "enter_notify_event", G_CALLBACK(PickWindowEnter), this);
    g_signal_connect(G_OBJECT(drawarea), "leave_notify_event", G_CALLBACK(PickWindowLeave), this);
    g_signal_connect(G_OBJECT(drawarea), "configure_event", G_CALLBACK(PickWindowReshape), this);
  }
  else {
    gtk_widget_set_events(drawarea, GDK_EXPOSURE_MASK | GDK_BUTTON_MOTION_MASK |
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_VISIBILITY_NOTIFY_MASK |
                          GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    
    g_signal_connect(G_OBJECT(drawarea), "expose_event", G_CALLBACK(PickWindowRepaint), this);
    g_signal_connect(G_OBJECT(drawarea), "button_press_event", G_CALLBACK(RMSPickWindowButtonPress), this);
    g_signal_connect(G_OBJECT(drawarea), "motion_notify_event", G_CALLBACK(RMSPickWindowMotion), this);
    g_signal_connect(G_OBJECT(drawarea), "configure_event", G_CALLBACK(PickWindowReshape), this);
    
  }
}

//static
PickWindow* PickWindow::PickWindowCreate(int _width, int _height, int _x, int _y, int def_width, int def_height)
{
  PickWindow* win = map3d_info.pickwins[map3d_info.numPickwins++];
  win->parentid = map3d_info.parentid;
  win->positionWindow(_width, _height, _x, _y, def_width, def_height);
  return win;
}

void PickWindowInit(GtkWidget *, GdkEvent *, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  priv->pick->show = 1;
  if (!priv->standardInitialize())
    return;
  gdk_gl_drawable_gl_begin(priv->gldrawable, priv->glcontext);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glShadeModel(GL_FLAT);
  
  glDepthMask(GL_FALSE);

  gdk_gl_drawable_gl_end(priv->gldrawable);

  if (priv->mesh && priv->mesh->gpriv) {
    GeomWindow* geom = (GeomWindow *) priv->mesh->gpriv;
    priv->bgcolor[0] = geom->bgcolor[0];
    priv->bgcolor[1] = geom->bgcolor[1];
    priv->bgcolor[2] = geom->bgcolor[2];
    priv->bgcolor[3] = geom->bgcolor[3];
    
    priv->fgcolor[0] = geom->fgcolor[0];
    priv->fgcolor[1] = geom->fgcolor[1];
    priv->fgcolor[2] = geom->fgcolor[2];
    priv->fgcolor[3] = geom->fgcolor[3];
  }
  else {
    priv->bgcolor[0] = priv->bgcolor[1] = priv->bgcolor[2] = priv->bgcolor[3] = 0;
    priv->fgcolor[0] = priv->fgcolor[1] = priv->fgcolor[2] = priv->fgcolor[3] = 1;
  }
  
  gtk_widget_queue_draw(priv->drawarea);
}

void PickWindowDestroy(PickWindow * priv)
{
  int i, j = -1;
  Mesh_Info* mesh = priv->mesh;
  
  for (i = 0; i <= mesh->pickstacktop; i++) {
    if (mesh->pickstack[i]->pickwin == priv) {
      delete mesh->pickstack[i];
      mesh->pickstacktop--;
      for (j = i; j <= mesh->pickstacktop; j++)
        mesh->pickstack[j] = mesh->pickstack[j + 1];
      break;
    }
  }
  for (i = 0; i < map3d_info.numPickwins; i++) {
    if (map3d_info.pickwins[i] == priv) {
      map3d_info.numPickwins--;
      for (j = i; j < map3d_info.numPickwins; j++)
        map3d_info.pickwins[j] = map3d_info.pickwins[j + 1];
      break;
    }
  }
  if (j != -1) {
    map3d_info.pickwins[j] = priv;
    DestroyWindow(priv);
  }
  
}

void PickWindowRepaint(GtkWidget * widget, GdkEvent * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  if (!priv->ready)
    return;
  
  // this seems kind of hacky, but it may not
  // have been done yet...
  if (!priv->initialized)
    PickWindowReshape(widget, NULL, data);
  
  GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
  
  if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
    printf("Can't find where I should be drawing!\n");
    return;
  }
  
  glClearColor(priv->bgcolor[0], priv->bgcolor[1], priv->bgcolor[2], 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  switch (priv->pick->type) {
    case 1:                      /* node */
      if (priv->pick->show)
        PickWindowDrawNode(widget, event, data);
      break;
    case 2:                      /* segment */
      break;
    case 3:                      /* triangle */
      break;
    case 4:                      /* tetra */
      break;
    default:
      break;
  }
  
  if (gdk_gl_drawable_is_double_buffered(gldrawable))
    gdk_gl_drawable_swap_buffers(gldrawable);
  else
    glFlush();
  
  gdk_gl_drawable_gl_end(gldrawable);
}

void PickWindowReshape(GtkWidget *, GdkEvent * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  if (!priv->ready)
    return;
  
  GtkWidget* widget = priv->drawarea;
  PickWindowInit(widget, event, data);
  
  priv->width = widget->allocation.width;
  priv->height = widget->allocation.height;
  
  gdk_gl_drawable_gl_begin(priv->gldrawable, priv->glcontext);
  glViewport(0, 0, priv->width, priv->height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  //projection in pixels
  gluOrtho2D(0, priv->width, 0, priv->height);
  glMatrixMode(GL_MODELVIEW);
  gdk_gl_drawable_gl_end(priv->gldrawable);
  gtk_widget_queue_draw(priv->drawarea);
  
}

void PickWindowEnter(GtkWidget *, GdkEventCrossing * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  if (priv == 0)
    return;
  
  Mesh_Info* mesh = priv->mesh;
  if (event->detail != GDK_NOTIFY_INFERIOR) // window crossing, not crossing widget boundaries
  {
    mesh->curpicknode = priv->pick->node;
    gtk_widget_queue_draw(mesh->gpriv->drawarea);
  }
}

void PickWindowLeave(GtkWidget *, GdkEventCrossing * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  if (priv == 0)
    return;
  
  Mesh_Info* mesh = priv->mesh;
  if (event->detail != GDK_NOTIFY_INFERIOR) // window crossing, not crossing widget boundaries
  {
    mesh->curpicknode = -1;
    gtk_widget_queue_draw(mesh->gpriv->drawarea);
  }
}

void PickWindowMenu(menu_data * data)
{
  int menu = data->data;
  PickWindow *priv = (PickWindow *) data->priv;
  
  switch (menu) {
    case axes_color:
      PickColor(priv->axiscolor);
      break;
    case graph_color:
      PickColor(priv->graphcolor);
      break;
    case graph_width_menu:
      PickSize(&priv->graph_width, 10, "Graph Width");
      break;
    case full_screen:
      if (priv->showinfotext && !priv->showfullinfotext){
        PickWindowStyle(priv, 2);
      }
      else if(priv->showfullinfotext && priv->showfullinfotext){
        PickWindowStyle(priv, 0);
      }
      else{
        PickWindowStyle(priv, 1);
      }
      gtk_widget_queue_draw(priv->drawarea);
      break;
  }
}

// in here we hack the original values of event->x and event->y
void PickWindowButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  
  PickWindow* priv = (PickWindow*) data;
  Mesh_Info *mesh = priv->mesh;
  
  if(!mesh)
    return;
  
  // redraw all relevant windows.  The frame should already be selected
  if (priv->click && event->button == 1 && mesh->data) {
    if (!map3d_info.lockframes) {
      // if advancing in time only affects this surface
      if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
          map3d_info.scale_scope != SLAVE_FRAME) {
        GeomWindowUpdateAndRedraw(priv->mesh->gpriv);
      }
      // it affects at least one other surface...
      else {
        Broadcast(MAP3D_UPDATE, widget, (GdkEvent *) event, data);
      }
    }
    else 
      Broadcast(MAP3D_UPDATE, widget, (GdkEvent *) event, data);
    
    // if animating, save since we redrew the new frame
    if (map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_frame_advance))) {
      SaveScreen();
    }
  }
}



// in here we hack the original values of event->x and event->y
void PickWindowButtonPress(GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  priv->state = 1;
  SetCurrentWindow(priv->winid);
  Mesh_Info *mesh = priv->mesh;
  
  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }
  
  
  if (masterWindow != NULL) {
    masterWindow->focus = widget;
    gtk_widget_grab_focus(widget);
    
    if (COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK)) {
      priv->popWindow();
    }
  }
  else
    priv->setPopLevel();
  
  if(!mesh)
    return;
  
  float distance;
  priv->button = event->button;
  int x = (int)event->x;
  int y = (int)(priv->height - event->y);
  
  priv->click = false;
  /* LEFT MOUSE DOWN + ALT = start move window */
  priv->startx = (int)event->x_root;
  priv->starty = (int)event->y_root;
  if (COMPARE_MODIFIERS(event->state, (GDK_BUTTON1_MASK | GDK_MOD1_MASK))) {
    priv->startx = (int)event->x_root;
    priv->starty = (int)event->y_root;
  }
  
  /* MIDDLE MOUSE DOWN + ALT = start move window */
  else if (COMPARE_MODIFIERS(event->state, (GDK_BUTTON2_MASK | GDK_MOD1_MASK))) {
    priv->startx = (int)event->x_root;
    priv->starty = (int)event->y_root;
  }
  
  else if (event->button == 3)  // right click - menu popup
    gtk_menu_popup(GTK_MENU(priv->menu), NULL, NULL, NULL, NULL, event->button, event->time);
  
  /* LEFT MOUSE DOWN = select frame in graph */
  else if (event->button == 1 && mesh->data) {
    map3d_info.scale_frame_set = 0;
    if (y < priv->height * priv->topoffset && y > priv->height * priv->bottomoffset &&
        x > priv->width * priv->leftoffset && x < priv->width * priv->rightoffset) {
      priv->click = true;
      distance = (x - priv->width * priv->leftoffset) / (priv->rightoffset - priv->leftoffset) / priv->width;
      distance *= (mesh->data->numframes-1);
      event->x = -(mesh->data->framenum - distance);

      map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP)
        ? mesh->groupid : -1;
      ComputeLockFrameData();

      // clamp delta frames to the lock frame data
      if (event->x < 0 && map3d_info.lockframes) 
        event->x = MAX(event->x, fstart-cur);
      else if (map3d_info.lockframes)
        event->x = MIN(event->x, fend-cur);


      if (!map3d_info.lockframes) {
        mesh->data->FrameAdvance((int)event->x);
        updateContourDialogValues(mesh);
      }
      else 
        Broadcast(MAP3D_PICK_FRAMES, widget, (GdkEvent *) event, data);
    }
    
  }
  priv->curx = x;
  priv->cury = y;
  gtk_widget_queue_draw(priv->drawarea);
  
}

void RMSPickWindowButtonPress(GtkWidget *, GdkEventButton * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  priv->state = 1;
  Mesh_Info *mesh = priv->mesh;
  
  if(!mesh)
    return;
  
  //ComputeLockFrameData();
  float distance;
  priv->button = event->button;
  int x = (int)event->x;
  int y = (int)(priv->height - event->y);
  
  priv->click = false;
  
  if ((event->button == 1) && mesh->data) {
    map3d_info.scale_frame_set = 0;
    if (y < priv->height * priv->topoffset && y > priv->height * priv->bottomoffset &&
        x > priv->width * priv->leftoffset && x < priv->width * priv->rightoffset) {
      priv->click = true;
      distance = (x - priv->width * priv->leftoffset) / (priv->rightoffset - priv->leftoffset) / priv->width;
      distance *= (mesh->data->numframes-1);
      
      // set window_line to 0 if the click is closer to the left line, 1 if closer to right
      int clicked_frame = (int) (distance+1);
      int left_line = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart));
      int right_line = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend));
      priv->window_line = (abs(clicked_frame-left_line) > abs(clicked_frame-right_line)) ? 1 : 0;
      
      if(priv->window_line == 0){
        if(distance > gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend))){
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->datastart), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend)));
        }else{
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->datastart), distance+1);
        }
      }
      if(priv->window_line == 1){
        if(distance < gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart))){
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->dataend), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart)));
        }else{
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->dataend), distance+1);
        }
      }
    }
  }
  
  gtk_widget_queue_draw(priv->drawarea);
  
  //delete Data;
}

void PickWindowMotion(GtkWidget * widget, GdkEventMotion * event, gpointer data)
{
  
  PickWindow *priv = (PickWindow *) data;
  Mesh_Info *mesh = priv->mesh;
  
  if(!mesh)
    return;
  
  float distance;
  int x = (int)event->x;
  int y = (int)(priv->height - event->y);
  event->y = PICK_INSIDE;
  
  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }
   
  //  if (COMPARE_MODIFIERS(event->state,GDK_BUTTON1_MASK))
  if (COMPARE_MODIFIERS(event->state, (GDK_BUTTON1_MASK | GDK_MOD1_MASK))) {
    priv->moveWindow(event);
  }
  else if (COMPARE_MODIFIERS(event->state, (GDK_MOD1_MASK | GDK_BUTTON2_MASK))) 
  {
    priv->sizeWindow(event);
  }
  
  else if (COMPARE_MODIFIERS(event->state, GDK_BUTTON1_MASK) && mesh->data) {
    //x -= priv->width / 10;
    distance = (x - priv->width * priv->leftoffset) / (priv->rightoffset - priv->leftoffset) / priv->width;
    
    distance = distance * (mesh->data->numframes-1);
    
    event->x = -(mesh->data->framenum - distance);
    map3d_info.scale_frame_set = 0;
    
    map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP)
      ? mesh->groupid : -1;
    ComputeLockFrameData();

    if (y < priv->height * priv->topoffset && y > priv->height * priv->bottomoffset &&
        x > priv->width * priv->leftoffset && x < priv->width * priv->rightoffset) {
      priv->click = true; // to signify that we are dragging inside the pick window

      // clamp delta frames to the lock frame data
      if (event->x < 0 && map3d_info.lockframes) 
        event->x = MAX(event->x, fstart-cur);
      else if (map3d_info.lockframes)
        event->x = MIN(event->x, fend-cur);
    }
    
    // we used to be in the frame but we're not anymore, so redraw the relevant windows
    else if (priv->click) {
      event->y = PICK_OUTSIDE;
      priv->click = false;
      //      for (i = 0; i < length; i++) {
      if (x <= priv->width * .1) {
        
        event->y = PICK_LEFT;
        event->x = (map3d_info.lockframes ? fstart - cur : mesh->data->ts_start - mesh->data->framenum);
      }
      else if (x >= priv->width * .95) {
        event->y = PICK_RIGHT;
        event->x = (map3d_info.lockframes ? fend - cur : mesh->data->ts_end - mesh->data->framenum);
      }
      
      if (!map3d_info.lockframes) {
        mesh->data->FrameAdvance((int)event->x);
        // if advancing in time only affects this surface
        if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
            map3d_info.scale_scope != SLAVE_FRAME) {
          GeomWindowUpdateAndRedraw(priv->mesh->gpriv);
        }
        // it affects at least one other surface...
        else {
          Broadcast(MAP3D_UPDATE, widget, (GdkEvent *) event, data);
        }
      }
      else {
        Broadcast(MAP3D_PICK_FRAMES, widget, (GdkEvent *) event, data);
        Broadcast(MAP3D_UPDATE, widget, (GdkEvent *) event, data);
      }
      
      // if animating, save since we redrew the new frame
      if (map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_frame_advance))) {
        SaveScreen();
      }
      return;
      }
    else {
      return;
    }
    
    // update the frames but don't redraw
    if (!map3d_info.lockframes)
      mesh->data->FrameAdvance((int)event->x);
    else
      Broadcast(MAP3D_PICK_FRAMES, widget, (GdkEvent *) event, data);
    gtk_widget_queue_draw(priv->drawarea);
  }
}

void RMSPickWindowMotion(GtkWidget*, GdkEventMotion * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  Mesh_Info *mesh = priv->mesh;
  
  if(!mesh)
    return;
  
  float distance;
  int x = (int)event->x;
  int y = (int)(priv->height - event->y);
  event->y = PICK_INSIDE;
  
  
  if (COMPARE_MODIFIERS(event->state, GDK_BUTTON1_MASK) && mesh->data) {
    //x -= priv->width / 10;
    distance = (x - priv->width * priv->leftoffset) / (priv->rightoffset - priv->leftoffset) / priv->width;
    
    distance = distance * (mesh->data->numframes-1);
    
    
    if (y < priv->height * priv->topoffset && y > priv->height * priv->bottomoffset &&
        x > priv->width * priv->leftoffset && x < priv->width * priv->rightoffset) {
      priv->click = true; // to signify that we are dragging inside the pick window
    }
    
    // we used to be in the frame but we're not anymore, so set frames to max extent
    else if (priv->click) {
      event->y = PICK_OUTSIDE;
      priv->click = false;
      //      for (i = 0; i < length; i++) {
      if (x <= priv->width * .1) {
        
        event->y = PICK_LEFT;
        distance = 0;
      }
      else if (x >= priv->width * .95) {
        event->y = PICK_RIGHT;
        distance = 1;
      }
      
      }
    
    // set the frame vals - window_line was set in ButtonPress based on which line we were closer to
    if(priv->window_line == 0)
    {
      if(distance > gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend))){
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->datastart), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend)));
      }else{
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->datastart), distance+1);
      }
    }
    if(priv->window_line == 1)
    {
      if(distance < gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart))){
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->dataend), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart)));
      }else{
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->dialogRowData->dataend), distance+1);
      }
    }
    }
  
  //delete Data;
  }

void PickWindowKeyboardRelease(GtkWidget *, GdkEventKey * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  GeomWindow *gpriv = priv->mesh->gpriv;
  if (event->keyval == GDK_Left || event->keyval == GDK_Right) {
    GeomWindowKeyboardRelease(gpriv->drawarea, event, (gpointer) gpriv);
  }
}
void PickWindowKeyboardPress(GtkWidget *, GdkEventKey * event, gpointer data)
{
  PickWindow *priv = (PickWindow *) data;
  int key = event->keyval;
  if (event->keyval == 'p') {
    menu_data data(full_screen,priv);
    PickWindowMenu(&data);
  }
  else if (key == 'q') {
    priv->pick->show = 0;
    if (masterWindow)
      gtk_widget_hide(priv->drawarea);
    else
      gtk_widget_hide(priv->window);
  }
  else if (key == GDK_Escape) {
    priv->pick->show = 0;
    PickWindowDestroy(priv);
  }
  else if (key == GDK_Left || key == GDK_Right || key == 'f') {
    GeomWindowKeyboardPress(priv->mesh->gpriv->drawarea, event, priv->mesh->gpriv);
  }
}

void PickWindowHandleKeyboard(key_data * data)
{
  if (HandleKeyboard(data))     // already handled by window manager
    return;
  
  switch (data->key) {
    case 'q':
      //DestroyWindow(priv);
      break;
  }
}

void PickWindowDrawNode(GtkWidget *, GdkEvent *, gpointer arg)
{
  PickWindow *priv = (PickWindow *) arg;
  
  int loop;
  float a, b;
  float d;
  float min = FLT_MAX, max = -FLT_MAX;
  float right = priv->rightoffset;
  float left = priv->leftoffset;
  float top = priv->topoffset;
  float bottom = priv->bottomoffset;
  
  float pos[3] = { 5.f, (float)priv->height, 0 };
  //float norm[3]={0,0,-1}, up[3]={0,1,0};
  //float aspect = .62f * priv->height / priv->width;
  float coloroffset = .5f;
  Mesh_Info *curmesh = priv->mesh;
  
  // this is for the case in the files dialog with an empty row
  if (!curmesh)
    return;
  Surf_Data* data = curmesh->data;  
  
  /* Find the extrema of the time signal */
  if (data && priv->pick->rms) {
    for (loop = 0; loop < data->numframes; loop++) {
    	if (data->rmspotvals[loop] < min)
	      min = data->rmspotvals[loop];
	    if (data->rmspotvals[loop] > max)
	      max = data->rmspotvals[loop];
    }
  }
  else if (data){
    for (loop = 0; loop <data->numframes; loop++) {
    	if (data->potvals[loop][priv->pick->node] < min)
	      min = data->potvals[loop][priv->pick->node];
	    if (data->potvals[loop][priv->pick->node] > max)
	      max = data->potvals[loop][priv->pick->node];
    }
  }
  else {
    min = 0;
    max = 0;
  }
  //ComputeLinearMappingCoefficients(min, max, -.6 * PROJ_SIZE, .5 * PROJ_SIZE, a, b);
  
  a = ((top - bottom) * priv->height) / (max - min);
  b = (bottom * priv->height * max - min * top * priv->height) / (max - min);
  
  
  if (priv->bgcolor[0] + .3 > 1 || priv->bgcolor[1] + .3 > 1 || priv->bgcolor[2] + .3 > 1)
    coloroffset = -.5;
  
  glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);
  
  if (priv->showinfotext) {
    
    pos[1] = priv->height - (2+getFontWidth(priv->mesh->gpriv->med_font))*1.5f;
    
    
    if (data) {
      // print real frame num if start is not beginning
      if (data->ts_start != 0 || data->ts_sample_step != 1)
        renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, 
                       "Frame: %d (%d)   Time: %dms", data->framenum + 1,
                       data->framenum * data->ts_sample_step + 
                       data->ts_start + 1,
                       (data->framenum - data->zerotimeframe));
      else
        renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, 
                       "Frame: %d   Time: %dms", data->framenum + 1,
                       (data->framenum - data->zerotimeframe));
    }
    else {
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "Frame: ---");
    }
    
    pos[0] = priv->width - 14 * getFontWidth(priv->mesh->gpriv->med_font);
    if (data) {
      
      if(priv->pick->rms){
        renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, " Value: %6.3f",
                       data->rmspotvals[data->framenum]);
      }
      else{
        renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, " Value: %6.3f",
                       
                       data->potvals[data->framenum][priv->pick->node]);
      }
      
    }
    else {
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "Value: ---");
      
    }
    
    if(priv->showfullinfotext){
      pos[0] = 5;
      pos[1] = 20;
    }else{
      pos[0] = 5;
      pos[1] = 3;
    }
    
    if(priv->pick->rms){
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "RMS");
    }
    else{
      if (curmesh->geom->channels)
        //fix channel printing
        renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "Node# %d (Ch %d)",
                       priv->pick->node + 1, curmesh->geom->channels[priv->pick->node] + 1);
      else
        renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, 
                       "Node# %d", priv->pick->node + 1);
    }
    pos[0] = priv->width - 14 * getFontWidth(priv->mesh->gpriv->med_font);
    
    if (curmesh->geom->subsurf <= 0)
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, 
                     "Surface# %d", curmesh->geom->surfnum);
    else
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, 
                     "Surface# %d-%d", curmesh->geom->surfnum, curmesh->geom->subsurf);
    
    
    if(priv->showfullinfotext){
      pos[0] = 5;
      pos[1] = 3;
      //pos[1] = 10;
      
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "%s",
                     shorten_filename(data->potfilename));
      switch(map3d_info.scale_mapping){
        case 0:
          pos[0] = priv->width - 15 * getFontWidth(priv->mesh->gpriv->med_font);
          renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "Map: Symmetric");
          break;
        case 1:
          pos[0] = priv->width - 14 * getFontWidth(priv->mesh->gpriv->med_font);
          renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "Map: Seperate");
          break;
        case 2:
          pos[0] = priv->width - 14 * getFontWidth(priv->mesh->gpriv->med_font);
          renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "Map: True");
          break;
      }
      
    }
    
    
    
    /* axis labels */
    pos[0] = priv->width/2 - 2*getFontWidth(priv->mesh->gpriv->med_font);
    if(priv->showfullinfotext){
      pos[1] = priv->height/3.5f;
      
    }else{
      pos[1] = priv->height/7.5f;
    }
    renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "Time");
    //      glprintf(pos,norm,up,CHAR_MED*aspect,CHAR_MED,"Time");
    pos[0] = 2;
    pos[1] = b;
    if (data && data->units != 0) {
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "%s", 
                     units_strings[data->units - 1]);
    }
    else {
      renderString3f(pos[0], pos[1], pos[2], priv->mesh->gpriv->med_font, "data");
      
    }
    
  }  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  /* draw vertical axis line, and zero horizontal line */
  glLineWidth(1);
  glColor3f(priv->axiscolor[0], priv->axiscolor[1], priv->axiscolor[2]);
  glBegin(GL_LINES);
  if (data) {
    d = priv->width / (float)(data->numframes-1) * (right - left); //graph's domain
    
    glVertex3f(left * priv->width + d*(float)data->zerotimeframe, top * priv->height, 0);
    glVertex3f(left * priv->width + d*(float)data->zerotimeframe, bottom * priv->height, 0);
    glVertex3f((left - .02f) * priv->width, b, 0);
    glVertex3f((right + .02f) * priv->width, b, 0);
    glEnd();
  }
  else{
    glVertex3f(left * priv->width, top * priv->height, 0);
    glVertex3f(left * priv->width, bottom * priv->height, 0);
    glVertex3f((left - .02f) * priv->width, b, 0);
    glVertex3f((right + .02f) * priv->width, b, 0);
    glEnd();
  }
  /* draw time signal */
  if (data) {
    d = priv->width / (float)(data->numframes-1) * (right - left); //graph's domain
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(priv->graph_width);
    glColor3f(priv->graphcolor[0], priv->graphcolor[1], priv->graphcolor[2]);
    glBegin(GL_LINE_STRIP);
    for (loop = 0; loop < data->numframes; loop++) {
      if(priv->pick->rms){
        glVertex3f(left * priv->width + d * loop, data->rmspotvals[loop] * a + b, 0);
      }
      else{
        glVertex3f(left * priv->width + d * loop, data->potvals[loop][priv->pick->node] * a + b, 0);
      }
    }
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    
    
    //draw fiducial markers
    if(priv->wintype == TIMEWINDOW){
      int index = 0;
      for(int fidsets = 0; fidsets < data->numfs; fidsets++){
//        printf("fidsets %d\n", fidsets);
//        printf("curmesh->fidConts.size() %d\n",curmesh->fidConts.size());
//        printf("1data->fids[fidsets].numfidleads %d\n", data->fids[fidsets].numfidleads);

        if(priv->pick->node < data->fids[fidsets].numfidleads){
//          printf("2data->fids[fidsets].numfidleads %d\n", data->fids[fidsets].numfidleads);
          for(int i = 0; i < data->fids[fidsets].leadfids[priv->pick->node].numfids;i++){
            short fidType = data->fids[fidsets].leadfids[priv->pick->node].fidtypes[i];
            for(unsigned j = 0; j<curmesh->fidConts.size();j++){
              if(curmesh->fidConts[j]->datatype == fidType)
                index = j;
            }
            glLineWidth(1);
            glColor3f(curmesh->fidConts[index]->fidcolor.red/65535.0f,
                      curmesh->fidConts[index]->fidcolor.green/65535.0f,
                      curmesh->fidConts[index]->fidcolor.blue/65535.0f);
            glBegin(GL_LINES);
            glVertex3f(left * priv->width + d * data->fids[fidsets].leadfids[priv->pick->node].fidvals[i], b+(.1f * priv->height), 0);
            glVertex3f(left * priv->width + d * data->fids[fidsets].leadfids[priv->pick->node].fidvals[i], b - (.1f * priv->height), 0);
            glEnd();
//            printf("index %d i %d\n", index, i);
            index++;
          }
        }else{index+=data->fids[fidsets].numfidtypes;}
      }
    }
    
    
    
    /* draw vertical frame line */
    
    if(priv->wintype == TIMEWINDOW){
      glLineWidth(1);
      glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
      glBegin(GL_LINES);
      glVertex3f(left * priv->width + d * (float)data->framenum, (top + .02f) * priv->height, 0);
      glVertex3f(left * priv->width + d * (float)data->framenum, (bottom - .02f) * priv->height, 0);
      glEnd();
    }
    
    if(priv->wintype == RMSWINDOW){
      glLineWidth(1);
      glColor3f(0, 1, 0);
      glBegin(GL_LINES);
      //int x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart)) - 1;
      //int y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend)) - 1;
      
      glVertex3f(left * priv->width + d * (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart)) - 1), (top + .02f) * priv->height, 0);
      glVertex3f(left * priv->width + d * (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->datastart)) - 1), (bottom - .02f) * priv->height, 0);
      glColor3f(1, 0, 0);
      glVertex3f(left * priv->width + d * (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend)) - 1), (top + .02f) * priv->height, 0);
      glVertex3f(left * priv->width + d * (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->dialogRowData->dataend)) - 1), (bottom - .02f) * priv->height, 0);
      glEnd();
    }
  }
}

void PickWindowStyle(PickWindow * priv, int x)
{
  switch (x) {
    case 0:                      //full size
      priv->showinfotext = 0;
      priv->showfullinfotext = 0;
      
      priv->leftoffset = priv->bottomoffset = .025f;
      priv->topoffset = priv->rightoffset = .975f;
      break;
      
    case 1:                      //details
      priv->showinfotext = 1;
      priv->showfullinfotext = 0;
      
      priv->leftoffset = 0.1f;
      priv->rightoffset = 0.95f;
      priv->topoffset = .83f;
      priv->bottomoffset = .27f;
      break;
      
    case 2:                      //Full details
      priv->showinfotext = 1;
      priv->showfullinfotext = 1;
      //priv->setMinSize(240,180);
      priv->leftoffset = 0.1f;
      priv->rightoffset = 0.95f;
      priv->topoffset = .83f;
      priv->bottomoffset = .40f;
      break;
      
      
      
  }
  
}
