/* LegendWindow.cxx */

#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <math.h>
#include "glprintf.h"
#ifdef OSX
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

extern Map3d_Info map3d_info;
extern GdkGLConfig *glconfig;
extern MainWindow *masterWindow;

extern bool menulock;
//bool legendmenulock = false;

#define FONT_ADJUST 1.75
#define CHAR_WIDTH .07 * FONT_ADJUST
#define CHAR_HEIGHT .07 * FONT_ADJUST

LegendWindow::LegendWindow() : GLWindow(LEGENDWINDOW, "Colormap Legend",120,135)
{
  
  bgcolor[0] = bgcolor[1] = bgcolor[2] = 0;
  fgcolor[0] = fgcolor[1] = fgcolor[2] = 1;
  //showinfotext = 1;
  is_displayed = 1;
  matchContours = 1;

  menu = gtk_menu_new();
  GtkWidget *ms = AddSubMenu(menu, "Orientation");
  vert_orient = AddCheckMenuEntry(ms, "Vertical", 1, this, MapOrientation);
  horiz_orient = AddCheckMenuEntry(ms, "Horizontal", 0, this, MapOrientation);
  GtkWidget *cm = AddSubMenu(menu, "Number of Tick Marks");
  ticks.add(AddCheckMenuEntry(cm, "0", 0, this, MapTicks));
  ticks.add(AddCheckMenuEntry(cm, "1", 1, this, MapTicks));
  ticks.add(AddCheckMenuEntry(cm, "2", 2, this, MapTicks));
  ticks.add(AddCheckMenuEntry(cm, "3", 3, this, MapTicks));
  ticks.add(AddCheckMenuEntry(cm, "4", 4, this, MapTicks));
  ticks.add(AddCheckMenuEntry(cm, "5", 5, this, MapTicks));
  ticks.add(AddCheckMenuEntry(cm, "6", 6, this, MapTicks));
  ticks.add(AddCheckMenuEntry(cm, "Match Contours", 128, this, MapTicks));
  menulock = true;
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(vert_orient), true);
  menulock = false;

  setupEventHandlers();
}

void LegendWindow::setupEventHandlers()
{
  gtk_widget_set_events(drawarea, GDK_EXPOSURE_MASK |
                        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_VISIBILITY_NOTIFY_MASK |
                        GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_MOTION_MASK);

  //  g_signal_connect(G_OBJECT(drawarea), "realize", G_CALLBACK(GeomWindowInit), this);
  g_signal_connect(G_OBJECT(drawarea), "expose_event", G_CALLBACK(LegendWindowRepaint), this);
  g_signal_connect(G_OBJECT(drawarea), "button_press_event", G_CALLBACK(LegendWindowButtonPress), this);
  g_signal_connect(G_OBJECT(drawarea), "button_release_event", G_CALLBACK(LegendWindowButtonRelease), this);
  g_signal_connect(G_OBJECT(drawarea), "motion_notify_event", G_CALLBACK(LegendWindowMouseMotion), this);
  g_signal_connect(G_OBJECT(drawarea), "configure_event", G_CALLBACK(LegendWindowReshape), this);

  if (masterWindow == 0) {
    g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(LegendWindowKeyboardPress), this);
    g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(LegendWindowKeyboardRelease), this);
    g_signal_connect_swapped(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_hide),
                             window);
    g_signal_connect(G_OBJECT(window), "window_state_event", G_CALLBACK (window_state_callback), NULL);
  }

}

//static
LegendWindow* LegendWindow::LegendWindowCreate(int _width, int _height, int _x, int _y, int def_width, int def_height, bool hidden)
{
  if (map3d_info.numLegendwins >= MAX_SURFS) {
    printf("Warning: cannot create more than %d Colormap Windows\n", MAX_SURFS);
    return 0;
  }
  LegendWindow* win = map3d_info.legendwins[map3d_info.numLegendwins++];
  win->parentid = map3d_info.parentid;
  if(!hidden){
    win->positionWindow(_width, _height, _x, _y, def_width, def_height);
  }
  else{
    win->startHidden = true;
    if (_width > 0 && _height > 0) {
      win->specifiedCoordinates = true;
      win->width = _width;
      win->height = _height;
      win->x = _x;
      win->y = _y;
    }
    else {
      win->specifiedCoordinates = false;
      win->width = def_width;
      win->height = def_height;
    }
  }
  return win;
}

