/* LegendWindow.cxx */

#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#include <math.h>
#include "glprintf.h"
#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#include "Map3d_Geom.h"
#include "Contour_Info.h"
#include "LegendWindow.h"
#include "MeshList.h"
#include "Surf_Data.h"
#include "WindowManager.h"
#include "colormaps.h"
#include "dialogs.h"
#include "eventdata.h"
#include "glprintf.h"
#include "reportstate.h"
#include "scalesubs.h"
#include "MainWindow.h"
#include "GeomWindow.h"

#include <QMenu>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>

// keep the cont_xx in order so we can use math
enum { vertical, horizontal, cont_match, cont_0, cont_1, cont_2, cont_3, cont_4, cont_5, cont_6 };

extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;

#define FONT_ADJUST 1.75
#define CHAR_WIDTH .07 * FONT_ADJUST
#define CHAR_HEIGHT .07 * FONT_ADJUST

static const int min_width = 100;
static const int min_height = 100;
static const int default_width = 170;
static const int default_height = 256;


LegendWindow::LegendWindow(QWidget* parent) : Map3dGLWidget(parent, LEGENDWINDOW, "Colormap Legend",120,135)
{
  // since we can't guarantee the order of initialization, or when the window will be created, we need
  //   to initialize mesh to NULL, and set it later
  mesh = NULL;
  bgcolor[0] = bgcolor[1] = bgcolor[2] = 0;
  fgcolor[0] = fgcolor[1] = fgcolor[2] = 1;
  //showinfotext = 1;
  matchContours = 1;
}

//static
LegendWindow* LegendWindow::LegendWindowCreate(Mesh_Info* mesh, int _width, int _height, int _x, int _y, bool hidden)
{
  LegendWindow* win = new LegendWindow(masterWindow ? masterWindow->childrenFrame : NULL);
  win->positionWindow(_width, _height, _x, _y, default_width, default_height);
  win->setVisible(!hidden);
  //win->mesh = mesh;
  return win;
}

void LegendWindow::initializeGL()
{
  // since we can't guarantee the order of initialization, or when the window will be created, we need
  //   to initialize mesh to NULL, and set it later
  mesh = NULL;

  Map3dGLWidget::initializeGL();
  // FIX if (matchContours)
  //  ticks.setActive(7);
  //else
  //  ticks.setActive(nticks);


  glLineWidth(3);
  glDisable(GL_DEPTH_TEST);
}

