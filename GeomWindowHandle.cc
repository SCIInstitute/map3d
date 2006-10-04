/* GeomWindowHandle.cxx */

#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#else
#include <unistd.h>
#endif
#ifdef OSX
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include "GeomWindow.h"
#include "WindowManager.h"
#include "Contour_Info.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "Transforms.h"
#include "BallMath.h"
#include "colormaps.h"
#include "eventdata.h"
#include "landmarks.h"
#include "map3dmath.h"
#include "glprintf.h"
#include "scalesubs.h"
#include "dialogs.h"
#include "lock.h"
#include "pickinfo.h"
#include "reportstate.h"
#include "GeomWindowMenu.h"
#include "MainWindow.h"
#include "savescreen.h"

extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
extern SaveDialog *savedialog;
extern ColorPicker *cp;
extern SizePicker *sp;
extern vector<Surface_Group> surf_group;
extern int fstep;

#define CHAR_WIDTH .07
#define CHAR_HEIGHT .07

int key_pressed = 0;            //indicate whether (and which) key is held down
int idle_iteration = 0;

extern bool menulock;

// callback from GTK on a geom window.  If it is something that needs
// to happen once per mesh or once per window, we will call
// GeomWindowHandleMenu either directly or via Broadcast.
// Otherwise, we will handle it here.
void GeomWindowMenu(menu_data * data)
{
  // menulock is for options, like changing status of menu check items,
  // which would recursively call the menu callback
  if (menulock) 
    return;

  // check to see it we handle options that apply globally
  if (GeomWindowMenuGlobalOptions(data))
    return;

  GeomWindow *priv = (GeomWindow*) data->priv;
  int menu = data->data;

  map3d_info.selected_group = (map3d_info.lockgeneral == LOCK_GROUP && priv->meshes.size() > 0)
    ? priv->meshes[0]->groupid : -1;

  //careful with this - some things we want very specific on when to be
  //broadcasted
  if (map3d_info.lockgeneral || 
    (map3d_info.lockframes && (menu == frame_reset || menu == frame_zero))){
    Broadcast(MAP3D_MENU, data);
  }
  else{
    GeomWindowHandleMenu(data);
  }
  if (menu == frame_align || menu == frame_reset)
    Broadcast(MAP3D_REPAINT_ALL, 0);
  if (map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_other_events))) {
    SaveScreen();
  } 

  // reset the entries in ColorPicker and SizePicker so the next call to PickColor or Size doesn't just
  // append to the list of colors/sizes.  This is the best place, as it can be broadcasted.
  if (cp) 
    cp->post_change = true;
  if (sp) 
    sp->post_change = true;
}



bool GeomWindowHandleFrameAdvances(gpointer data)
{
  GeomWindow *priv = (GeomWindow *) data;
  if (!key_pressed) {
    return false;
  }
  GdkEventKey event;
  event.keyval = key_pressed;
  map3d_info.scale_frame_set = 0;

  map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP && priv->meshes.size() > 0)
    ? priv->meshes[0]->groupid : -1;

  if (map3d_info.lockframes) {
    Broadcast(MAP3D_FRAMES, 0, (GdkEvent*) &event, data);
  }
  else {
    // advance the only mesh in this window or only the dominant (if gen lock off too)
    // or only the secondary surf
    int index = priv->dominantsurf != -1 ? priv->dominantsurf : priv->secondarysurf != -1 ? priv->secondarysurf : 0;
    Surf_Data* sd = priv->meshes[index]->data;
    int delta_frames = 0;
    if (event.keyval == GDK_Left) 
      delta_frames = -fstep;
    else if (event.keyval == GDK_Right) 
      delta_frames = fstep;

    sd->FrameAdvance(delta_frames);

    // if advancing in time only affects this surface
    if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
        map3d_info.scale_scope != SLAVE_FRAME) {
      GeomWindowUpdateAndRedraw(priv);
    }
    // it affects at least one other surface...
    else {
      Broadcast(MAP3D_UPDATE, 0);
    }
  }
  idle_iteration++;
  if (map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_frame_advance))) {
    SaveScreen();
  } 
  return true;
}

