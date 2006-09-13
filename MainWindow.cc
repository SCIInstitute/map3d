/* MainWindow.cxx */

#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#ifdef OSX
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#include "MainWindow.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "GeomWindowMenu.h"
#include "WindowManager.h"
#include "dialogs.h"
#include "glprintf.h"
#include "map3d-struct.h"
#include "eventdata.h"

extern GdkGLConfig *glconfig;

MainWindow *masterWindow = NULL;
//extern GtkWidget *fixed;

MainWindow::MainWindow() : GenericWindow()
{
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Map3D");
  //gtk_window_set_resizable(GTK_WINDOW(window), true);

  fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);
  gtk_widget_show(fixed);

  //gtk_container_set_resize_mode(GTK_CONTAINER(window), GTK_RESIZE_PARENT);
  gtk_container_set_reallocate_redraws(GTK_CONTAINER(fixed), TRUE);
  gtk_widget_set_redraw_on_allocate(fixed, TRUE);

  x = y = 40;

  //drawarea = gtk_drawing_area_new();
  //gtk_widget_set_size_request(drawarea, width, height);
  //gtk_widget_set_gl_capability(drawarea, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
  //gtk_fixed_put(GTK_FIXED(fixed), drawarea, 0, 0);
  //gtk_widget_show(drawarea);

  GdkColor color;

  color.red = 0;
  color.blue = 0;
  color.green = 0;

  gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

  //winid = AssociateWindow(masterWindow);

  bgcolor[0] = bgcolor[1] = bgcolor[2] = 0;
  fgcolor[0] = fgcolor[1] = fgcolor[2] = 1;
  textLines = 0;
  startHidden = false;
  //initialized = false;
  setupEventHandlers();
  buildMenus();
}

void MainWindow::setupEventHandlers()
{
  gtk_widget_set_events(window, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
                        GDK_VISIBILITY_NOTIFY_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

  g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(MainWindowReshape), this);
  g_signal_connect(G_OBJECT(fixed), "configure_event", G_CALLBACK(MainWindowReshape), this);
  g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(MainWindowKeyboardPress), NULL);
  g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(MainWindowKeyboardRelease), NULL);
  g_signal_connect_swapped(G_OBJECT(window), "delete_event", G_CALLBACK(map3d_quit),
                           window);
  g_signal_connect(G_OBJECT(window), "button_press_event", G_CALLBACK(MainWindowMouseButton),
                   this);

}

void MainWindowRepaint(GtkWidget *, GdkEvent *, gpointer /*data*/)
{
/*  MainWindow *priv = (MainWindow *) data;

  if (!gdk_gl_drawable_gl_begin(priv->gldrawable, priv->glcontext)) {
    printf("Can't find where I should be drawing!\n");
    return;
  }

  glClearColor(priv->bgcolor[0], priv->bgcolor[1], priv->bgcolor[2], 1);
  glClear(GL_COLOR_BUFFER_BIT);
  if (priv->showinfotext)
    for (int i = 0; i < priv->textLines; i++) {
      glColor3fv(priv->fgcolor);
      renderString3f(priv->width / 2 - strlen(priv->mainWindowText[i]) / 2 * getFontWidth((int)map3d_info.large_font),
                     priv->height - (25 * i + 25), 0, (int)map3d_info.large_font, priv->mainWindowText[i]);
    }

  if (gdk_gl_drawable_is_double_buffered(priv->gldrawable))
    gdk_gl_drawable_swap_buffers(priv->gldrawable);
  else
    glFlush();
  gdk_gl_drawable_gl_end(priv->gldrawable);
  */
}

void MainWindowMouseButton(GtkWidget *, GdkEventButton * event, gpointer data)
{
  MainWindow *priv = (MainWindow *) data;
  if (event->button == 3)       // right click
  {
    GtkFixed *myfixed = (GtkFixed *) masterWindow->fixed;
    GList *gl = myfixed->children;
    int size = g_list_length(gl);
    bool popmenu = true;

    for (int i = 0; i < size; i++) {
      GtkFixedChild *child = (GtkFixedChild *) g_list_nth_data(gl, i);
      GenericWindow *draw = FindWidgetOwner(child->widget);
      if (!draw)
        continue;
      if (event->x >= child->x && event->x <= child->x + draw->width && event->y >= child->y
          && event->y <= child->y + draw->height) {
        popmenu = false;
        break;
      }
    }
    if (popmenu)
      gtk_menu_popup(GTK_MENU(priv->menu), NULL, NULL, NULL, NULL, event->button, event->time);
  }
}