void LegendWindow::paintGL()
{
  if (mesh == NULL || mesh->cont == NULL)
    return;
  
  int numconts = mesh->cont->numlevels;

  if (matchContours)
    nticks = numconts;

  // for the top and bottom
  int actualTicks = nticks + 2;

  int length = (*(map))->max;
  float nextval, lastval, colorval;

  int loop;

  unsigned char color[3];

  //number of pixels to take off for intermediate lines on graph
  int size_adjuster;
  float delta;
  unsigned char *map = (*(this->map))->map;
  float position[3] = { -1, height() - 20.f, 0 };
  float factor;
  float potmin, potmax;
  Surf_Data *cursurf = surf;

  cursurf->get_minmax(potmin, potmax);

  if (map3d_info.scale_mapping == SYMMETRIC) {
    if (fabs(potmax) > fabs(potmin))
      potmin = -potmax;
    else
      potmax = -potmin;
  }
  
  if (map3d_info.scale_mapping == SEPARATE) {
    if (potmax < 0)
      potmax = 0;
    if (potmin > 0)
      potmin = 0;
  }
  


  // contvals will be all the lines drawn, whether in matchContours mode or not, including the min and max line
  vector<float> contvals;
  contvals.push_back(potmin);
  if (matchContours) {
    nticks = mesh->cont->numlevels;
    for (int i = 0; i < mesh->cont->numlevels; i++)
      contvals.push_back(mesh->cont->isolevels[i]);
  }
  else {
    float tick_range = (potmax-potmin)/(nticks+1);
    for (int i = 0; i < nticks; i++)
      contvals.push_back(potmin + (tick_range*(i+1)));
  }
  contvals.push_back(potmax);

  float coloroffset = .5;

  if (bgcolor[0] + .3 > 1 || bgcolor[1] + .3 > 1 || bgcolor[2] + .3 > 1)
    coloroffset = -coloroffset;

  glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int pixelFactor = QApplication::desktop()->devicePixelRatio();
  // this compensates for the "Retina" display ratio.  See http://doc.qt.io/qt-5/highdpi.html
  //  (for some reason the picking doesn't need this)
  glViewport(0, 0, width()*pixelFactor, height()*pixelFactor);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width(), 0, height());
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);

  // link info to geom window
  if (mesh->gpriv->showinfotext) {
    /*position[0] = -6*CHAR_WIDTH*aspect*1.5;
      glprintf(position,normal,up,CHAR_WIDTH*aspect*1.5,CHAR_HEIGHT*1.5,"Surface # %d",cursurf->surfnum);
      position[0] = -((strlen(cursurf->potfilename)+3)/2.0f)*CHAR_WIDTH*aspect;
      position[1] -= CHAR_HEIGHT*1.5;
      glprintf(position,normal,up,CHAR_WIDTH*aspect,CHAR_HEIGHT,"%s@%d",cursurf->potfilename,cursurf->seriesnum);
    */

    // output surface and data file information
    QString toRender;
    float fontWidth;
    if (mesh->geom->subsurf <= 0)
      toRender = "Surface # " + QString::number(mesh->geom->surfnum);
    else
      toRender = "Surface # " + QString::number(mesh->geom->surfnum) + "-" + QString::number(mesh->geom->subsurf);

    fontWidth = (float)getFontWidth(mesh->gpriv->large_font, toRender);
    if(!orientation)//Horizontal
      position[0] = (width() /3) - fontWidth/2;
    else//vertical
      position[0] = (width() / 2) - fontWidth/2;
    
    position[1] = (float) (height() - getFontHeight(mesh->gpriv->large_font));
    renderString3f(position[0], position[1], position[2], mesh->gpriv->large_font, toRender);

    char * slash = cursurf->potfilename;// FIX shorten_filename(cursurf->potfilename);
    toRender = QString(slash) + "@" + QString::number(cursurf->seriesnum+1);
    fontWidth = (float)getFontWidth(mesh->gpriv->med_font, toRender);
    if(!orientation)//Horizontal
      position[0] = (width() / 3) - fontWidth / 2;
    else//vertical
      position[0] = (width() / 2) - fontWidth / 2;

    position[1] -= getFontHeight(mesh->gpriv->med_font)*.8f ;

    renderString3f(position[0], position[1], position[2], mesh->gpriv->med_font, toRender);

    // output spatial scaling information (which surfaces)
    if(!orientation)//Horizontal
      position[1] = (float) (height() - getFontHeight(mesh->gpriv->med_font));
    else//vertical
      position[1] -= getFontHeight(mesh->gpriv->med_font) ;

    // set default to local if invalid things are set
    int scope = map3d_info.scale_scope;
    if (scope == SLAVE_FRAME && !cursurf->mastersurf)
      scope = LOCAL_SCALE;
    else if (scope == SLAVE_GLOBAL && !cursurf->mastersurf)
      scope = GLOBAL_SURFACE;

    if (cursurf->user_scaling && cursurf->userpotmin < cursurf->userpotmax) {
      toRender = "User-specified range";
    }
    else if (scope == SLAVE_FRAME || scope == SLAVE_GLOBAL) {
      toRender = "Slave to Surface # " + QString::number(cursurf->mastersurf->surfnum);
    }
    else if (scope == GROUP_FRAME || scope == GROUP_GLOBAL) {
      toRender = "Group #" + QString::number(cursurf->mesh->groupid + 1);
    }
    else if (scope == GLOBAL_GLOBAL || scope == GLOBAL_FRAME) {
      toRender = "All surfaces";
    }
    else if (scope == LOCAL_SCALE || scope == GLOBAL_SURFACE) {
      toRender = "Local surface";
    }

    fontWidth = (float)getFontWidth(mesh->gpriv->med_font, toRender);
    if(!orientation)//Horizontal
      position[0] = (width() *.8f) - fontWidth/2;
    else//vertical
      position[0] = (width() / 2.0f) - fontWidth/2;
    renderString3f(position[0], position[1], position[2], mesh->gpriv->med_font, toRender);

    // output temporal scaling information (which frames)
    position[1] -= getFontHeight(mesh->gpriv->med_font)*.8f ;

    toRender = "";
    if (scope == GLOBAL_SURFACE || scope == GLOBAL_GLOBAL || scope == GROUP_GLOBAL || scope == SLAVE_GLOBAL) {
      toRender = "All frames";
    }
    else if (scope == GLOBAL_FRAME || scope == LOCAL_SCALE || scope == GROUP_FRAME ||
             scope == SLAVE_FRAME) {
      toRender = "Frame # " + QString::number(cursurf->framenum + 1);
    }
    if (toRender != "")
    {
      fontWidth = (float)getFontWidth(mesh->gpriv->med_font, toRender);
      if(!orientation)//Horizontal
        position[0] = (width() *.8f) - fontWidth/2;
      else//vertical
        position[0] = (width() / 2.0f) - fontWidth/2;
      renderString3f(position[0], position[1], position[2], mesh->gpriv->med_font, toRender);
    }
  }

  if (map3d_info.scale_mapping == SYMMETRIC) {
    if (fabs(potmax) > fabs(potmin))
      potmin = -potmax;
    else
      potmax = -potmin;
  }

  if (map3d_info.scale_mapping == SEPARATE) {
    if (potmax < 0)
      potmax = 0;
    if (potmin > 0)
      potmin = 0;
  }
  //set up variables for vertical and horizontal
  int bottom, left, size, vsize, hsize;

  vsize = hsize = 0;
  //horiz
  if (!orientation) {
    bottom = 40;
    //vsize = ((height() > 150) ? (int)(height() * .75 - 30) : (int)(height() - 80));
    left = 20;
    size = width() - 40;
    if (mesh->gpriv->showinfotext)
      vsize = height() - (getFontHeight(mesh->gpriv->large_font) +
			     getFontHeight(mesh->gpriv->med_font)+10);
    else
      vsize = height() - 20;
    //     size = height - (getFontHeight(mesh->gpriv->large_font) +
    //  			   3*getFontHeight(mesh->gpriv->med_font)+20);
  }
  //vertical
  else {
    if (mesh->gpriv->showinfotext)
      size = height() - (getFontHeight(mesh->gpriv->large_font) +
			     3*getFontHeight(mesh->gpriv->med_font)+20);
    else
      size = height() - 40;
    hsize = width() - getFontWidth(mesh->gpriv->small_font, "-XXX.XX")-20;
    if (hsize < 40) hsize = 40; // make at least 20 pizels for the color bar
    bottom = 20;
    left = 20;
  }

  //set up window vars
  if (size / contvals.size() > 4)
    size_adjuster = 0;
  else if (size / contvals.size() > 3)
    size_adjuster = 1;
  else
    size_adjuster = 2;
  delta = ((float)size) / length;
  factor = (float)(size / (actualTicks - 1));

  //draw surface color if mesh is displayed as solid color
  if (mesh->drawmesh == RENDER_MESH_ELTS || mesh->drawmesh == RENDER_MESH_ELTS_CONN) {
    if (mesh->gpriv->secondarysurf == mesh->geom->surfnum - 1)
      glColor3f(mesh->secondarycolor[0], mesh->secondarycolor[1], mesh->secondarycolor[2]);
    else
      glColor3f(mesh->meshcolor[0], mesh->meshcolor[1], mesh->meshcolor[2]);
    //vertical
    if (orientation) {
      glBegin(GL_QUADS);
      glVertex2d(left, bottom);
      glVertex2d(left, size + bottom);
      glVertex2d(hsize, size + bottom);
      glVertex2d(hsize, bottom);
      glEnd();
    }
    //horizontal
    else {
      glBegin(GL_QUADS);
      glVertex2d(left, vsize);
      glVertex2d(left, bottom);
      glVertex2d(left + size, bottom);
      glVertex2d(left + size, vsize);
      glEnd();
    }
  }
  //gouraud shading (or band-shading without matching contours - it wouldn't make sense to 
  // draw bands that don't correspond to the display)
  else if (mesh->shadingmodel == SHADE_GOURAUD || 
           mesh->shadingmodel == SHADE_FLAT ||
           (mesh->shadingmodel == SHADE_BANDED && !matchContours)) {

    glBegin(GL_QUAD_STRIP);

    for (loop = 0; loop < length; loop++) {
      getContColor(potmin + loop * (potmax - potmin) / length, potmin, potmax, *(this->map), color, mesh->invert);
      //glColor3ub(map[loop*3],map[loop*3+1],map[loop*3+2]);
      glColor3ubv(color);
      if (!orientation) {
        glVertex2d(left + loop * delta, vsize);
        glVertex2d(left + loop * delta, bottom);
      }
      else {
        glVertex2d(left, bottom + loop * delta);
        glVertex2d(hsize, bottom + loop * delta);
      }
    }
    glEnd();
  }
  //band shading
  else if (mesh->shadingmodel == SHADE_BANDED) {
    // assumes matched contours
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    for (loop = 0; loop < (int) contvals.size()-1; loop++) {
      nextval = contvals[loop + 1];
      lastval = contvals[loop];
      if (loop == 0) {
        colorval = potmin;
      }
      else if (loop == contvals.size() - 2) {
        colorval = potmax;
      }
      else {
        colorval = contvals[loop] + (contvals[loop+1]-contvals[loop])*(loop)/(numconts);
      }
      // don't get color by the potval, but by the ratio between the low/high vals
      getContColor(colorval, potmin, potmax, *(this->map), color, mesh->invert);
      glColor3ubv(color);


      //horiz
      if (!orientation) {
        glVertex2d(left + (lastval - potmin) / (potmax - potmin) * size, vsize);
        glVertex2d(left + (lastval - potmin) / (potmax - potmin) * size, bottom);
        glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, bottom);
        glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, vsize);
      }
      else {
        glVertex2d(left, bottom + (lastval - potmin) / (potmax - potmin) * size);
        glVertex2d(hsize, bottom + (lastval - potmin) / (potmax - potmin) * size);  // -.85 to .4
        glVertex2d(hsize, bottom + (nextval - potmin) / (potmax - potmin) * size);  // -.85 to .4
        glVertex2d(left, bottom + (nextval - potmin) / (potmax - potmin) * size);
      }
    }
    glEnd();
  }

  /* draw legend outline */
  glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
  glBegin(GL_LINES);

  //horiz
  if (!orientation) {
    glVertex2d(left, vsize);
    glVertex2d(left + size, vsize);
    glVertex2d(left, bottom);
    glVertex2d(left + size, bottom);
  }
  //vert
  else {
    glVertex2d(left, bottom);
    glVertex2d(left, bottom + size);
    glVertex2d(hsize, bottom + size);
    glVertex2d(hsize, bottom);
  }

  glEnd();

  glLineWidth((float)(3 - size_adjuster));
  glBegin(GL_LINES);


  //draw contour lines in legend window
  for (loop = 0; loop < (int) contvals.size(); loop++) {
    nextval = contvals[loop];
    if (mesh->shadingmodel == SHADE_NONE) {
      if (loop == 0 || loop == contvals.size()-1){
        glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
   	  }
      else {
        getContColor(nextval, potmin, potmax, *(this->map), color, mesh->invert);
        glColor3ubv(color);
      }
    }
    //horiz
    if (!orientation) {
      glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, vsize);  // -.85 to .4
      glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, bottom - 5);
    }
    //vert
    else {
      glVertex2d(left, bottom + (nextval - potmin) / (potmax - potmin) * size); // -.85 to .4
      int extension = (loop == 0 || loop == contvals.size()-1) ? 5 : 0;
      glVertex2d(hsize + extension, bottom + (nextval - potmin) / (potmax - potmin) * size);
    }

  }
  glEnd();
  //glLineWidth(3);

  if (orientation) {
    position[0] = (float)hsize + 10;
    position[1] = 0;
    position[2] = 0;
  }
  else {
    position[0] = (float)left;
    position[1] = (float)bottom - 15;
    position[2] = 0;
  }

  //write contour values
  // vars used in inner loop
  char string[256] = { '\0' };
  int prevcont = bottom;
  int lastcont = bottom + size;
  int stagger = 0;            //which row you're on
  int rowpos[2] = { -15, -100 };  //
  int endpos = (int)(left - getFontWidth(mesh->gpriv->small_font, "XXX") + size);
  int font_height = getFontHeight(mesh->gpriv->small_font);

  for (loop = 0; loop < (unsigned) contvals.size(); loop++) {
    glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);
    nextval = contvals[loop];

    if (nextval > -.1 && nextval < .1) // 3-decimal precision
      sprintf(string, "%.3f", nextval);
    else if (nextval >= .1 && nextval < 10) //2 decimal precision
      sprintf(string, "%.2f", nextval);
    else                      //1 decimal precision
      sprintf(string, "%.1f", nextval);
    //horizontal
    if (!orientation) {
      position[0] = left - getFontWidth(mesh->gpriv->small_font, "XXX") + ((nextval - potmin) / (potmax - potmin) * size);
      //glprintf(position,normal,up,mod_width*aspect,mod_height,"%.2f\0",nextval);
      if (loop == 0 || loop == contvals.size()-1 || (rowpos[stagger] + getFontWidth(mesh->gpriv->small_font, "XXXXXX") < position[0] &&
          (position[0] + getFontWidth(mesh->gpriv->small_font, "XXXXXX") < endpos ||
          rowpos[(stagger + 1) % 2] + getFontWidth(mesh->gpriv->small_font, "XXXXXX") < endpos))) {
        glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);
        renderString3f(position[0], position[1] - 12 * stagger, position[2], mesh->gpriv->small_font, string);
        //Extend the contour line if number is staggered(on the bottom)
        if(stagger == 1){
          glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
          if (mesh->shadingmodel == SHADE_NONE && loop != 0 && loop != contvals.size()-1) {
            getContColor(nextval, potmin, potmax, *(this->map), color, mesh->invert);
            glColor3ubv(color);
          }
          glBegin(GL_LINES);
          glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, bottom - 5); 
          glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, position[1] - 2 * stagger);
          glEnd();
          glLineWidth(3);
        }
        //end draw contour line
        rowpos[stagger] = (int)position[0];
        stagger = (stagger + 1) % 2;
      }
    }
    //vertical
    else {
      //int numconts = actualTicks;
      position[1] = bottom - font_height/4 + ((nextval - potmin) / (potmax - potmin) * size);
      //determines which contour val to write
      if (loop == 0 || loop == contvals.size()-1 || 
          position[1] >= prevcont + font_height && position[1] <= lastcont - font_height) {
        renderString3f(position[0], position[1], position[2], mesh->gpriv->small_font, string);
        prevcont = (int)position[1];
        if (loop != 0 && loop != contvals.size()-1) {
          // extend the contour line, so we can see which value it points to
          glBegin(GL_LINES);
          if (mesh->shadingmodel != SHADE_NONE) {
            glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
          }
          else {
            getContColor(nextval, potmin, potmax, *(this->map), color, mesh->invert);
            glColor3ubv(color);
          }

          glVertex2d(left, bottom + (nextval - potmin) / (potmax - potmin) * size); // -.85 to .4
          glVertex2d(hsize + 5, bottom + (nextval - potmin) / (potmax - potmin) * size);
          glEnd();
        }
      }
    }
  }

  glColor3f(1, 1, 1);