void GeomWindowKPress(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  GeomWindow *priv = (GeomWindow *) data;
  int loop = 0, loop2 = 0;
  int length = priv->meshes.size();

  Mesh_Info *mesh = 0;
  Clip_Planes *clip = priv->clip;
  LegendWindow *lpriv = 0;
  menu_data mdata(0, priv);

  if (length > 1 && !map3d_info.lockgeneral) {
    loop = priv->dominantsurf;
    length = loop + 1;
  }
  
  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }

  bool first = true;
  for (; loop < length; loop++)
  {
    mesh = priv->meshes[loop];
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;

    // for lining up meshes with cycling things like shading
    Mesh_Info* tmpmesh = mesh;
    if (map3d_info.lockgeneral)
      tmpmesh = getFirstMesh();

    // call keyboard transformations function - the keys between home and end
    // should be all the keys on the keypad
    if (event->keyval >= GDK_Home && event->keyval <= GDK_End && 
        (event->state == GDK_CONTROL_MASK || event->state == GDK_MOD1_MASK)) {
      GeomWindowTransformKeyboard(mesh, event);
      continue;
    }
    switch (event->keyval) {
    case GDK_Escape:
      if (masterWindow != NULL)
        map3d_quit(NULL);
      else
        map3d_quit(priv->window);
      break;
    }

    // with the toggle controls there is an interesting phenomenon such that if you
    // turn the general lock off, change a few things, and turn it back on, when you 
    // change things, they change in and out and never line up.  To avoid that, we want
    // to 
    switch (event->keyval) {

      /* l = toggle lighting */
    case 'l':
      MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, lighting);
      break;

      /* L - reLoad geometry and data */

    case 'L':
      mdata.data = mesh_both_reload;
      GeomWindowHandleMenu(&mdata);
      loop = length;
      break;
    case 'g':
      if (mesh->gouraudstyle == SHADE_GOURAUD)
        mesh->gouraudstyle = SHADE_TEXTURED;
      else
        mesh->gouraudstyle = SHADE_GOURAUD;
      break;
      /* m = rotate show mesh */
    case 'm':
      // see comment in 's' for explanation of the ugliness
      switch (tmpmesh->drawmesh) {
      case RENDER_MESH_NONE:
        mdata.data = tmpmesh==mesh?mesh_render_points:mesh_render_none;
        GeomWindowHandleMenu(&mdata);
        break;
      case RENDER_MESH_PTS:
        mdata.data = tmpmesh==mesh?mesh_render_elements:mesh_render_points;
        GeomWindowHandleMenu(&mdata);
        break;
      case RENDER_MESH_ELTS:
        mdata.data = tmpmesh==mesh?mesh_render_connectivity:mesh_render_elements;
        GeomWindowHandleMenu(&mdata);
        break;
      case RENDER_MESH_CONN:
      case RENDER_MESH_ELTS_CONN:
      case RENDER_MESH_PTS_CONN:
      case RENDER_MESH_NONDATA_ELTS:
        mdata.data = tmpmesh==mesh?mesh_render_none:mesh_render_connectivity;
        GeomWindowHandleMenu(&mdata);
        break;
      }
      loop = length;
      break;

    case 'M':
      // see comment in 's' for explanation of the ugliness
      switch (tmpmesh->drawmesh) {
      case RENDER_MESH_NONE:
      case RENDER_MESH_ELTS_CONN:
      case RENDER_MESH_PTS_CONN:
      case RENDER_MESH_NONDATA_ELTS:
        mdata.data = tmpmesh==mesh?mesh_render_connectivity:mesh_render_none;
        GeomWindowHandleMenu(&mdata);
        break;
      case RENDER_MESH_PTS:
        mdata.data = tmpmesh==mesh?mesh_render_none:mesh_render_points;
        GeomWindowHandleMenu(&mdata);
        break;
      case RENDER_MESH_ELTS:
        mdata.data = tmpmesh==mesh?mesh_render_points:mesh_render_elements;
        GeomWindowHandleMenu(&mdata);
        break;
      case RENDER_MESH_CONN:
        mdata.data = tmpmesh==mesh?mesh_render_elements:mesh_render_connectivity;
        GeomWindowHandleMenu(&mdata);
        break;
      }
      loop = length;
      break;

      /* n = show node toggle */
    case 'n':
      MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, qshowpnts);
      break;

      /* c = show contours toggle */
    case 'c':
      MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, drawcont);
      break;

      /* r = reset all */
    case 'r':
      map3d_info.lockframes = LOCK_FULL;
      map3d_info.lockrotate = LOCK_FULL;
      map3d_info.lockgeneral = LOCK_FULL;
      priv->dominantsurf = -1;
      mesh->tran->reset();
      priv->showinfotext = 1;
      clip->init();
      break;

    case 'R':
      mesh->shadingmodel = SHADE_NONE;
      mesh->drawmesh = RENDER_MESH_CONN;
      break;

      /* S = rotate colormapped surface to next type or off */
    case 'S':
      // this mess is to line up all meshes when the lock is on
      // we want to call HandleMenu to keep the code consistent
      // especially so we only have to check the menu in one place
      switch (tmpmesh->shadingmodel) {
      case SHADE_NONE:
        mdata.data = tmpmesh==mesh?surface_render_banded:surface_render_none;
        GeomWindowHandleMenu(&mdata);
        break;
      case SHADE_FLAT:
        mdata.data = tmpmesh==mesh?surface_render_none:surface_render_flat;
        GeomWindowHandleMenu(&mdata);
        break;
      case SHADE_GOURAUD:
        mdata.data = tmpmesh==mesh?surface_render_none:surface_render_gouraud;
        GeomWindowHandleMenu(&mdata);
        break;
      case SHADE_BANDED:
        mdata.data = tmpmesh==mesh?surface_render_gouraud:surface_render_banded;
        GeomWindowHandleMenu(&mdata);
        break;
      }
      
	  loop = length; // this is so that the change doesn't happen n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in GeomWindowHAndleMenu
      break;

      /* s = rotate colormapped surface to next type or off
             go the opposite direction as S (see comments for S as to the mess)*/
    case 's':
      switch (tmpmesh->shadingmodel) {
      case SHADE_NONE:
        mdata.data = tmpmesh==mesh?surface_render_gouraud:surface_render_none;
        GeomWindowHandleMenu(&mdata);
        break;
      case SHADE_FLAT:
        mdata.data = tmpmesh==mesh?surface_render_gouraud:surface_render_none;
        GeomWindowHandleMenu(&mdata);
        break;
      case SHADE_GOURAUD:
        mdata.data = tmpmesh==mesh?surface_render_banded:surface_render_gouraud;
        GeomWindowHandleMenu(&mdata);
        break;
      case SHADE_BANDED:
        mdata.data = tmpmesh==mesh?surface_render_none:surface_render_banded;
        GeomWindowHandleMenu(&mdata);
        break;
        }
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in GeomWindowHAndleMenu
      break;

      // a - cycle colormaps
    case 'a':
      // similar mess to S for synchronizing cycling
      if (tmpmesh->cmap == &Rainbow) {
        mdata.data = tmpmesh==mesh?surface_color_jet:surface_color_rainbow;
        GeomWindowHandleMenu(&mdata);
      }
      else if (tmpmesh->cmap == &Green2Red) {
        mdata.data = tmpmesh==mesh?surface_color_rainbow:surface_color_red2green;
        GeomWindowHandleMenu(&mdata);
      }
      else if (tmpmesh->cmap == &Grayscale) {
        mdata.data = tmpmesh==mesh?surface_color_red2green:surface_color_grayscale;
        GeomWindowHandleMenu(&mdata);
      }
      else if (tmpmesh->cmap == &Jet) {
        mdata.data = tmpmesh==mesh?surface_color_grayscale:surface_color_jet;
        GeomWindowHandleMenu(&mdata);
      }
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in GeomWindowHAndleMenu
      break;
      // A - cycle colormaps - go opposite direction as a
    case 'A':
      if (tmpmesh->cmap == &Rainbow) {
        mdata.data = tmpmesh==mesh?surface_color_red2green:surface_color_rainbow;
        GeomWindowHandleMenu(&mdata);
      }
      else if (tmpmesh->cmap == &Green2Red) {
        mdata.data = tmpmesh==mesh?surface_color_grayscale:surface_color_red2green;
        GeomWindowHandleMenu(&mdata);
      }
      else if (tmpmesh->cmap == &Grayscale) {
        mdata.data = tmpmesh==mesh?surface_color_jet:surface_color_grayscale;
        GeomWindowHandleMenu(&mdata);
      }
      else if (tmpmesh->cmap == &Jet) {
        mdata.data = tmpmesh==mesh?surface_color_rainbow:surface_color_jet;
        GeomWindowHandleMenu(&mdata);
      }
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in GeomWindowHAndleMenu
      break;

      /* i = toggle info display */
    case 'i':
    case 'I':
      mdata.data = surface_color_invert;
      GeomWindowHandleMenu(&mdata);
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in GeomWindowHAndleMenu
      break;


      /* d = toggle fog (depth cue) */
    case 'd':
      MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, fogging);
      break;

      // x - toggle axes
    case 'x':
      MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, axes);
      break;

      // increment size of currently-selected feature
    case '+':
      if (mesh->current_size_selector && (first || mesh->current_size_mesh_based))
        incrSize(mesh->current_size_selector, mesh->current_size_increment,
                mesh->current_size_midpoint, map3d_info.sizeincrement);
      break;
    case '-':
      if (mesh->current_size_selector && (first || mesh->current_size_mesh_based))
        incrSize(mesh->current_size_selector, mesh->current_size_increment,
                mesh->current_size_midpoint, -map3d_info.sizeincrement);
      break;

    }
    //update legend window
    if (mesh && mesh->legendwin != 0) {
      lpriv = mesh->legendwin;
      gtk_widget_queue_draw(lpriv->drawarea);
    }
    // update all windows in the pick stack
    if (mesh->pickstacktop >= 0) {
      for (loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
        gtk_widget_queue_draw(mesh->pickstack[loop2]->pickwin->drawarea);
      }
    }
    first = false;
  }

  //once-per-window items instead of once per mesh
  switch (event->keyval) {
  case GDK_Left:
  case GDK_Right:
    // left or right was pressed
    break;
    /* enable/disable clipping planes */
  case '<':
    mdata.data = clip_front;
    GeomWindowHandleMenu(&mdata);
    break;
  case '>':
    mdata.data = clip_back;
    GeomWindowHandleMenu(&mdata);
    break;

    /* makes the clipping planes rotate with the mesh. */
  case ',':
    mdata.data = clip_with_object;
    GeomWindowHandleMenu(&mdata);
    //clip->lock_with_object = !clip->lock_with_object;
    break;

    /* makes the clipping planes translate together. */
  case '.':
    mdata.data = clip_together;
    GeomWindowHandleMenu(&mdata);
    //clip->lock_together = !clip->lock_together;
    break;

    /* move front clipping plane in +z direction */
  case ']':
    if (clip->lock_together) {
      if (clip->front + clip->step <= clip->frontmax)
        clip->front += clip->step;
      if (clip->back - clip->step >= -clip->frontmax)
        clip->back -= clip->step;
    }
    else if (clip->front + clip->step <= clip->frontmax)
      clip->front += clip->step;
    break;

    /* move rear clipping plane in +z direction */
  case '}':
    if (clip->lock_together) {
      if (clip->front + clip->step <= clip->frontmax)
        clip->front += clip->step;
      if (clip->back - clip->step >= -clip->frontmax)
        clip->back -= clip->step;
    }
    else if (clip->back - clip->step >= -clip->frontmax)
      clip->back -= clip->step;
    break;

    /* move front clipping plane in -z direction */
  case '[':
    if (clip->lock_together) {
      if (clip->front - clip->step >= -clip->backmax)
        clip->front -= clip->step;
      if (clip->back + clip->step <= clip->backmax)
        clip->back += clip->step;
    }
    else if (clip->front - clip->step >= -clip->backmax)
      clip->front -= clip->step;
    break;

    /* move rear clipping plane in -z direction */
  case '{':
    if (clip->lock_together) {
      if (clip->front - clip->step >= -clip->backmax)
        clip->front -= clip->step;
      if (clip->back + clip->step <= clip->backmax)
        clip->back += clip->step;
    }
    else if (clip->back + clip->step <= clip->backmax)
      clip->back += clip->step;
    break;

  case 'p':
    mdata.data = fid_map_toggle;//pick_info_display;
    GeomWindowHandleMenu(&mdata);
	loop = length; // this is so that the change happens n*n times (when 
	// the general lock is on) - n times in this loop and n times
	// in GeomWindowHAndleMenu
    break;

    /* lock/unlock frames */
  case 'f':
    mdata.data = frame_lock;
    GeomWindowMenu(&mdata);
	loop = length; // this is so that the change happens n*n times (when 
	// the general lock is on) - n times in this loop and n times
	// in GeomWindowHAndleMenu
    break;
  case 't':
    mdata.data = trans_lock;
    GeomWindowMenu(&mdata);
	loop = length; // this is so that the change happens n*n times (when 
	// the general lock is on) - n times in this loop and n times
	// in GeomWindowHAndleMenu
    break;
  case '(':
    if (priv->secondarysurf != -1 && priv->secondarysurf + 1 < (int)priv->meshes.size())
      priv->secondarysurf++;
    break;
  case ')':
    if (priv->secondarysurf != -1 && priv->secondarysurf - 1 >= 0)
      priv->secondarysurf--;
    break;
  case 'w':
    SaveScreen();
  }
  gtk_widget_queue_draw(widget);
}

