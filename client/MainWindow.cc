/* MainWindow.cxx */

#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#include <string.h>
#ifdef __APPLE__
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

#include <QColor>
#include <QLabel>
#include <QVBoxLayout>

MainWindow *masterWindow = NULL;

MainWindow::MainWindow()
{
  textLines = 0;
  startHidden = false;

  QFrame* frame = new QFrame(this);
  setCentralWidget(frame);
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(0);
  layout->setSpacing(0);
  frame->setLayout(layout);

  label = new QLabel("blah", frame);
  QFont font = label->font();
  font.setPointSize(18);
  label->setFont(font);
  label->setAlignment(Qt::AlignHCenter);
  layout->addWidget(label);
  childrenFrame = new QWidget(frame);
  layout->addWidget(childrenFrame);
  layout->addStretch();


}

void MainWindow::updateLabel()
{
  QString text;

  int i = 0;
  for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
    if (i > 0)
      text += "\n";
    if (i > 3)
    {
      text += "+ others";
      break;
    }

    Mesh_Info* mesh = mi.getMesh();
    text += shorten_filename(mesh->mysurf->geomfilename);
    i++;
  }

  label->setText(text);
}

void MainWindowRepaint(GtkWidget *, GdkEvent *, void* /*data*/)
{
/*  FIX MainWindow *priv = (MainWindow *) data;

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

void MainWindowMenu(menu_data * data)
{
#if 0
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
#endif
}

//void MainWindow::buildMenus()
//{
//  AddMenuEntry(menu, "Set Background Color", back_color, this, MainWindowMenu);
//  AddMenuEntry(menu, "Set Foreground Color", fore_color, this, MainWindowMenu);
//  AddMenuEntry(menu, "Show Info", info, this, MainWindowMenu);
//  AddMenuEntry(menu, "Quit Map3d", quit, this, MainWindowMenu);
//}

void MainWindow::adjustSize()
{
  int width = 0, height = 0;
  for (unsigned i = 0; i < numWindows(); i++)
  {
    Map3dGLWidget* window = GetWindow(i);
    if (window)
    {
      if (window->x() + window->width() > width) 
        width = window->x() + window->width();
      if (window->y() + window->height() > height) 
        height = window->y() + window->height();
    }
  }
  childrenFrame->setMinimumSize(width, height);
  childrenFrame->setMaximumSize(width, height);
  this->setMaximumSize(qMax(width, label->minimumSize().width()), label->minimumSize().height() + height);
}
