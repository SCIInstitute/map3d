#include "GenericWindow.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "MainWindow.h"
#include "WindowManager.h"
#include "eventdata.h"
#include "map3d-struct.h"
#include "colormaps.h"
#include "texture.h"
#include "lock.h"
#include "dot.h"
#include "glprintf.h"

#include <QMouseEvent>
#include <QDebug>
#include <QList>

#include <stdio.h>

extern MainWindow *masterWindow;
extern Map3d_Info map3d_info;

static int font_size[] = {4, 6, 8, 10, 12, 14, 18, 24, 36, 48 };

Map3dGLWidget* Map3dGLWidget::sharedWidget = NULL;

// in Qt, the x,y coords of a toplevel widget are the window coordinates.  The width and the height
//   are still the size of the interior widget, and we can get the entire size of the window (including
//   decoration) with frameGeometry.  However, we need to create one window before we can 
//   figure this out, so we'll create our first window blindly, and then adjust it afterward.
static QList<Map3dGLWidget*> windowsToAdjust;

Map3dGLWidget::Map3dGLWidget(QWidget* parent) : QGLWidget(parent, NULL), wintype(RMSWINDOW)
{
  winid = -1;
}

Map3dGLWidget::Map3dGLWidget(QWidget* parent, int type, const char* title, int min_width, int min_height) 
  : QGLWidget(parent, sharedWidget), startHidden(false)
{
  wintype = type;
  winid = -1;
  poplevel = 9999;
  
  if (sharedWidget == NULL)
    sharedWidget = this;

  winid = AssociateWindow(this);
  if (wintype == GEOMWINDOW)
    AssociateGeomWindow((GeomWindow*)this);

  setPopLevel();
  // FIX setMinimumSize(min_width, min_height);
}

void Map3dGLWidget::resizeGL()
{
  glViewport(0, 0, width(), height());
}


// this one is to put one "borderless window" on top of the rest
// set this window's poplevel to 1, and increment the others
// this should generate a unique, incremental poplevel for each window
// this is for sorting windows for generating the image
void Map3dGLWidget::setPopLevel()
{
  int old_poplevel = poplevel;
  for (unsigned i = 0; i < numWindows(); i++) {
    Map3dGLWidget* win = (Map3dGLWidget*) GetWindow(i);
    if (!win || win == this || win->poplevel > old_poplevel)
      continue;
    win->poplevel++;
  }
  poplevel = 1;
}

// This is to position and size the WINDOW, not the widget.  Even if exactCoords is true,
//   adjust to make the size of the window.
void Map3dGLWidget::positionWindow(int _width, int _height, int _x, int _y, int def_width, int def_height, bool exactCoords)
{
  int width = _width, height = _height, x = _x, y = _y;
  const int cascadeDiff = 35;
  static int numTimesWrapped = 0;
  if (_width > 0 && _height > 0 && !exactCoords) {
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

    //x = _x + map3d_info.borderWidth;
    //y = map3d_info.screenHeight - (_y + _height) - map3d_info.borderWidth;  // height had VERTICAL taken out when it was passed in
    width = _width + 1;
    height = _height + 1;

  }
  else if (!exactCoords) {

    //default position for next window - wrap around the screen if it won't fit
    if (map3d_info.posx + def_width > map3d_info.screenWidth) {
      map3d_info.posx = numTimesWrapped * cascadeDiff;
      map3d_info.posy += def_height;
    }
    if (map3d_info.posy + def_height > map3d_info.screenHeight) {
      // here use absolute border width so the windows cascade even with borderless windows
      numTimesWrapped++;
      map3d_info.posy = numTimesWrapped * cascadeDiff;
      map3d_info.posx = numTimesWrapped * cascadeDiff;
    }
    x = map3d_info.posx;
    y = map3d_info.posy;
    width = def_width;
    height = def_height;
    map3d_info.posx += def_width;
  }

  // see note at the top of the file
  width -= HORIZ;
  height -= VERTICAL;

  resize(width, height);
  move(x, y);

  if (masterWindow)
    masterWindow->adjustSize();
}

void Map3dGLWidget::getCommandLineCoordinates(int& _width, int& _height, int& _x, int& _y)
{
  _x = x();
  _y = y();
  _width = frameGeometry().width();
  _height = frameGeometry().height();
}

void Map3dGLWidget::showEvent(QShowEvent * event)
{
  if (!map3d_info.borderInitialized)
  {
    map3d_info.borderInitialized = true;
    if (masterWindow == NULL)
    {
      QRect frameGeom = frameGeometry();
      map3d_info.borderWidth = (frameGeom.width() - width()) / 2;
      map3d_info.titleHeight = frameGeom.height() - height() - map3d_info.borderWidth*2;

      resize(width() - HORIZ, height() - VERTICAL);
    }
  }
}