void GeomWindowKRelease(GtkWidget *, GdkEventKey *, gpointer)
{
}
void GeomWindowKeyboardRelease(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  GeomWindow *priv = (GeomWindow *) data;
  key_pressed = idle_iteration = 0;

  // turn off idle loop
  if (priv->idle_id != 0) {
    gtk_idle_remove(priv->idle_id);
    priv->idle_id = 0;
  }

  // don't worry about the selected_group here.  Letting go doesn't change anything for non-selected groups
  
  //  if conditions...((!map3d_info.lockgeneral && key != 'r') || (key == 'f') ||
  //           (key == 't') || (key == '(') || (key == ')') || (key == 'p'))
  if (!map3d_info.lockgeneral)
    GeomWindowKRelease(widget, event, data);
  else
    Broadcast(MAP3D_KEY_RELEASE, widget, (GdkEvent *) event, data);
}

void GeomWindowKeyboardPress(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  GeomWindow *priv = (GeomWindow *) data;

  int key = event->keyval;

  //set up here so both Broadcast branches can access it.  Up/down/left/right won't need it
  int lock;
  if (event->keyval >= GDK_Home && event->keyval <= GDK_End && 
      (event->state == GDK_CONTROL_MASK || event->state == GDK_MOD1_MASK)) {
    lock = map3d_info.lockrotate;
  }
  else {
    lock = map3d_info.lockgeneral;
  }
  map3d_info.selected_group = (lock == LOCK_GROUP && priv->meshes.size() > 0)
    ? priv->meshes[0]->groupid : -1;

  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }
  

  // okay, the way frame advancing works, so that the window manager won't get bogged down, is
  // we have the idle loop handle it.  However, we also want to be able to select only one frame,
  // so have the first key handle one, and then on the second time (key repeat should be on), turn 
  // on the idle loop.
  if ((event->keyval == GDK_Left || event->keyval == GDK_Right) &&
       (event->state != GDK_CONTROL_MASK && event->state != GDK_MOD1_MASK)) {
    key_pressed = event->keyval;

    if (idle_iteration >= 1) {
      if (priv->idle_id == 0) {
        priv->idle_id = gtk_idle_add_priority(G_PRIORITY_DEFAULT_IDLE,(GtkFunction) GeomWindowHandleFrameAdvances, priv);
      }
    }
    else {
      idle_iteration++;
      GeomWindowHandleFrameAdvances(priv); // call it directly
    }
    return;
  }
  if (event->keyval == GDK_Escape) {
    if (masterWindow != NULL)
      map3d_quit(NULL);
    else
      map3d_quit(priv->window);
    return;
  }
  else if (event->keyval == GDK_Up && event->state != GDK_CONTROL_MASK && event->state != GDK_MOD1_MASK){
    if (map3d_info.lockgeneral == LOCK_GROUP || (surf_group.size() == 1 && priv->dominantsurf+1 >= (int) priv->meshes.size())) {
      map3d_info.lockgeneral = LOCK_FULL;
    }
    else if (map3d_info.lockgeneral == LOCK_OFF && surf_group.size() > 1 && priv->dominantsurf+1 >= (int) priv->meshes.size()) {
      map3d_info.lockgeneral = LOCK_GROUP;
    }

    if (map3d_info.lockgeneral != LOCK_OFF) {
      for (unsigned i = 0; i < numGeomWindows(); i++) {
        GeomWindow* gwin = GetGeomWindow(i);

        gwin->dominantsurf = -1;
      }      
    }
    else {
      priv->dominantsurf++;
    }
#if 0
    // 2 passes - determine whether the lock needs to stay off, then
    // set the values for each window
    unsigned i;
    for (i = 0; i < numGeomWindows(); i++) {
      GeomWindow* gwin = GetGeomWindow(i);

      if (gwin->dominantsurf +1 < (int)gwin->meshes.size() && gwin->dominantsurf != -1) {
        lock = false;
        break;
      }
    }

    // if surfaces are not yet locked, set dominant surf to highest mesh possible
    for (i = 0; i < numGeomWindows(); i++) {
      GeomWindow* gwin = GetGeomWindow(i);

      if (gwin->dominantsurf == -1) {
        // do nothing
      }
      else {
        gwin->dominantsurf++;
        if (gwin->dominantsurf >= (int)gwin->meshes.size()) {
          if (!lock)
            gwin->dominantsurf = (int)gwin->meshes.size() -1;
          else
            gwin->dominantsurf = -1;
        }
        else {
          // keep lock off - some windows haven't gone through the last window
          lock = false;
        }
      }
    }
#endif
    Broadcast(MAP3D_REPAINT_ALL, 0);
  }
  else if (event->keyval == GDK_Down && event->state != GDK_CONTROL_MASK && event->state != GDK_MOD1_MASK) {
    if (map3d_info.lockgeneral == LOCK_FULL &&  surf_group.size() > 1) {
      // only turn on "group" lock - where we lock the groups together
      map3d_info.lockgeneral = LOCK_GROUP;
    }
    else if (map3d_info.lockgeneral != LOCK_OFF){
      for (unsigned i = 0; i < numGeomWindows(); i++) {
        GeomWindow* gwin = GetGeomWindow(i);
        if (gwin->dominantsurf == -1) {
          gwin->dominantsurf = gwin->meshes.size()-1;
        }
      }
      map3d_info.lockgeneral = LOCK_OFF;
    }
    else if (priv->dominantsurf > 0) {
      priv->dominantsurf--;
    }
    Broadcast(MAP3D_REPAINT_ALL, 0);
  }
  else if ((!map3d_info.lockgeneral && key != 'r') || (key == 'f') ||
    (key == 't') || (key == '(') || (key == ')') || (key == 'w') ) {
    GeomWindowKPress(widget, event, data);
    Broadcast(MAP3D_REPAINT_ALL, 0);
  }
  else
    Broadcast(MAP3D_KEY_PRESS, widget, (GdkEvent *) event, data);

  if (map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_other_events))) {
    SaveScreen();
  }
  // otherwise if it was a transformation event (this is the same check to see if it was a keyboard transform
  else if ((map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_transformation))) &&
           event->keyval >= GDK_Home && event->keyval <= GDK_End && 
           (event->state == GDK_CONTROL_MASK || event->state == GDK_MOD1_MASK)) {
    SaveScreen();
  }

}