void LegendWindowInit(GtkWidget *, GdkEvent *, gpointer data)
{
  LegendWindow *priv = (LegendWindow *) data;
  if (!priv->standardInitialize())
    return;

  if (priv->matchContours)
    priv->ticks.setActive(7);
  else
    priv->ticks.setActive(priv->nticks);


  gdk_gl_drawable_gl_begin(priv->gldrawable, priv->glcontext);
  glLineWidth(3);
  glDisable(GL_DEPTH_TEST);
  gdk_gl_drawable_gl_end(priv->gldrawable);

  gtk_widget_queue_resize(priv->drawarea);
}

void LegendWindowDestroy(LegendWindow * priv)
{
  DestroyWindow(priv);
}

void LegendWindowRepaint(GtkWidget * widget, GdkEvent *, gpointer data)
{
  LegendWindow *priv = (LegendWindow *) data;
  if (!priv->ready)
    return;

  // this seems kind of hacky, but it may not
  // have been done yet...
  if (!priv->initialized)
    LegendWindowReshape(widget, NULL, data);

  if (!priv->mesh->data || !priv->mesh->cont)
    return;
  int numconts = priv->mesh->cont->numlevels;

  if (priv->matchContours)
    priv->nticks = numconts;

  // for the top and bottom
  int actualTicks = priv->nticks + 2;

  GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

  if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
    printf("Can't find where I should be drawing!\n");
    return;
  }

  int length = (*(priv->map))->max;
  float nextval, lastval, colorval;

  int loop;

  unsigned char color[3];

  //number of pixels to take off for intermediate lines on graph
  int size_adjuster;
  float delta;
  unsigned char *map = (*(priv->map))->map;
  float position[3] = { -1, priv->height - 20.f, 0 };
  float factor;
  float potmin, potmax;
  Surf_Data *cursurf = priv->surf;

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
  if (priv->matchContours) {
    priv->nticks = priv->mesh->cont->numlevels;
    for (unsigned i = 0; i < priv->mesh->cont->numlevels; i++)
      contvals.push_back(priv->mesh->cont->isolevels[i]);
  }
  else {
    float tick_range = (potmax-potmin)/(priv->nticks+1);
    for (unsigned i = 0; i < priv->nticks; i++)
      contvals.push_back(potmin + (tick_range*(i+1)));
  }
  contvals.push_back(potmax);

  float coloroffset = .5;

  if (priv->bgcolor[0] + .3 > 1 || priv->bgcolor[1] + .3 > 1 || priv->bgcolor[2] + .3 > 1)
    coloroffset = -coloroffset;

  glClearColor(priv->bgcolor[0], priv->bgcolor[1], priv->bgcolor[2], 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);

  // link info to geom window
  if (priv->mesh->gpriv->showinfotext) {
    /*position[0] = -6*CHAR_WIDTH*aspect*1.5;
      glprintf(position,normal,up,CHAR_WIDTH*aspect*1.5,CHAR_HEIGHT*1.5,"Surface # %d",cursurf->surfnum);
      position[0] = -((strlen(cursurf->potfilename)+3)/2.0f)*CHAR_WIDTH*aspect;
      position[1] -= CHAR_HEIGHT*1.5;
      glprintf(position,normal,up,CHAR_WIDTH*aspect,CHAR_HEIGHT,"%s@%d",cursurf->potfilename,cursurf->seriesnum);
    */

    // output surface and data file information
    if(!priv->orientation)//Horizontal
      position[0] = (priv->width /3) - (float)getFontWidth(priv->mesh->gpriv->large_font) * 6;
    else//vertical
      position[0] = (priv->width / 2) - (float)getFontWidth(priv->mesh->gpriv->large_font) * 6;
    
    position[1] = (float) (priv->height - getFontHeight(priv->mesh->gpriv->large_font));

    if (priv->mesh->geom->subsurf <= 0)
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->large_font, 
        "Surface # %d", priv->mesh->geom->surfnum);
    else
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->large_font, 
        "Surface # %d-%d", priv->mesh->geom->surfnum, priv->mesh->geom->subsurf);

    char * slash = shorten_filename(cursurf->potfilename);
    if(!priv->orientation)//Horizontal
      position[0] = (priv->width / 3) - ((float)getFontWidth(priv->mesh->gpriv->med_font) + 1) * (strlen(slash) + 2) / 2;
    else//vertical
      position[0] = (priv->width / 2) - ((float)getFontWidth(priv->mesh->gpriv->med_font) + 1) * (strlen(slash) + 2) / 2;

    position[1] -= getFontHeight(priv->mesh->gpriv->med_font)*.8f ;

    renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, "%s@%d", slash,
                   cursurf->seriesnum+1);

    // output spatial scaling information (which surfaces)
    if(!priv->orientation)//Horizontal
      position[1] = (float) (priv->height - getFontHeight(priv->mesh->gpriv->med_font));
    else//vertical
      position[1] -= getFontHeight(priv->mesh->gpriv->med_font) ;

    // set default to local if invalid things are set
    int scope = map3d_info.scale_scope;
    if (scope == SLAVE_FRAME && !cursurf->mastersurf)
      scope = LOCAL_SCALE;
    else if (scope == SLAVE_GLOBAL && !cursurf->mastersurf)
      scope = GLOBAL_SURFACE;

    if (cursurf->user_scaling && cursurf->userpotmin < cursurf->userpotmax) {
      if(!priv->orientation)//Horizontal
	position[0] = (priv->width *.8f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 10;
      else//vertical
	position[0] = (priv->width / 2.0f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 10;
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, "User-specified range");
    }
    else if (scope == SLAVE_FRAME || scope == SLAVE_GLOBAL) {
      if(!priv->orientation)//Horizontal
	position[0] = (priv->width *.8f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 10.0f;
      else//vertical
	position[0] = (priv->width / 2.0f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 10.0f;
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, 
                     "Slave to Surface # %d",
                     cursurf->mastersurf->surfnum);
    }
    else if (scope == GROUP_FRAME || scope == GROUP_GLOBAL) {
      if(!priv->orientation)//Horizontal
	position[0] = (priv->width *.8f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 6.0f;
      else//vertical
	position[0] = (priv->width / 2.0f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 6.0f;
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, 
                     "Group # %d", cursurf->mesh->groupid + 1);
    }
    else if (scope == GLOBAL_GLOBAL || scope == GLOBAL_FRAME) {
      if(!priv->orientation)//Horizontal
	position[0] = (priv->width *.8f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 7;
      else//vertical
	position[0] = (priv->width / 2.0f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 7;
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, "All surfaces");
    }
    else if (scope == LOCAL_SCALE || scope == GLOBAL_SURFACE) {
      if(!priv->orientation)//Horizontal
	position[0] = (priv->width *.8f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 8;
      else//vertical
	position[0] = (priv->width / 2.0f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 8;
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, "Local surface");
    }

    // output temporal scaling information (which frames)
    position[1] -= getFontHeight(priv->mesh->gpriv->med_font)*.8f ;

    if (scope == GLOBAL_SURFACE || scope == GLOBAL_GLOBAL || scope == GROUP_GLOBAL || scope == SLAVE_GLOBAL) {
      if(!priv->orientation)//Horizontal
	position[0] = (priv->width *.8f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 6;
      else//vertical
	position[0] = (priv->width / 2.0f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 6;
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, "All frames");
    }
    else if (scope == GLOBAL_FRAME || scope == LOCAL_SCALE || scope == GROUP_FRAME ||
             scope == SLAVE_FRAME) {
      if(!priv->orientation)//Horizontal
	position[0] = (priv->width *.8f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 6;
      else//vertical
	position[0] = (priv->width / 2.0f) - (float)getFontWidth(priv->mesh->gpriv->med_font) * 6;
      renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->med_font, "Frame # %d",
                     cursurf->framenum + 1);
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
  if (!priv->orientation) {
    bottom = 40;
    //vsize = ((priv->height > 150) ? (int)(priv->height * .75 - 30) : (int)(priv->height - 80));
    left = 20;
    size = priv->width - 40;
    if (priv->mesh->gpriv->showinfotext)
      vsize = priv->height - (getFontHeight(priv->mesh->gpriv->large_font) +
			     getFontHeight(priv->mesh->gpriv->med_font)+10);
    else
      vsize = priv->height - 20;
    //     size = priv->height - (getFontHeight(priv->mesh->gpriv->large_font) +
    //  			   3*getFontHeight(priv->mesh->gpriv->med_font)+20);
  }
  //vertical
  else {
    if (priv->mesh->gpriv->showinfotext)
      size = priv->height - (getFontHeight(priv->mesh->gpriv->large_font) +
			     3*getFontHeight(priv->mesh->gpriv->med_font)+20);
    else
      size = priv->height - 40;
    hsize = priv->width - 5*getFontWidth(priv->mesh->gpriv->small_font)-20;
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
  if (priv->mesh->drawmesh == RENDER_MESH_ELTS || priv->mesh->drawmesh == RENDER_MESH_ELTS_CONN) {
    if (priv->mesh->gpriv->secondarysurf == priv->mesh->geom->surfnum - 1)
      glColor3f(priv->mesh->secondarycolor[0], priv->mesh->secondarycolor[1], priv->mesh->secondarycolor[2]);
    else
      glColor3f(priv->mesh->meshcolor[0], priv->mesh->meshcolor[1], priv->mesh->meshcolor[2]);
    //vertical
    if (priv->orientation) {
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
  else if (priv->mesh->shadingmodel == SHADE_GOURAUD || 
           priv->mesh->shadingmodel == SHADE_FLAT ||
           (priv->mesh->shadingmodel == SHADE_BANDED && !priv->matchContours)) {

    glBegin(GL_QUAD_STRIP);

    for (loop = 0; loop < length; loop++) {
      getContColor(potmin + loop * (potmax - potmin) / length, potmin, potmax, *(priv->map), color, priv->mesh->invert);
      //glColor3ub(map[loop*3],map[loop*3+1],map[loop*3+2]);
      glColor3ubv(color);
      if (!priv->orientation) {
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
  else if (priv->mesh->shadingmodel == SHADE_BANDED) {
    // assumes matched contours
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    for (loop = 0; loop < contvals.size()-1; loop++) {
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
      getContColor(colorval, potmin, potmax, *(priv->map), color, priv->mesh->invert);
      glColor3ubv(color);


      //horiz
      if (!priv->orientation) {
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
  glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
  glBegin(GL_LINES);

  //horiz
  if (!priv->orientation) {
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
  for (loop = 0; loop < contvals.size(); loop++) {
    nextval = contvals[loop];
    if (priv->mesh->shadingmodel == SHADE_NONE) {
      if (loop == 0 || loop == contvals.size()-1){
        glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
   	  }
      else {
        getContColor(nextval, potmin, potmax, *(priv->map), color, priv->mesh->invert);
        glColor3ubv(color);
      }
    }
    //horiz
    if (!priv->orientation) {
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

  if (priv->orientation) {
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
  int endpos = (int)(left - getFontWidth(priv->mesh->gpriv->small_font) * 2.5 + size);
  int font_height = getFontHeight(priv->mesh->gpriv->small_font);

  for (loop = 0; loop < contvals.size(); loop++) {
    glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);
    nextval = contvals[loop];

    if (nextval > -.1 && nextval < .1) // 3-decimal precision
      sprintf(string, "%.3f", nextval);
    else if (nextval >= .1 && nextval < 10) //2 decimal precision
      sprintf(string, "%.2f", nextval);
    else                      //1 decimal precision
      sprintf(string, "%.1f", nextval);
    //horizontal
    if (!priv->orientation) {
      position[0] = left - getFontWidth(priv->mesh->gpriv->small_font) * 2.5f + ((nextval - potmin) / (potmax - potmin) * size);
      //glprintf(position,normal,up,mod_width*aspect,mod_height,"%.2f\0",nextval);
      if (loop == 0 || loop == contvals.size()-1 || (rowpos[stagger] + getFontWidth(priv->mesh->gpriv->small_font) * 6 < position[0] &&
          (position[0] + getFontWidth(priv->mesh->gpriv->small_font) * 6 < endpos ||
          rowpos[(stagger + 1) % 2] + getFontWidth(priv->mesh->gpriv->small_font) * 6 < endpos))) {
        glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);
        renderString3f(position[0], position[1] - 12 * stagger, position[2], priv->mesh->gpriv->small_font, string);
        //Extend the contour line if number is staggered(on the bottom)
        if(stagger == 1){
          glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
          if (priv->mesh->shadingmodel == SHADE_NONE && loop != 0 && loop != contvals.size()-1) {
            getContColor(nextval, potmin, potmax, *(priv->map), color, priv->mesh->invert);
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
        renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->small_font, string);
        prevcont = (int)position[1];
        if (loop != 0 && loop != contvals.size()-1) {
          // extend the contour line, so we can see which value it points to
          glBegin(GL_LINES);
          if (priv->mesh->shadingmodel != SHADE_NONE) {
            glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
          }
          else {
            getContColor(nextval, potmin, potmax, *(priv->map), color, priv->mesh->invert);
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

  if (gdk_gl_drawable_is_double_buffered(gldrawable))
    gdk_gl_drawable_swap_buffers(gldrawable);
  else
    glFlush();

  gdk_gl_drawable_gl_end(gldrawable);
}

void LegendWindowKeyboardRelease(GtkWidget *, GdkEventKey *, gpointer)
{
}
void LegendWindowKeyboardPress(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  //key_data *data = new key_data;
  //data->key = key;
  //data->x = x;
  //data->y = y;
  LegendWindowHandleKeyboard(widget, event, data);
  //  delete data;
}

void LegendWindowHandleKeyboard(GtkWidget *, GdkEventKey * event, gpointer data)
{
  LegendWindow *priv = (LegendWindow *) data;
  //if (HandleKeyboard(data))     // already handled by window manager
  //return;

  switch (event->keyval) {
  case 'q':
    if (masterWindow)
      gtk_widget_hide(priv->drawarea);
    else
      gtk_widget_hide(priv->window);
    priv->mesh->showlegend = false;
    break;
  }
}

void LegendWindowReshape(GtkWidget *widget, GdkEvent * event, gpointer data)
{
  LegendWindow *priv = (LegendWindow *) data;
  if (!priv->ready)
    return;

  LegendWindowInit(widget, event, data);
  gdk_gl_drawable_gl_begin(priv->gldrawable, priv->glcontext);
  priv->width = widget->allocation.width;
  priv->height = widget->allocation.height;

  glViewport(0, 0, priv->width, priv->height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, priv->width, 0, priv->height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gdk_gl_drawable_gl_end(priv->gldrawable);

  gtk_widget_queue_draw(priv->drawarea);
}

void LegendWindowButtonRelease(GtkWidget *, GdkEventButton * event, gpointer data)
{
  LegendWindow *priv = (LegendWindow *) data;
  int x = (int)event->x;
  int y = (int)(priv->height - event->y);

  y = priv->height - y;

  priv->curx = x;
  priv->cury = y;

  gtk_widget_queue_draw(priv->drawarea);

}

void LegendWindowButtonPress(GtkWidget * widget, GdkEventButton * event, gpointer data)
{

  LegendWindow *priv = (LegendWindow *) data;
  //int x = (int)event->x;
  //int y = (int)(priv->height - event->y);
  SetCurrentWindow(priv->winid);

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

  if (event->button == 3)       // right click
    gtk_menu_popup(GTK_MENU(priv->menu), NULL, NULL, NULL, NULL, event->button, event->time);
  /* LEFT MOUSE DOWN + ALT = start move window */
  if (event->button == 1 && COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK)) {
    priv->startx = (int)event->x_root;
    priv->starty = (int)event->y_root;
  }
  /* MIDDLE MOUSE DOWN + ALT = start resize window */
  if (event->button == 2 && COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK)) {
    priv->startx = (int)event->x_root;
    priv->starty = (int)event->y_root;
  }

  gtk_widget_queue_draw(priv->drawarea);
}

void LegendWindowMouseMotion(GtkWidget *, GdkEventMotion * event, gpointer data)
{
  LegendWindow *priv = (LegendWindow *) data;
  //int x = (int)event->x;
  //int y = (int)(priv->height - event->y);

  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }

  if (COMPARE_MODIFIERS(event->state, (GDK_MOD1_MASK | GDK_BUTTON1_MASK))) {
    priv->moveWindow(event);
  }
  else if (COMPARE_MODIFIERS(event->state, (GDK_MOD1_MASK | GDK_BUTTON2_MASK))) 
    {
      priv->sizeWindow(event);
    }
  gtk_widget_queue_draw(priv->drawarea);
}

void LegendWindowMenu(int)
{
}

void MapOrientation(menu_data * data)
{

  if (menulock)
    return;
  
  int entry = data->data;
  LegendWindow *priv = (LegendWindow *) data->priv; //GetCurrentWindow();
  int temp = priv->orientation;

  priv->orientation = entry;
  if (priv->orientation != temp) {
    temp = priv->width;
    priv->width = priv->height;
    priv->height = temp;
    
    if (masterWindow != NULL) {
      gtk_widget_set_size_request(priv->drawarea, priv->width, priv->height);
      // call to resize children is so the children draw in a logical manner
      gtk_container_resize_children(GTK_CONTAINER(masterWindow->fixed));
    }
    else
      gtk_window_resize(GTK_WINDOW(priv->window),priv->width, priv->height);
  }

  //LegendWindowReshape(priv->drawarea, NULL, priv);
  
  if(entry == 1){
    menulock = true;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->horiz_orient), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->vert_orient), true);
    menulock = false;
  }
  else if (entry == 0){
    menulock = true;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->vert_orient), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->horiz_orient), true);
    menulock = false;
  }

}

void MapTicks(menu_data * data)
{
  if(menulock)
    return;
  
  int entry = data->data;
  LegendWindow *priv = (LegendWindow *) data->priv; //GetCurrentWindow();

  if (entry == 128) {
    entry = priv->surf->numconts;
    priv->matchContours = 1;
    menulock = true;
    priv->ticks.setActive(7); // the index for match contours
    menulock = false;
  }
  else {
    priv->matchContours = 0;
    menulock = true;
    priv->ticks.setActive(entry); // the indices start at 0, and the first one is 0
    menulock = false;
  }
  priv->nticks = entry;

}