void Map3dGLWidget::initializeGL() {
  // Lock Texture
  map3d_info.lock_texture = CreateTexture(LOCK_TEXTURE, GL_TEXTURE_2D, 4, 64, 64, GL_RGBA, 
    GL_LINEAR, GL_LINEAR, GL_MODULATE, padlock);

  // Dot Texture   
  map3d_info.dot_texture = CreateTexture(DOT_TEXTURE, GL_TEXTURE_2D, 1, 128, 128, GL_ALPHA,
    GL_LINEAR, GL_LINEAR, GL_REPLACE, dot);

  // Rainbow colormap Texture
  map3d_info.rainbow_texture = CreateTexture(RAINBOW_CMAP, GL_TEXTURE_1D, 3, Rainbow.max+1, 1, GL_RGB,
    GL_NEAREST, GL_NEAREST, GL_MODULATE, Rainbow.map);
  
  // Jet colormap Texture
  map3d_info.jet_texture = CreateTexture(JET_CMAP, GL_TEXTURE_1D, 3, Jet.max+1, 1, GL_RGB,
    GL_NEAREST, GL_NEAREST, GL_MODULATE, Jet.map);
}

// size should be a float/int from 1-10. 
void Map3dGLWidget::renderString3f(float x, float y, float z, float size, QString string)
{
  int sizeindex = (int) size;

  QFont f = font();
  f.setPointSize(font_size[sizeindex]);
  //setFont(f);

  renderText(x, y, z, string, f);
}


int Map3dGLWidget::getFontWidth(int size_index, QString string) 
{
  QFont f = font();
  f.setPointSize(font_size[size_index]);

  return QFontMetrics(f).width(string);
}

int Map3dGLWidget::getFontHeight(int size_index) 
{
  QFont f = font();
  f.setPointSize(font_size[size_index]);

  return QFontMetrics(f).height();
}

// this should be called on clicking a window.  
void Map3dGLWidget::setMoveCoordinates(QMouseEvent* event)
{
  raise();
  setPopLevel();

  if (masterWindow)
  {
    //grabKeyboard();
    setFocus(Qt::OtherFocusReason);

  }

  // set start
  mouseStart = event->globalPos();
  startPos = pos();
  startSize = size();
}

void Map3dGLWidget::moveEvent(QMouseEvent* event)
{
  QPoint newPos = startPos + event->globalPos() - mouseStart;

  positionWindow(width(), height(), newPos.x(), newPos.y(), -1, -1, true);
}

void Map3dGLWidget::sizeEvent(QMouseEvent* event)
{
  QPoint diff = event->globalPos() - mouseStart;
  QSize newSize(startSize.width() + diff.x(), startSize.height() + diff.y());

  positionWindow(newSize.width(), newSize.height(), x(), y(), -1, -1, true);
}

bool Map3dGLWidget::matchesModifiers(int windowModifiers, int desiredModifiers, bool exactMatch)
{
  // we don't want to consider these
  windowModifiers = windowModifiers & ~Qt::KeypadModifier;
  windowModifiers = windowModifiers & ~Qt::GroupSwitchModifier;
  
#ifdef Q_OS_MAC
  // translate what Qt gives us for Mac into what we expect on windows
  int tmpMod = windowModifiers;
  if (windowModifiers & Qt::ShiftModifier) tmpMod -= Qt::ShiftModifier;
  if (windowModifiers & Qt::ControlModifier) tmpMod -= Qt::ControlModifier;
  if (windowModifiers & Qt::AltModifier) tmpMod -= Qt::AltModifier;
  if (windowModifiers & Qt::MetaModifier) tmpMod -= Qt::MetaModifier;
  if (windowModifiers & Qt::KeypadModifier) tmpMod -= Qt::KeypadModifier;
#endif
  if (exactMatch)
    return windowModifiers == desiredModifiers;
  else 
    return (windowModifiers & desiredModifiers) == desiredModifiers;
}

int Map3dGLWidget::mouseButtonOverride(QMouseEvent* event)
{
  int button = event->type() == QEvent::MouseMove ? event->buttons() : event->button();

  // do this for psycho one-button mac mice
#ifdef Q_OS_MAC
  if (button == Qt::LeftButton)
  {
    // don't use matchesModifiers here, as that translates these away
    //if (event->modifiers() == Qt::ControlModifier)
      //return Qt::RightButton;
    if (event->modifiers() == Qt::AltModifier)
      return Qt::MidButton;
  }
#endif
  return button;
}