void GeomWindowUpdateAndRedraw(GeomWindow* priv) 
{
  int loop = 0;
  int length = priv->meshes.size();
  if (length > 0 && !map3d_info.lockframes && map3d_info.scale_scope != GLOBAL_FRAME &&
      map3d_info.scale_scope != GROUP_FRAME && map3d_info.scale_scope != SLAVE_FRAME)
    if (!map3d_info.lockgeneral && length > 1) {
      loop = priv->dominantsurf;
      length = loop + 1;
    }
  for (; loop < length; loop++) {
    Mesh_Info *mesh = priv->meshes[loop];

    if (!mesh->cont)
      mesh->cont = new Contour_Info(mesh);
    mesh->cont->buildContours();
    for(unsigned i = 0; i<mesh->fidConts.size();i++){      
      mesh->fidConts[i]->buildContours();
    }
    for(unsigned i = 0; i<mesh->fidMaps.size();i++){      
      mesh->fidMaps[i]->buildContours();
    }
//    if(mesh->fidactcont)
//      mesh->fidactcont->buildContours();
//    if(mesh->fidreccont)
//      mesh->fidreccont->buildContours();
//    if(mesh->fidactmapcont)
//      mesh->fidactmapcont->buildContours();
//    if(mesh->fidrecmapcont)
//      mesh->fidrecmapcont->buildContours();
    
    // if legend window exists, update it too
    if (mesh->legendwin != 0) {
      gtk_widget_queue_draw(mesh->legendwin->drawarea);
    }

    // update all windows in the pick stack
    if (mesh->pickstacktop >= 0) {
      for (int loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
        gtk_widget_queue_draw(mesh->pickstack[loop2]->pickwin->drawarea);
      }
    }

    // update the contour dialog for this mesh too
    updateContourDialogValues(mesh);

  }

  gtk_widget_queue_draw(priv->drawarea);
}

