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
#include <QMenu>
#include <QContextMenuEvent>

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

  label = new QLabel("", frame);
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
  resize(1,1);
  update();
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu menu;
  QAction* bgAction = menu.addAction("Set Background Color");
  QAction* fgAction = menu.addAction("Set Foreground Color");
  QAction* infoAction = menu.addAction("Show Info");
  infoAction->setCheckable(true);
  infoAction->setChecked(label->isVisible());
  QAction* quitAction = menu.addAction("Quit Map3d");

  QAction* selectedAction = menu.exec(event->globalPos());

  if (bgAction == selectedAction)
  {
    QColor color = palette().color(backgroundRole());

    float c[3]; c[0] = color.redF(); c[1] = color.greenF(); c[2] = color.blueF();
    PickColor(c, true);
    color.setRedF(c[0]); color.setGreenF(c[1]); color.setBlueF(c[2]);

    QPalette palette;
    palette.setColor(backgroundRole(), color);
    palette.setColor(foregroundRole(), this->palette().color(foregroundRole()));
    setPalette(palette);
  }
  else if (fgAction == selectedAction)
  {
    QColor color = palette().color(foregroundRole());

    float c[3]; c[0] = color.redF(); c[1] = color.greenF(); c[2] = color.blueF();
    PickColor(c, true);
    color.setRedF(c[0]); color.setGreenF(c[1]); color.setBlueF(c[2]);

    QPalette palette;
    palette.setColor(foregroundRole(), color);
    palette.setColor(backgroundRole(), this->palette().color(backgroundRole()));
    setPalette(palette);
  }
  else if (infoAction == selectedAction)
  {
    label->setVisible(!label->isVisible());
    adjustSize();
  }
  else if (quitAction == selectedAction)
  {
    map3d_quit(this);
  }
}

void MainWindow::updateBGColor(QColor color)
{

}

void MainWindow::updateFGColor(QColor color)
{

}