void MainWindowMouseMotion(int, int)
{
}

void MainWindowMenu(menu_data * data)
{
  MainWindow *priv = (MainWindow *) data->priv;
  int menu = data->data;

  if (menu == quit)
    map3d_quit(masterWindow->window);
  else if (menu == back_color) {
    /*GdkColor *color;
    color = PickColor();
    if (color != NULL) {
      priv->bgcolor[0] = color->red / 65535.;
      priv->bgcolor[1] = color->green / 65535.;
      priv->bgcolor[2] = color->blue / 65535.;
      gtk_widget_modify_bg(masterWindow->window, GTK_STATE_NORMAL, color);
      gtk_widget_hide(masterWindow->window);
      gtk_widget_show(masterWindow->window);
      gtk_widget_queue_draw(masterWindow->drawarea);
      delete color;
    }*/
    PickColor(priv->bgcolor);
  }
  else if (menu == fore_color) {
    PickColor(priv->fgcolor);
  }
  else if (menu == info)
    priv->showinfotext = !priv->showinfotext;
  //gtk_widget_queue_draw(priv->drawarea);

}

void MainWindowKeyboardPress(GtkWidget * widget, GdkEventKey * event, gpointer)
{
  GenericWindow *me = FindWidgetOwner(masterWindow->focus);


  //switch (event->keyval) {
  //case GDK_Escape:
    //map3d_quit(masterWindow->window);
  //default:
    if (me == NULL)
      return;
    switch (me->wintype) {
    case GEOMWINDOW:
      GeomWindowKeyboardPress(widget, event, me);
      break;
    case LEGENDWINDOW:
      LegendWindowKeyboardPress(widget, event, me);
      break;
    case TIMEWINDOW:
      PickWindowKeyboardPress(widget, event, me);
      break;
    }
    //break;
  //}
}

void MainWindowKeyboardRelease(GtkWidget * widget, GdkEventKey * event, gpointer)
{
  GenericWindow *me = FindWidgetOwner(masterWindow->focus);

  if (me == NULL)
    return;

  switch (me->wintype) {
  case GEOMWINDOW:
    GeomWindowKeyboardRelease(widget, event, me);
    break;
  case LEGENDWINDOW:
    LegendWindowKeyboardRelease(widget, event, me);
    break;
  case TIMEWINDOW:
    PickWindowKeyboardRelease(widget, event, me);
    break;
  default:
    break;
  }
}

/*
void MainWindowKeyboard(unsigned char key, int, int)
{
	if (key == 27)
	{
		exit(0);
	}
}
*/

void MainWindowReshape(GtkWidget*, GdkEvent* event, gpointer /*data*/)
{
  //MainWindow *priv = (MainWindow *) data;
  //GtkRequisition req;

  masterWindow->width = masterWindow->fixed->allocation.width;
  masterWindow->height = masterWindow->fixed->allocation.height;

  GdkEventConfigure* conf = (GdkEventConfigure*) event;
  masterWindow->x = conf->x;
  masterWindow->y = conf->y;

/*	if (!priv->initialized) {
		priv->glcontext = gtk_widget_get_gl_context (priv->drawarea);
		priv->gldrawable = gtk_widget_get_gl_drawable (priv->drawarea);
	}
	gtk_widget_size_request(priv->drawarea, &req);

	gdk_gl_drawable_gl_begin (priv->gldrawable, priv->glcontext);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, priv->width, priv->height);
	gluOrtho2D(0, priv->width, 0, priv->height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gdk_gl_drawable_gl_end (priv->gldrawable);*/
}

void MainWindow::buildMenus()
{
  menu = gtk_menu_new();
  AddMenuEntry(menu, "Set Background Color", back_color, this, MainWindowMenu);
//  AddMenuEntry(menu, "Set Foreground Color", fore_color, this, MainWindowMenu);
//  AddMenuEntry(menu, "Show Info", info, this, MainWindowMenu);
  AddMenuEntry(menu, "Quit Map3d", quit, this, MainWindowMenu);
}