void GeomWindowBPress(GtkWidget * widget, GdkEventButton * event, gpointer data, float xn, float yn)
{
  GeomWindow *priv = (GeomWindow *) data;
  Clip_Planes *clip = priv->clip;

  int x = int (priv->width * xn);
  int y = int (priv->height - priv->height * yn);

  priv->curx = (int)event->x_root;
  priv->cury = map3d_info.screenHeight - (int)event->y_root;
  priv->relx = x;
  priv->rely = y;
  
  int length = priv->meshes.size();
  int loop = 0;
  Transforms *tran = 0;
  HVect vNow;
  bool keep_picking = true; // set this to false on a successful pick
    // so we don't pick on another mesh in the same window

  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }
  
  for (loop = 0; loop < length; loop++) {
    tran = priv->meshes[loop]->tran;
    // MIDDLE MOUSE DOWN + SHIFT = start rotate clipping planes
    if (event->button == 2 && COMPARE_MODIFIERS(event->state, GDK_SHIFT_MASK)) {
      if ((clip->back_enabled || clip->front_enabled) &&!clip->lock_with_object) {
        vNow.x = 2.0f * x / priv->width - 1;
        vNow.y = 2.0f * y / priv->height - 1;
        Ball_Mouse(&clip->bd, vNow);
        Ball_BeginDrag(&clip->bd);
        break;
      }
#ifdef ROTATING_LIGHT
      else {
        vNow.x = 2.0f * x / priv->width - 1;
        vNow.y = 2.0f * y / priv->height - 1;
        Ball_Mouse(&priv->light_pos, vNow);
        Ball_BeginDrag(&priv->light_pos);
        break;
      }
#endif
    }
    // LEFT MOUSE DOWN + CTRL = pick
    else if (event->button == 1 && COMPARE_MODIFIERS(event->state, GDK_CONTROL_MASK) &&
		((!map3d_info.lockgeneral && (priv->dominantsurf == loop || length == 1)) || map3d_info.lockgeneral)) {
      if (keep_picking)
		keep_picking = !Pick(priv, loop, x, y);
	}
    // MIDDLE MOUSE DOWN + CTRL = pick for DELETE TRIANGLE
    else if (event->button == 2 && COMPARE_MODIFIERS(event->state, GDK_CONTROL_MASK) &&
		((!map3d_info.lockgeneral && (priv->dominantsurf == loop || length == 1)) || map3d_info.lockgeneral)) {
	  if (keep_picking)
		keep_picking = !Pick(priv, loop, x, y, true);
	}
    // LEFT MOUSE DOWN + ALT = start move window
    else if (event->button == 1 && (COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK))) {
      priv->startx = (int)event->x_root;
      priv->starty = (int)event->y_root;
    }

    /* MIDDLE MOUSE DOWN + ALT = start resize window */
    else if (event->button == 2 && (COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK))) {
      priv->startx = (int)event->x_root;
      priv->starty = (int)event->y_root;
    }

    // LEFT MOUSE DOWN = start rotate
    else if (event->button == 1) {
      vNow.x = 2.0f * x / priv->width - 1.0f;
      vNow.y = 2.0f * y / priv->height - 1.0f;
      Ball_Mouse(&tran->rotate, vNow);
      Ball_BeginDrag(&tran->rotate);
      if (clip->lock_with_object) {
        Ball_Mouse(&clip->bd, vNow);
        Ball_BeginDrag(&clip->bd);
      }
    }
  }
  gtk_widget_queue_draw(widget);
}