#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("LegendWindow OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void LegendWindow::keyPressEvent ( QKeyEvent * event )
{
  switch (event->text().toLatin1()[0]) {
  case 'q':
    hide();
    mesh->showlegend = false;
    break;
  }
}

void LegendWindow::mousePressEvent ( QMouseEvent * event )
{
  if (event->button() == Qt::RightButton){       // right click
    int menu_data = OpenMenu(mapToGlobal(event->pos()));
    if (menu_data >= 0)
      MenuEvent(menu_data);
  }

  setMoveCoordinates(event);
}

void LegendWindow::mouseMoveEvent ( QMouseEvent * event )
{
  if (event->buttons() == Qt::LeftButton && event->modifiers() & Qt::AltModifier) {
    moveEvent(event);
  }
  // MIDDLE MOUSE DOWN + ALT = draw reshaping window
  else if (event->buttons() == Qt::MidButton && event->modifiers() & Qt::AltModifier) {
    sizeEvent(event);
  }
}

int LegendWindow::OpenMenu(QPoint point)
{
  QMenu menu(this);
  QMenu* submenu = menu.addMenu("Orientation");
  QAction* action = NULL;
  action = submenu->addAction("Vertical");
  action->setData(vertical); action->setCheckable(true); action->setChecked(orientation);
  action = submenu->addAction("Horizontal");
  action->setData(horizontal); action->setCheckable(true); action->setChecked(!orientation);

  submenu = menu.addMenu("Number of Tick Marks");
  action = submenu->addAction("Match Contours");
  action->setData(cont_match); action->setCheckable(true); action->setChecked(matchContours);
  action = submenu->addAction("0");
  action->setData(cont_0); action->setCheckable(true); action->setChecked(!matchContours && nticks == 0);
  action = submenu->addAction("1");
  action->setData(cont_1); action->setCheckable(true); action->setChecked(!matchContours && nticks == 1);
  action = submenu->addAction("2");
  action->setData(cont_2); action->setCheckable(true); action->setChecked(!matchContours && nticks == 2);
  action = submenu->addAction("3");
  action->setData(cont_3); action->setCheckable(true); action->setChecked(!matchContours && nticks == 3);
  action = submenu->addAction("4");
  action->setData(cont_4); action->setCheckable(true); action->setChecked(!matchContours && nticks == 4);
  action = submenu->addAction("5");
  action->setData(cont_5); action->setCheckable(true); action->setChecked(!matchContours && nticks == 5);
  action = submenu->addAction("6");
  action->setData(cont_6); action->setCheckable(true); action->setChecked(!matchContours && nticks == 6);

  action = menu.exec(point);
  if (action)
    return action->data().toInt();
  else
    return -1;
}

void LegendWindow::MenuEvent(int menu_data)
{
  if (menu_data == vertical)
    orientation = true;
  else if (menu_data == horizontal)
    orientation = false;
  else if (menu_data == cont_match)
    matchContours = true;
  else if (menu_data >= cont_0 && menu_data <= cont_6)
  {
    matchContours = false;
    nticks = menu_data - cont_0;
  }
  update();
}
