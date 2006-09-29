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
  showinfotext = 1;
  is_displayed = 1;
  matchContours = 1;

  menu = gtk_menu_new();
  GtkWidget *ms = AddSubMenu(menu, "Orientation");
  vert_orient = AddCheckMenuEntry(ms, "Vertical", 1, this, MapOrientation);
  horiz_orient = AddCheckMenuEntry(ms, "Horizontal", 0, this, MapOrientation);
  GtkWidget *cm = AddSubMenu(menu, "Number of Tick Marks");
  tick2 = AddCheckMenuEntry(cm, "2", 2, this, MapTicks);
  tick4 = AddCheckMenuEntry(cm, "4", 4, this, MapTicks);
  tick8 = AddCheckMenuEntry(cm, "8", 8, this, MapTicks);
  tick_match = AddCheckMenuEntry(cm, "Match Contours", 128, this, MapTicks);
  menulock = true;
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(vert_orient), true);
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(tick_match), true);
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

  GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

  if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
    printf("Can't find where I should be drawing!\n");
    return;
  }

  int length = (*(priv->map))->max;
  float nextval, lastval, colorval;

  int tick_length = length / (priv->nticks - 1);
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
  if (!priv->mesh->data || !priv->mesh->cont)
    return;
  int numconts = priv->mesh->cont->numlevels;

  cursurf->get_minmax(potmin, potmax);

  float coloroffset = .5;

  if (priv->bgcolor[0] + .3 > 1 || priv->bgcolor[1] + .3 > 1 || priv->bgcolor[2] + .3 > 1)
    coloroffset = -coloroffset;

  glClearColor(priv->bgcolor[0], priv->bgcolor[1], priv->bgcolor[2], 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);

  if (priv->showinfotext) {
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
    if (priv->showinfotext)
      vsize = priv->height - (getFontHeight(priv->mesh->gpriv->large_font) +
			     getFontHeight(priv->mesh->gpriv->med_font)+10);
    else
      vsize = priv->height - 20;
    //     size = priv->height - (getFontHeight(priv->mesh->gpriv->large_font) +
    //  			   3*getFontHeight(priv->mesh->gpriv->med_font)+20);
  }
  //vertical
  else {
    if (priv->showinfotext)
      size = priv->height - (getFontHeight(priv->mesh->gpriv->large_font) +
			     3*getFontHeight(priv->mesh->gpriv->med_font)+20);
    else
      size = priv->height - 40;
    hsize = ((priv->width > 180) ? (int)(priv->width * .75) : (int)(priv->width - 50));
    bottom = 20;
    left = 20;
  }

  //set up window vars
  if (size / priv->nticks > 3)
    size_adjuster = 0;
  else if (size / priv->nticks > 2)
    size_adjuster = 1;
  else
    size_adjuster = 2;
  delta = ((float)size) / length;
  factor = (float)(size / (priv->nticks - 1));

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
  //gouraud shading
  else if (priv->mesh->shadingmodel == SHADE_GOURAUD || 
           priv->mesh->shadingmodel == SHADE_FLAT) {

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
    if (priv->matchContours) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glBegin(GL_QUADS);
      for (loop = -1; loop < numconts; loop++) {
        if (loop == -1) {
          nextval = priv->mesh->cont->isolevels[loop + 1];
          lastval = potmin;
          colorval = potmin;
        }
        else if (loop == numconts - 1) {
          nextval = potmax;
          lastval = priv->mesh->cont->isolevels[loop];
          colorval = potmax;
        }
        else {
          nextval = priv->mesh->cont->isolevels[loop + 1];
          lastval = priv->mesh->cont->isolevels[loop];
          float* conts = priv->mesh->cont->isolevels;
          colorval = conts[loop] + (conts[loop+1]-conts[loop])*(loop+1)/(numconts);
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
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glBegin(GL_QUADS);
      for (loop = 0; loop < priv->nticks - 1; loop++) {
        nextval = priv->mesh->cont->isolevels[loop];
        if (!priv->mesh->invert)
          glColor3ub(map[loop * tick_length * 3], map[loop * tick_length * 3 + 1], map[loop * tick_length * 3 + 2]);
        else
          glColor3ub(map[(priv->nticks - loop - 2) * tick_length * 3],
                     map[(priv->nticks - loop - 2) * tick_length * 3 + 1],
                     map[(priv->nticks - loop - 2) * tick_length * 3 + 2]);
        //horiz
        if (!priv->orientation) {
          glVertex2d(left + loop * factor, vsize);
          glVertex2d(left + loop * factor, bottom);
          glVertex2d(left + loop * factor + factor, bottom);
          glVertex2d(left + loop * factor + factor, vsize);

        }
        //vert
        else {
          glVertex2d(left, bottom + loop * factor);
          glVertex2d(hsize, bottom + loop * factor);
          glVertex2d(hsize, bottom + loop * factor + factor);
          glVertex2d(left, bottom + loop * factor + factor);
        }
      }
      glEnd();
    }

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
  if (priv->matchContours) {
    for (loop = -1; loop <= numconts; loop++) {
      if (loop == -1)
        nextval = potmin;
      else if (loop == numconts)
        nextval = potmax;
      else
        nextval = priv->mesh->cont->isolevels[loop];
      if (priv->mesh->shadingmodel == SHADE_NONE) {
        if (loop == -1 || loop == numconts){
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
        glVertex2d(hsize + 5, bottom + (nextval - potmin) / (potmax - potmin) * size);
      }

    }
  }
  else {
    for (loop = 0; loop < priv->nticks; loop++) {
      if (priv->mesh->shadingmodel != SHADE_NONE)
        if (!priv->mesh->invert)
          glColor3ub(map[loop * tick_length * 3], map[loop * tick_length * 3 + 1], map[loop * tick_length * 3 + 2]);
        else
          glColor3ub(map[(priv->nticks - loop - 1) * tick_length * 3],
                     map[(priv->nticks - loop - 1) * tick_length * 3 + 1],
                     map[(priv->nticks - loop - 1) * tick_length * 3 + 2]);

      //horiz
      if (!priv->orientation) {
        glVertex2d(left + loop * factor, vsize);
        glVertex2d(left + loop * factor, bottom - 5);
      }
      else {
        glVertex2d(left, bottom + loop * factor);
        glVertex2d(hsize + 5, bottom + loop * factor);
      }
    }
  }
  glEnd();
  glLineWidth(3);

  position[0] = (float)hsize + 10;
  position[1] = 0;
  position[2] = 0;
  glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);

  //write contour values
  //horizontal
  if (!priv->orientation) {
    position[0] = (float)left;
    position[1] = (float)bottom - 15;
    position[2] = 0;

    //int numconts = priv->nticks;
    int stagger = 0;            //which row you're on
    int rowpos[2] = { -15, -100 };  //
    int endpos = (int)(left - getFontWidth(priv->mesh->gpriv->small_font) * 2.5 + size);
    char string[256] = { '\0' };
    for (loop = -1; loop <= numconts; loop++) {
      if (loop == -1)
        nextval = potmin;
      else if (loop == numconts)
        nextval = potmax;
      else
        nextval = priv->mesh->cont->isolevels[loop];

      if (nextval >= 0 && nextval < 10) //2 decimal precision
        sprintf(string, "%.2f", nextval);
      else                      //1 decimal precision
        sprintf(string, "%.1f", nextval);

      position[0] = left - getFontWidth(priv->mesh->gpriv->small_font) * 2.5f + ((nextval - potmin) / (potmax - potmin) * size);
      //glprintf(position,normal,up,mod_width*aspect,mod_height,"%.2f\0",nextval);
      if (loop == -1 || loop == numconts) {
	glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);
        renderString3f(position[0], position[1] - 12 * stagger, position[2], priv->mesh->gpriv->small_font, string);
	//Extend the contour line if number is staggered(on the bottom)
	if(stagger == 1){
	  glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
	  if (priv->matchContours) {
	    if (priv->mesh->shadingmodel == SHADE_NONE) {
	      if (loop == -1 || loop == numconts){
		glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
	      }
	      else {
		getContColor(nextval, potmin, potmax, *(priv->map), color, priv->mesh->invert);
		glColor3ubv(color);
	      }
	    }
	    glLineWidth((float)(3 - size_adjuster));
	    glBegin(GL_LINES);
	    glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, bottom - 5);  
	    glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, position[1] - 2 * stagger);
	    glEnd();
	    glLineWidth(3);
	  }
	}
	//end draw contour line
        rowpos[stagger] = (int)position[0];
        stagger = (stagger + 1) % 2;

      }
      else
        if (rowpos[stagger] + getFontWidth(priv->mesh->gpriv->small_font) * 6 < position[0] &&
            (position[0] + getFontWidth(priv->mesh->gpriv->small_font) * 6 < endpos ||
	     rowpos[(stagger + 1) % 2] + getFontWidth(priv->mesh->gpriv->small_font) * 6 < endpos)) {
	  glColor3f(priv->fgcolor[0], priv->fgcolor[1], priv->fgcolor[2]);
	  renderString3f(position[0], position[1] - 12 * stagger, position[2], priv->mesh->gpriv->small_font, string);
	  //Extend the contour line if number is staggered(on the bottom)
	  if(stagger == 1){
	    glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
	    if (priv->matchContours) {
	      if (priv->mesh->shadingmodel == SHADE_NONE) {
		if (loop == -1 || loop == numconts){
		  glColor3f(priv->bgcolor[0] + coloroffset, priv->bgcolor[1] + coloroffset, priv->bgcolor[2] + coloroffset);
		}
		else {
		  getContColor(nextval, potmin, potmax, *(priv->map), color, priv->mesh->invert);
		  glColor3ubv(color);
		}
	      }
	      glLineWidth((float)(3 - size_adjuster));
	      glBegin(GL_LINES);
	      glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, bottom - 5); 
	      glVertex2d(left + (nextval - potmin) / (potmax - potmin) * size, position[1] - 2 * stagger);
	      glEnd();
	      glLineWidth(3);
	    }
	  }
	  //end draw contour line
	  rowpos[stagger] = (int)position[0];
	  stagger = (stagger + 1) % 2;
	}
    }

  }
  //vertical
  else {
    //int numconts = priv->nticks;
    int prevcont = bottom;
    int lastcont = bottom + size;
    for (loop = -1; loop <= numconts; loop++) {
      if (loop == -1)
        nextval = potmin;
      else if (loop == numconts)
        nextval = potmax;
      else
        nextval = priv->mesh->cont->isolevels[loop];
      position[1] = bottom - 3 + ((nextval - potmin) / (potmax - potmin) * size);
      if (loop == -1)
        prevcont = (int)position[1];


      //determines based on 12 pixels per character which contour val to write
      if (loop == -1 || loop == numconts)
        renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->small_font, "%.2f\0", nextval);
      else if (position[1] >= prevcont + 12 && position[1] <= lastcont - 12) {
        renderString3f(position[0], position[1], position[2], priv->mesh->gpriv->small_font, "%.2f\0", nextval);
        prevcont = (int)position[1];
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
    entry = priv->surf->numconts + 2;
    priv->matchContours = 1;
    menulock = true;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick2), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick4), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick8), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick_match), true);
    menulock = false;
  }
  else if(entry == 2){
    priv->matchContours = 0;
    menulock = true;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick2), true);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick4), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick8), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick_match), false);
    menulock = false;
  }
  else if(entry == 4){
    priv->matchContours = 0;
    menulock = true;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick2), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick4), true);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick8), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick_match), false);
    menulock = false;
  }
  else if(entry == 8){
    priv->matchContours = 0;
    menulock = true;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick2), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick4), false);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick8), true);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->tick_match), false);
    menulock = false;
  }
  priv->nticks = entry;

}