void GeomWindowBRelease(GtkWidget * widget, GdkEventButton * event, gpointer data, float /*xn*/, float /*yn*/)
{
  GeomWindow *priv = (GeomWindow *) data;
  Clip_Planes *clip = priv->clip;

  int length = priv->meshes.size();
  int loop = 0;
  Transforms *tran = 0;

  for (loop = 0; loop < length; loop++) {
    tran = priv->meshes[loop]->tran;
    // MIDDLE MOUSE UP + SHIFT = end rotate clipping planes
    if (event->button == 2 && COMPARE_MODIFIERS(event->state, GDK_SHIFT_MASK)) {
      if ((clip->back_enabled || clip->front_enabled) &&!clip->lock_with_object)
        Ball_EndDrag(&clip->bd);
#ifdef ROTATING_LIGHT
      else
        Ball_EndDrag(&priv->light_pos);
#endif
      break;
    }
    // LEFT MOUSE UP = end rotate 
    if (event->button == 1) {
      Ball_EndDrag(&tran->rotate);
      if (clip->lock_with_object)
        Ball_EndDrag(&clip->bd);
    }
  }
  gtk_widget_queue_draw(widget);
}

void GeomWindowButtonPress(GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  GeomWindow *priv = (GeomWindow *) data;
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


  if (event->button == 3){       // right click
    gtk_menu_popup(GTK_MENU(priv->menu), NULL, NULL, NULL, NULL, event->button, event->time);
  }
  else if ((!map3d_info.lockrotate) || COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK)
           || COMPARE_MODIFIERS(event->state, GDK_CONTROL_MASK)) {
    GeomWindowBPress(widget, event, data, (float)event->x / (float)priv->width, (float)event->y / (float)priv->height);
  }
  else {
    Broadcast(MAP3D_MOUSE_BUTTON_PRESS, widget, (GdkEvent *) event, data);
  }

  if (map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_other_events))) {
    SaveScreen();
  } 

}

void GeomWindowButtonRelease(GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  GeomWindow *priv = (GeomWindow *) data;
  
  // don't worry about the selected_group here.  Letting go doesn't change anything for non-selected groups

  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }

  
  if ((!map3d_info.lockrotate) || COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK)
      || COMPARE_MODIFIERS(event->state, GDK_CONTROL_MASK))
    GeomWindowBRelease(widget, event, data, (float)event->x / (float)priv->width,
                       (float)event->y / (float)priv->height);
  else {
    Broadcast(MAP3D_MOUSE_BUTTON_RELEASE, widget, (GdkEvent *) event, data);
  }
}

void GeomWindowMouseMotion(GtkWidget * widget, GdkEventMotion * event, gpointer data)
{
  GeomWindow *priv = (GeomWindow *) data;

  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }
  
  map3d_info.selected_group = (map3d_info.lockrotate == LOCK_GROUP && priv->meshes.size() > 0)
    ? priv->meshes[0]->groupid : -1;

  if ((!map3d_info.lockrotate) || COMPARE_MODIFIERS(event->state, GDK_MOD1_MASK)
      || COMPARE_MODIFIERS(event->state, GDK_CONTROL_MASK))
    GeomWindowHandleMouseMotion(widget, event, data, (float)event->x / (float)priv->width,
                                (float)event->y / (float)priv->height);
  else
    Broadcast(MAP3D_MOUSE_MOTION, widget, (GdkEvent *) event, data);

  if (map3d_info.saving_animations && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->animation_transformation))) {
    SaveScreen();
  } 

}

void GeomWindowHandleMouseMotion(GtkWidget *, GdkEventMotion * event, gpointer data, float xn, float yn)
{
  GeomWindow *priv = (GeomWindow *) data;
  int x = int (priv->width * xn);
  int y = int (priv->height - priv->height * yn);
  int length = priv->meshes.size();
  int loop = 0;
  Transforms *tran = 0;
  Clip_Planes *clip = priv->clip;
  Mesh_Info *mesh = 0;

  if (length > 1 && !map3d_info.lockrotate)
    if (!map3d_info.lockgeneral) {
      loop = priv->dominantsurf;
      length = loop + 1;
    }
    else {
      loop = priv->secondarysurf;
      length = loop + 1;
    }


  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }
  
  for (; loop < length; loop++) {
    mesh = priv->meshes[loop];
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;
    tran = mesh->tran;

    /* CTRL = nothing */
    if (event->state & GDK_CONTROL_MASK) {
      return;
    }

    /* MIDDLE MOUSE DOWN + SHIFT = rotate clipping planes */
    else if (COMPARE_MODIFIERS(event->state, (GDK_SHIFT_MASK | GDK_BUTTON2_MASK))) {
      if ((clip->back_enabled || clip->front_enabled) &&!clip->lock_with_object) {
        HVect vNow;
        vNow.x = 2.0f * x / priv->width - 1.0f;
        vNow.y = 2.0f * y / priv->height - 1.0f;

        Ball_Mouse(&clip->bd, vNow);
        Ball_Update(&clip->bd, 0);
        break;
      }
      else {
        HVect vNow;
        vNow.x = 2.0f * x / priv->width - 1.0f;
        vNow.y = 2.0f * y / priv->height - 1.0f;

#ifdef ROTATING_LIGHT
        Ball_Mouse(&priv->light_pos, vNow);
        Ball_Update(&priv->light_pos, 0);
#endif
        break;
      }
    }
    // LEFT MOUSE DOWN + ALT = draw moving window 
    else if (COMPARE_MODIFIERS(event->state, (GDK_MOD1_MASK | GDK_BUTTON1_MASK))) {
      priv->moveWindow(event);
    }
    // MIDDLE MOUSE DOWN + ALT = draw reshaping window
    else if (COMPARE_MODIFIERS(event->state, (GDK_MOD1_MASK | GDK_BUTTON2_MASK))) {
      priv->sizeWindow(event);
    }
    // LEFT MOUSE DOWN + SHIFT = translate
    else if (COMPARE_MODIFIERS(event->state, (GDK_SHIFT_MASK | GDK_BUTTON1_MASK))) {
      tran->tx += priv->l2norm * ((float)event->x_root - priv->curx) / priv->width * priv->vfov / 29.f;
      tran->ty += priv->l2norm * ((map3d_info.screenHeight - (float)event->y_root) - priv->cury) / priv->height * priv->vfov / 29.f;
      
      //set the globalSurf translation
      mesh->mysurf->parent->SurfList[MAX_SURFS]->translation[0] = tran->tx;
      mesh->mysurf->parent->SurfList[MAX_SURFS]->translation[1] = tran->ty;
      mesh->mysurf->parent->SurfList[MAX_SURFS]->translation[2] = tran->tz;

    }
    // LEFT MOUSE DOWN = rotate
    else if (COMPARE_MODIFIERS(event->state, GDK_BUTTON1_MASK)) {
      HVect vNow;
      vNow.x = 2.0f * x / priv->width - 1.0f;
      vNow.y = 2.0f * y / priv->height - 1.0f;

      Ball_Mouse(&tran->rotate, vNow);
      Ball_Update(&tran->rotate, 0);
      
      //set the globalSurf translation
      mesh->mysurf->parent->SurfList[MAX_SURFS]->rotationQuat = tran->rotate.qNow;
      
      if (clip->lock_with_object) {
        Ball_Mouse(&clip->bd, vNow);
        Ball_Update(&clip->bd, 0);
      }
    }

    /* MIDDLE MOUSE DOWN = zoom (scale) */
    else if (COMPARE_MODIFIERS(event->state, GDK_BUTTON2_MASK)) {
      if (y < priv->rely)
        priv->vfov -= priv->vfov / 24.0f;
      else if (y > priv->rely)
        priv->vfov += priv->vfov / 24.0f;

      if (priv->vfov > 179)
        priv->vfov = 179.0f;
      if (priv->vfov < 0.01)
        priv->vfov = 0.01f;
    }
  }

  priv->curx = (int) event->x_root;
  priv->cury = (map3d_info.screenHeight - (int)event->y_root);
  priv->relx = x;
  priv->rely = y;
  gtk_widget_queue_draw(priv->drawarea);
}

// this function will not work with translation when in edit node pick
// mode except by the mesh that has a node selected for editing
void GeomWindowTransformKeyboard(Mesh_Info * curmesh, GdkEventKey* event)
{
  // to rotate we need to both update the quaternion and the rotation
  // matrix stored in the arcball

  // quaternion for keyboard rotations
  Quat q;
  GeomWindow *priv = curmesh->gpriv;
  float sin_angle = 0;
  int key = event->keyval;

  // if this value is -1 translate the entire mesh.  Else edit a selected mesh point.
  // value gets set when translation is checked below
  int edit_node_num = -1;

  // same, but for landmark points
  int lm_seg = -1;
  int lm_pt = -1;

  bool rotate = false;
  bool translate = false;
  bool edit = false;
  float *mNow = (float *)curmesh->tran->rotate.mNow;
  HMatrix minv;
  float vec[4] = { 0, 0, 0, 0 };
  float trans[4] = { 0, 0, 0, 0 };

  InvertMatrix16((float *)mNow, (float *)minv);

  //Make ctrl+shift = alt for mac compatibility
  if(COMPARE_MODIFIERS(event->state, (GDK_CONTROL_MASK|GDK_SHIFT_MASK))){
    event->state = (event->state & (~(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))| GDK_MOD1_MASK;
  }
  

  // prepare to rotate - 1/3 are scale, 2,4,6,7,8,9 are rotate 
  if ((key != GDK_End && key != GDK_Page_Down && (event->state & GDK_CONTROL_MASK))) {
    rotate = true;
    if (key == GDK_Down || key == GDK_Right || key == GDK_Page_Up) { // CCW rotations, positive angles 
      sin_angle = sin(map3d_info.rotincrement / 2);
      q.w = cos(map3d_info.rotincrement / 2);
    }
    else {
      sin_angle = sin(-map3d_info.rotincrement / 2);
      q.w = cos(-map3d_info.rotincrement / 2);
    }
    q.x = q.y = q.z = 0;
  }
  // check translation - if edit mode is on make sure we have the right mesh,
  //   if landmark edit mode then make sure we have a point selected
  else if ((key != GDK_End && key != GDK_Page_Down && (event->state & GDK_MOD1_MASK))) {
    translate = true;
    if (map3d_info.pickmode == EDIT_NODE_PICK_MODE) {
      if (curmesh->num_selected_pts > 0) {
        edit = true;
        edit_node_num = curmesh->selected_pts[0];
        curmesh->geom->modified = true;
      }
      else
        return;
    }
    else if (map3d_info.pickmode == EDIT_LANDMARK_PICK_MODE) {
      if (curmesh->landmarkdraw.picked_segnum > 0) {
        lm_seg = curmesh->landmarkdraw.picked_segnum;
        lm_pt = curmesh->landmarkdraw.picked_ptnum;
        curmesh->landmarkdraw.modified_lm = true;
      }
      else
        return;
    }
  }
  switch (key) {
  case GDK_End:                    // zoom in
    priv->vfov -= priv->vfov / map3d_info.scaleincrement;
    if (priv->vfov < 0.01)
      priv->vfov = 0.01f;
    break;
  case GDK_Down:
    if (translate) {            // translate down
      vec[1] = -map3d_info.transincrement;
    }
    else                        // rotate CCW about X-axis
    {
      q.x = 1 * sin_angle;
    }
    break;
  case GDK_Page_Down:                    // zoom out
    priv->vfov += priv->vfov / map3d_info.scaleincrement;
    if (priv->vfov > 179)
      priv->vfov = 179.0f;
    break;
  case GDK_Left:                    // translate left
    if (translate) {
      vec[0] = -map3d_info.transincrement;
    }
    else                        // rotate CW around Y-axis
    {
      q.y = 1 * sin_angle;
    }
    break;
  case GDK_Right:                    // translate right
    if (translate) {
      vec[0] = map3d_info.transincrement;
    }
    else                        // rotate CCW around Y-axis
    {
      q.y = 1 * sin_angle;
    }
    break;
  case GDK_Home:
    if (translate) {            // translate to the back
      vec[2] = map3d_info.transincrement;
    }
    else                        // rotate CW around Z-axis
    {
      q.z = 1 * sin_angle;
    }
    break;
  case GDK_Up:
    if (translate) {            // translate up
      vec[1] = map3d_info.transincrement;
    }
    else                        // rotate CW about X-axis
    {
      q.x = 1 * sin_angle;
    }
    break;
  case GDK_Page_Up:
    if (translate) {            // translate to the front
      vec[2] = -map3d_info.transincrement;
    }
    else                        // rotate CCW around Z-axis
    {
      q.z = 1 * sin_angle;
    }
    break;

  }

  if (translate) {
    MultMatrix16x4((float *)minv, vec, trans);
    if (lm_seg >= 0) {          // move a landmark point
      Landmark_Draw draw = curmesh->landmarkdraw;
      LandMarkSeg seg = curmesh->geom->landmarks->segs[draw.picked_segnum];
      seg.pts[lm_pt][0] += trans[0];
      seg.pts[lm_pt][1] += trans[1];
      seg.pts[lm_pt][2] += trans[2];

    }
    else if (edit_node_num >= 0) {  // move a geom point
      float** pts = curmesh->geom->points[curmesh->geom->geom_index];
      pts[edit_node_num][0] += trans[0];
      pts[edit_node_num][1] += trans[1];
      pts[edit_node_num][2] += trans[2];
    }
    else {                      // move the whole mesh
      curmesh->tran->tx += vec[0];
      curmesh->tran->ty += vec[1];
      curmesh->tran->tz += vec[2];
    }


  }

  if (edit){
    curmesh->cont->buildContours();
    for(unsigned i = 0; i<curmesh->fidConts.size();i++){      
      curmesh->fidConts[i]->buildContours();
    }
    for(unsigned i = 0; i<curmesh->fidMaps.size();i++){      
      curmesh->fidMaps[i]->buildContours();
    }
//    if(curmesh->fidactcont)
//      curmesh->fidactcont->buildContours();
//    if(curmesh->fidreccont)
//      curmesh->fidreccont->buildContours();
//    if(curmesh->fidactmapcont)
//      curmesh->fidactmapcont->buildContours();
//    if(curmesh->fidrecmapcont)
//      curmesh->fidrecmapcont->buildContours();
  }
  // finish rotations - save quaternion and matrix to arcball
  //   if we used the arcball's rotation matrix, next time we used the mouse to rotate
  //   it would erase what we just did, since the data is stored in quaternions, and 
  //   they just use quaternions to create rotation matrices
  if (rotate) {
    normalizeQuat(&q);

    // update normal rotation quaternion and matrix
    BallData *bd = &(curmesh->tran->rotate);
    bd->qNow = Qt_Mul(q, bd->qNow);
    Qt_ToMatrix(Qt_Conj(bd->qNow), bd->mNow);
    Ball_EndDrag(bd);           // used to reset mouse

    // update clipping plane quaternion and matrix
    if (priv->clip->lock_with_object) {
      BallData *bd = &(priv->clip->bd);
      bd->qNow = Qt_Mul(q, bd->qNow);
      Qt_ToMatrix(Qt_Conj(bd->qNow), bd->mNow);
      Ball_EndDrag(bd);
    }

  }
}
