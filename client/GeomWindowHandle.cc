/* GeomWindowHandle.cxx */

#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#else
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

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

#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QTime>

extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
extern ColorPicker *cp;
extern SizePicker *sp;
extern vector<Surface_Group> surf_group;
extern int fstep;

#define CHAR_WIDTH .07
#define CHAR_HEIGHT .07

int key_pressed = 0;            //indicate whether (and which) key is held down
int idle_iteration = 0;

// If it is something that needs
// to happen once per mesh or once per window, we will call
// HandleMenu either directly or via Broadcast.
// Otherwise, we will handle it here.
void GeomWindow::MenuEvent(int menu)
{
  // check to see it we handle options that apply globally
  if (MenuGlobalOptions(menu))
    return;

  map3d_info.selected_group = (map3d_info.lockgeneral == LOCK_GROUP && meshes.size() > 0)
    ? meshes[0]->groupid : -1;

  //careful with this - some things we want very specific on when to be
  //broadcasted
  if (map3d_info.lockgeneral || 
    (map3d_info.lockframes && (menu == frame_reset || menu == frame_zero))){
    Broadcast(MAP3D_MENU, menu);
  }
  else{
    HandleMenu(menu);
  }
  if (menu == frame_align || menu == frame_reset)
    Broadcast(MAP3D_REPAINT_ALL);
  if (map3d_info.saving_animations) {
    SaveScreen();
  } 
}


bool GeomWindow::HandleFrameAdvances()
{
  static QTime time;
  static bool firstTime = true;
  if (!key_pressed) {
    return false;
  }

  if (firstTime)
    time.start();
  else if (time.elapsed() < 10)
  {
    return false; // only do 100 frames per second
  }

  time.restart(); // restart timer to return false above
  firstTime = false;

  
  map3d_info.scale_frame_set = 0;

  map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP && meshes.size() > 0)
    ? meshes[0]->groupid : -1;

  if (map3d_info.lockframes) {
    Broadcast(MAP3D_FRAMES, this, 0, key_pressed);
  }
  else {
    // advance the only mesh in this window or only the dominant (if gen lock off too)
    // or only the secondary surf
    int index = dominantsurf != -1 ? dominantsurf : secondarysurf != -1 ? secondarysurf : 0;
    Surf_Data* sd = meshes[index]->data;
    int delta_frames = 0;
    if (key_pressed == Qt::Key_Left) 
      delta_frames = -fstep;
    else if (key_pressed == Qt::Key_Right) 
      delta_frames = fstep;

    sd->FrameAdvance(delta_frames, map3d_info.frame_loop);

    // if advancing in time only affects this surface
    if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
        map3d_info.scale_scope != SLAVE_FRAME) {
      UpdateAndRedraw();
    }
    // it affects at least one other surface...
    else {
      Broadcast(MAP3D_UPDATE);
    }
  }
  if (map3d_info.saving_animations) {
    SaveScreen();
  } 
  return true;
}

void GeomWindow::keyPressEvent(QKeyEvent* event)
{
  int keysym = event->key();
  int key = event->text().toLatin1()[0];  // this one applies SHIFT, keysym does not


  //set up here so both Broadcast branches can access it.  Up/down/left/right won't need it
  int lock;
  if (keysym >= Qt::Key_Home && keysym <= Qt::Key_PageDown && 
      (matchesModifiers(event->modifiers(), Qt::ControlModifier, true) || 
       matchesModifiers(event->modifiers(), Qt::AltModifier, true))) {
    lock = map3d_info.lockrotate;
  }
  else {
    lock = map3d_info.lockgeneral;
  }
  map3d_info.selected_group = (lock == LOCK_GROUP && meshes.size() > 0)
    ? meshes[0]->groupid : -1;

  // okay, the way frame advancing works, so that the window manager won't get bogged down, is
  // handle it directly once (for control), then loop while occasionally checking the event loop
  if ((keysym == Qt::Key_Left || keysym == Qt::Key_Right) &&
      (matchesModifiers(event->modifiers(), Qt::NoModifier, true))) {

    key_pressed = keysym;

    if (idle_iteration == 0) {
      idle_iteration++;
      HandleFrameAdvances(); // call it directly
    }
    else if (idle_iteration == 1)
    {
      // this will happen on the first key repeat.  Subsequent key repeats will be ignored.
      //   It will loop until the user releases the key, and GUI control is given by calling processEvents
      idle_iteration++;
      while (idle_iteration > 0) // event loop can reset on key release
      {
        //for (int i = 0; i < 10; i++)
          HandleFrameAdvances(); // call it directly
        qApp->processEvents();
      }
      //frameAdvanceTimer.start();
    }
    return;
  }

  // don't broadcast these for now
  // TODO - lock with broadcast??? Is there enough probablility that the subseries
  //   of different surfaces will align???
  int index = dominantsurf != -1 ? dominantsurf : secondarysurf != -1 ? secondarysurf : 0;
  Surf_Data* sd = meshes[index]->data;


  if ((keysym == Qt::Key_Left || keysym == Qt::Key_Right) &&
    (matchesModifiers(event->modifiers(), Qt::ShiftModifier, true))) {
    // go forward/back in time one subseries.  Don't repeat
    sd->SubseriesAdvance(keysym == Qt::Key_Right ? 1 : -1);
    UpdateAndRedraw();
  }

  // don't do exact match with these, because you need shift to get the plus on the main keyboard
  else if (keysym == Qt::Key_Plus && matchesModifiers(event->modifiers(), Qt::ControlModifier, false)) {
    // Add the current subseries to the subseries stack.
    sd->StackSubseries();
  }
  else if (keysym == Qt::Key_Minus && matchesModifiers(event->modifiers(), Qt::ControlModifier, true)) {
    // Remove the current subseries to the subseries stack.
    sd->UnstackSubseries();
  }

  if (keysym == Qt::Key_Escape) {
    map3d_quit(masterWindow ? (QWidget*)masterWindow : (QWidget*)this);
  }
  else if (keysym == Qt::Key_Up && matchesModifiers(event->modifiers(), Qt::NoModifier, true)) {
    if (map3d_info.lockgeneral == LOCK_GROUP || (surf_group.size() == 1 && dominantsurf+1 >= (int) meshes.size())) {
      map3d_info.lockgeneral = LOCK_FULL;
    }
    else if (map3d_info.lockgeneral == LOCK_OFF && surf_group.size() > 1 && dominantsurf+1 >= (int) meshes.size()) {
      map3d_info.lockgeneral = LOCK_GROUP;
    }

    if (map3d_info.lockgeneral != LOCK_OFF) {
      for (unsigned i = 0; i < numGeomWindows(); i++) {
        GeomWindow* gwin = GetGeomWindow(i);

        gwin->dominantsurf = -1;
      }      
    }
    else {
      dominantsurf++;
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
    Broadcast(MAP3D_REPAINT_ALL);
  }
  else if (keysym == Qt::Key_Down && matchesModifiers(event->modifiers(), Qt::NoModifier, true)) {
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
    else if (dominantsurf > 0) {
      dominantsurf--;
    }
    Broadcast(MAP3D_REPAINT_ALL);
  }
  else if ((!map3d_info.lockgeneral && key != 'r') || (key == 'f') ||
    (key == 't') || (key == '(') || (key == ')') || (key == 'w') ) {
    HandleKeyPress(event);
    Broadcast(MAP3D_REPAINT_ALL);
  }
  else
    Broadcast(MAP3D_KEY_PRESS, this, event);

  if (map3d_info.saving_animations) {
    SaveScreen();
  }
}

void GeomWindow::HandleKeyPress(QKeyEvent* event)
{
  int loop = 0, loop2 = 0;
  int length = meshes.size();

  Mesh_Info *mesh = 0;
  LegendWindow *lpriv = 0;
  int menu_data;

  if (length > 1 && !map3d_info.lockgeneral) {
    loop = dominantsurf;
    length = loop + 1;
  }

  int keysym = event->key();
  int key = event->text().toLatin1()[0];  // this one applies SHIFT, keysym does not

  bool first = true;
  for (; loop < length; loop++)
  {
    mesh = meshes[loop];
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;

    // for lining up meshes with cycling things like shading
    Mesh_Info* tmpmesh = mesh;
    if (map3d_info.lockgeneral)
      tmpmesh = getFirstMesh();

    // call keyboard transformations function - the keys between home and end
    // should be all the keys on the keypad
    if (keysym >= Qt::Key_Home && keysym <= Qt::Key_PageDown && 
        (matchesModifiers(event->modifiers(), Qt::ControlModifier, true) || 
         matchesModifiers(event->modifiers(), Qt::AltModifier, true))) {
      TransformKeyboard(mesh, event);
      continue;
    }

    // with the toggle controls there is an interesting phenomenon such that if you
    // turn the general lock off, change a few things, and turn it back on, when you 
    // change things, they change in and out and never line up.  To avoid that, we want
    // to 
    switch (key) {

      /* l = toggle lighting */
    case 'l':
      MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, lighting);
      break;

      /* L - reLoad geometry and data */

    case 'L':
      menu_data = mesh_both_reload;
      HandleMenu(menu_data);
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
        menu_data = tmpmesh==mesh?mesh_render_points:mesh_render_none;
        HandleMenu(menu_data);
        break;
      case RENDER_MESH_PTS:
        menu_data = tmpmesh==mesh?mesh_render_elements:mesh_render_points;
        HandleMenu(menu_data);
        break;
      case RENDER_MESH_ELTS:
        menu_data = tmpmesh==mesh?mesh_render_connectivity:mesh_render_elements;
        HandleMenu(menu_data);
        break;
      case RENDER_MESH_CONN:
      case RENDER_MESH_ELTS_CONN:
      case RENDER_MESH_PTS_CONN:
      case RENDER_MESH_NONDATA_ELTS:
        menu_data = tmpmesh==mesh?mesh_render_none:mesh_render_connectivity;
        HandleMenu(menu_data);
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
        menu_data = tmpmesh==mesh?mesh_render_connectivity:mesh_render_none;
        HandleMenu(menu_data);
        break;
      case RENDER_MESH_PTS:
        menu_data = tmpmesh==mesh?mesh_render_none:mesh_render_points;
        HandleMenu(menu_data);
        break;
      case RENDER_MESH_ELTS:
        menu_data = tmpmesh==mesh?mesh_render_points:mesh_render_elements;
        HandleMenu(menu_data);
        break;
      case RENDER_MESH_CONN:
        menu_data = tmpmesh==mesh?mesh_render_elements:mesh_render_connectivity;
        HandleMenu(menu_data);
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
      dominantsurf = -1;
      mesh->tran->reset();
      showinfotext = 1;
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
        menu_data = tmpmesh==mesh?surface_render_banded:surface_render_none;
        HandleMenu(menu_data);
        break;
      case SHADE_FLAT:
        menu_data = tmpmesh==mesh?surface_render_none:surface_render_flat;
        HandleMenu(menu_data);
        break;
      case SHADE_GOURAUD:
        menu_data = tmpmesh==mesh?surface_render_none:surface_render_gouraud;
        HandleMenu(menu_data);
        break;
      case SHADE_BANDED:
        menu_data = tmpmesh==mesh?surface_render_gouraud:surface_render_banded;
        HandleMenu(menu_data);
        break;
      }
      
	  loop = length; // this is so that the change doesn't happen n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in HandleMenu
      break;

      /* s = rotate colormapped surface to next type or off
             go the opposite direction as S (see comments for S as to the mess)*/
    case 's':
      switch (tmpmesh->shadingmodel) {
      case SHADE_NONE:
        menu_data = tmpmesh==mesh?surface_render_gouraud:surface_render_none;
        HandleMenu(menu_data);
        break;
      case SHADE_FLAT:
        menu_data = tmpmesh==mesh?surface_render_gouraud:surface_render_none;
        HandleMenu(menu_data);
        break;
      case SHADE_GOURAUD:
        menu_data = tmpmesh==mesh?surface_render_banded:surface_render_gouraud;
        HandleMenu(menu_data);
        break;
      case SHADE_BANDED:
        menu_data = tmpmesh==mesh?surface_render_none:surface_render_banded;
        HandleMenu(menu_data);
        break;
        }
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in HandleMenu
      break;

      // a - cycle colormaps
    case 'a':
      // similar mess to S for synchronizing cycling
      if (tmpmesh->cmap == &Rainbow) {
        menu_data = tmpmesh==mesh?surface_color_jet:surface_color_rainbow;
        HandleMenu(menu_data);
      }
      else if (tmpmesh->cmap == &Green2Red) {
        menu_data = tmpmesh==mesh?surface_color_rainbow:surface_color_red2green;
        HandleMenu(menu_data);
      }
      else if (tmpmesh->cmap == &Grayscale) {
        menu_data = tmpmesh==mesh?surface_color_red2green:surface_color_grayscale;
        HandleMenu(menu_data);
      }
      else if (tmpmesh->cmap == &Jet) {
        menu_data = tmpmesh==mesh?surface_color_grayscale:surface_color_jet;
        HandleMenu(menu_data);
      }
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in HandleMenu
      break;
      // A - cycle colormaps - go opposite direction as a
    case 'A':
      if (tmpmesh->cmap == &Rainbow) {
        menu_data = tmpmesh==mesh?surface_color_red2green:surface_color_rainbow;
        HandleMenu(menu_data);
      }
      else if (tmpmesh->cmap == &Green2Red) {
        menu_data = tmpmesh==mesh?surface_color_grayscale:surface_color_red2green;
        HandleMenu(menu_data);
      }
      else if (tmpmesh->cmap == &Grayscale) {
        menu_data = tmpmesh==mesh?surface_color_jet:surface_color_grayscale;
        HandleMenu(menu_data);
      }
      else if (tmpmesh->cmap == &Jet) {
        menu_data = tmpmesh==mesh?surface_color_rainbow:surface_color_jet;
        HandleMenu(menu_data);
      }
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in HandleMenu
      break;

      /* i = toggle info display */
    case 'i':
    case 'I':
      menu_data = surface_color_invert;
      HandleMenu(menu_data);
	  loop = length; // this is so that the change happens n*n times (when 
	    // the general lock is on) - n times in this loop and n times
	    // in HandleMenu
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
      lpriv->update();
    }
    // update all windows in the pick stack
    if (mesh->pickstacktop >= 0) {
      for (loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
        mesh->pickstack[loop2]->pickwin->update();
      }
    }
    first = false;
  }

  //once-per-window items instead of once per mesh
  switch (keysym) {
  case Qt::Key_Left:
  case Qt::Key_Right:
    // left or right was pressed
    break;
    /* enable/disable clipping planes */
  case '<':
    menu_data = clip_front;
    HandleMenu(menu_data);
    break;
  case '>':
    menu_data = clip_back;
    HandleMenu(menu_data);
    break;

    /* makes the clipping planes rotate with the mesh. */
  case ',':
    menu_data = clip_with_object;
    HandleMenu(menu_data);
    //clip->lock_with_object = !clip->lock_with_object;
    break;

    /* makes the clipping planes translate together. */
  case '.':
    menu_data = clip_together;
    HandleMenu(menu_data);
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

  case 'P':
    menu_data = fid_map_shade_toggle;//pick_info_display;
    HandleMenu(menu_data);
	loop = length; // this is so that the change happens n*n times (when 
	// the general lock is on) - n times in this loop and n times
	// in HandleMenu
    break;

    /* lock/unlock frames */
  case 'F':
    menu_data = frame_lock;
    MenuEvent(menu_data);
	loop = length; // this is so that the change happens n*n times (when 
	// the general lock is on) - n times in this loop and n times
	// in HandleMenu
    break;
  case 'T':
    menu_data = trans_lock;
    MenuEvent(menu_data);
	loop = length; // this is so that the change happens n*n times (when 
	// the general lock is on) - n times in this loop and n times
	// in HandleMenu
    break;
  case '(':
    if (secondarysurf != -1 && secondarysurf + 1 < (int)meshes.size())
      secondarysurf++;
    break;
  case ')':
    if (secondarysurf != -1 && secondarysurf - 1 >= 0)
      secondarysurf--;
    break;
  case 'W':
    SaveScreen();
    break;
  }
  update();
}

void GeomWindow::keyReleaseEvent(QKeyEvent* event)
{

  // stop auto-frame firing
  if (!event->isAutoRepeat())
    key_pressed = idle_iteration = 0;

  // turn off frame advance firing
  frameAdvanceTimer.stop();

  if (!map3d_info.lockgeneral)
    HandleKeyRelease(event);
  else
    Broadcast(MAP3D_KEY_RELEASE, this, event);
}

void GeomWindow::HandleKeyRelease(QKeyEvent*)
{
}

void GeomWindow::UpdateAndRedraw() 
{
  int loop = 0;
  int length = meshes.size();
  if (length > 0 && !map3d_info.lockframes && map3d_info.scale_scope != GLOBAL_FRAME &&
      map3d_info.scale_scope != GROUP_FRAME && map3d_info.scale_scope != SLAVE_FRAME)
    if (!map3d_info.lockgeneral && length > 1) {
      loop = dominantsurf;
      length = loop + 1;
    }
  for (; loop < length; loop++) {
    Mesh_Info *mesh = meshes[loop];

    //if animated geom, update the frame here
    if (mesh->data)
      mesh->geom->UpdateTimestep(mesh->data);

    if (!mesh->cont)
      mesh->cont = new Contour_Info(mesh);
    mesh->cont->buildContours();
    for(unsigned i = 0; i<mesh->fidConts.size();i++){      
      mesh->fidConts[i]->buildContours();
      mesh->fidMaps[i]->buildContours();
    }
    // if legend window exists, update it too
    if (mesh->legendwin != 0) {
      mesh->legendwin->update();
    }

    // update all windows in the pick stack
    if (mesh->pickstacktop >= 0) {
      for (int loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
        mesh->pickstack[loop2]->pickwin->update();
      }
    }

    // update the contour dialog for this mesh too
    // FIX updateContourDialogValues(mesh);

  }
  update();
}

void GeomWindow::mousePressEvent(QMouseEvent * event)
{
  setMoveCoordinates(event);
  int button = mouseButtonOverride(event);
  
  if (button == Qt::RightButton){       // right click
    int menu_data = OpenMenu(mapToGlobal(event->pos()));
    if (menu_data >= 0)
      MenuEvent(menu_data);
  }
  else if ((!map3d_info.lockrotate) || matchesModifiers(event->modifiers(), Qt::AltModifier, true) ||
           matchesModifiers(event->modifiers(), Qt::ControlModifier, true)) {
    HandleButtonPress(event, (float)event->x() / (float)width(), (float)event->y() / (float)height());
  }
  else {
    Broadcast(MAP3D_MOUSE_BUTTON_PRESS, this, (QEvent *) event);
  }

  if (map3d_info.saving_animations) {
    SaveScreen();
  } 

}

void GeomWindow::mouseReleaseEvent(QMouseEvent * event)
{
  // don't worry about the selected_group here.  Letting go doesn't change anything for non-selected groups
  
  if ((!map3d_info.lockrotate) || matchesModifiers(event->modifiers(), Qt::AltModifier, true) ||
      matchesModifiers(event->modifiers(), Qt::ControlModifier, true))
    HandleButtonRelease(event, (float)event->x() / (float)width(),
                       (float)event->y() / (float)height());
  else {
    Broadcast(MAP3D_MOUSE_BUTTON_RELEASE, this, (QEvent *) event);
  }
}

void GeomWindow::mouseMoveEvent(QMouseEvent* event)
{
  map3d_info.selected_group = (map3d_info.lockrotate == LOCK_GROUP && meshes.size() > 0)
    ? meshes[0]->groupid : -1;

  if ((!map3d_info.lockrotate) || matchesModifiers(event->modifiers(), Qt::AltModifier, true) ||
      matchesModifiers(event->modifiers(), Qt::ControlModifier, true))
    HandleMouseMotion(event, (float)event->x() / (float)width(), (float)event->y() / (float)height());
  else
    Broadcast(MAP3D_MOUSE_MOTION, this, event);

  if (map3d_info.saving_animations) {
    SaveScreen();
  } 

}

void GeomWindow::HandleButtonPress(QMouseEvent * event, float xn, float yn)
{
  int button = mouseButtonOverride(event);
  int newModifiers = button == event->buttons() ? event->modifiers() : Qt::NoModifier;
  int x = int (width() * xn);
  int y = int (height() - height() * yn);

  last_xn = xn;
  last_yn = yn;
  
  int length = meshes.size();
  int loop = 0;
  Transforms *tran = 0;
  HVect vNow;
  bool keep_picking = true; // set this to false on a successful pick
    // so we don't pick on another mesh in the same window

  for (loop = 0; loop < length; loop++) {
    tran = meshes[loop]->tran;
    // MIDDLE MOUSE DOWN + SHIFT = start rotate clipping planes
    if (button == Qt::MidButton && matchesModifiers(newModifiers, Qt::ShiftModifier, true)) {
      if ((clip->back_enabled || clip->front_enabled) &&!clip->lock_with_object) {
        vNow.x = 2.0f * x / width() - 1;
        vNow.y = 2.0f * y / height() - 1;
        Ball_Mouse(&clip->bd, vNow);
        Ball_BeginDrag(&clip->bd);
        break;
      }
#ifdef ROTATING_LIGHT
      else {
        vNow.x = 2.0f * x / width() - 1;
        vNow.y = 2.0f * y / height() - 1;
        Ball_Mouse(&light_pos, vNow);
        Ball_BeginDrag(&light_pos);
        break;
      }
#endif
    }
    // LEFT MOUSE DOWN + CTRL = pick (not ctrl-shift - use that for mac for alt)
    else if (button == Qt::LeftButton && matchesModifiers(newModifiers, Qt::ControlModifier, true) &&
		((!map3d_info.lockgeneral && (dominantsurf == loop || length == 1)) || map3d_info.lockgeneral)) {
      if (keep_picking)
		keep_picking = !Pick(loop, x, y);
  	}
    // MIDDLE MOUSE DOWN + CTRL = pick for DELETE TRIANGLE
    else if (button == Qt::RightButton && matchesModifiers(newModifiers, Qt::ControlModifier, true) &&
		((!map3d_info.lockgeneral && (dominantsurf == loop || length == 1)) || map3d_info.lockgeneral)) {
	  if (keep_picking)
		keep_picking = !Pick(loop, x, y, true);
	  }

    // LEFT MOUSE DOWN = start rotate
    else if (button == Qt::LeftButton) {
      vNow.x = 2.0f * x / width() - 1.0f;
      vNow.y = 2.0f * y / height() - 1.0f;
      Ball_Mouse(&tran->rotate, vNow);
      Ball_BeginDrag(&tran->rotate);
      if (clip->lock_with_object) {
        Ball_Mouse(&clip->bd, vNow);
        Ball_BeginDrag(&clip->bd);
      }
    }
  }
  update();
}

void GeomWindow::HandleButtonRelease(QMouseEvent * event, float /*xn*/, float /*yn*/)
{
  int button = mouseButtonOverride(event);
  int newModifiers = button == event->button() ? event->modifiers() : Qt::NoModifier;
  
  int length = meshes.size();
  int loop = 0;
  Transforms *tran = 0;

  for (loop = 0; loop < length; loop++) {
    tran = meshes[loop]->tran;
    // MIDDLE MOUSE UP + SHIFT = end rotate clipping planes
    if (button == Qt::MidButton && matchesModifiers(newModifiers, Qt::ShiftModifier, true)) {
      if ((clip->back_enabled || clip->front_enabled) &&!clip->lock_with_object)
        Ball_EndDrag(&clip->bd);
#ifdef ROTATING_LIGHT
      else
        Ball_EndDrag(&light_pos);
#endif
      break;
    }
    // LEFT MOUSE UP = end rotate 
    if (button == Qt::LeftButton) {
      Ball_EndDrag(&tran->rotate);
      if (clip->lock_with_object)
        Ball_EndDrag(&clip->bd);
    }
  }
  update();
}

void GeomWindow::HandleMouseMotion(QMouseEvent * event, float xn, float yn)
{
  int button = mouseButtonOverride(event);
  int newModifiers = button == event->buttons() ? event->modifiers() : Qt::NoModifier;
  int x = int (width() * xn);
  int y = int (height() - height() * yn);

  int length = meshes.size();
  int loop = 0;
  Transforms *tran = 0;
  Mesh_Info *mesh = 0;

  if (length > 1 && !map3d_info.lockrotate)
    if (!map3d_info.lockgeneral) {
      loop = dominantsurf;
      length = loop + 1;
    }
    else {
      loop = secondarysurf;
      length = loop + 1;
    }
  
  for (; loop < length; loop++) {
    mesh = meshes[loop];
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;
    tran = mesh->tran;

    /* CTRL = nothing */
    if (matchesModifiers(newModifiers, Qt::ControlModifier, true)) {
      return;
    }

    /* MIDDLE MOUSE DOWN + SHIFT = rotate clipping planes */
    else if (button == Qt::MidButton && matchesModifiers(newModifiers, Qt::ShiftModifier, true)) {
      if ((clip->back_enabled || clip->front_enabled) &&!clip->lock_with_object) {
        HVect vNow;
        vNow.x = 2.0f * x / width() - 1.0f;
        vNow.y = 2.0f * y / height() - 1.0f;

        Ball_Mouse(&clip->bd, vNow);
        Ball_Update(&clip->bd, 0);
        break;
      }
      else {
        HVect vNow;
        vNow.x = 2.0f * x / width() - 1.0f;
        vNow.y = 2.0f * y / height() - 1.0f;

#ifdef ROTATING_LIGHT
        Ball_Mouse(&light_pos, vNow);
        Ball_Update(&light_pos, 0);
#endif
        break;
      }
    }
    // LEFT MOUSE DOWN + ALT = draw moving window 
    else if (button == Qt::LeftButton && matchesModifiers(newModifiers, Qt::AltModifier, true)) {
      moveEvent(event);
    }
    // MIDDLE MOUSE DOWN + ALT = draw reshaping window
    else if (button == Qt::MidButton && matchesModifiers(newModifiers, Qt::AltModifier, true)) {
      sizeEvent(event);
    }
    // LEFT MOUSE DOWN + SHIFT = translate
    else if (button == Qt::LeftButton && matchesModifiers(newModifiers, Qt::ShiftModifier, true)) {
      tran->tx += l2norm * (xn - last_xn)  * vfov / 29.f;
      tran->ty += l2norm * (last_yn - yn)  * vfov / 29.f;
      
      //set the globalSurf translation
      mesh->mysurf->parent->SurfList[MAX_SURFS]->translation[0] = tran->tx;
      mesh->mysurf->parent->SurfList[MAX_SURFS]->translation[1] = tran->ty;
      mesh->mysurf->parent->SurfList[MAX_SURFS]->translation[2] = tran->tz;

    }
    // LEFT MOUSE DOWN = rotate
    else if (button == Qt::LeftButton) {
      HVect vNow;
      vNow.x = 2.0f * x / width() - 1.0f;
      vNow.y = 2.0f * y / height() - 1.0f;

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
    else if (button == Qt::MidButton) {
      if (yn < last_yn)
        vfov -= vfov / 24.0f;
      else if (yn > last_yn)
        vfov += vfov / 24.0f;

      if (vfov > 179)
        vfov = 179.0f;
      if (vfov < 0.01)
        vfov = 0.01f;
    }
  }

  last_xn = xn;
  last_yn = yn;

  update();
}

// this function will not work with translation when in edit node pick
// mode except by the mesh that has a node selected for editing
void GeomWindow::TransformKeyboard(Mesh_Info * curmesh, QKeyEvent* event)
{
  // to rotate we need to both update the quaternion and the rotation
  // matrix stored in the arcball

  // quaternion for keyboard rotations
  Quat q;
  GeomWindow *priv = curmesh->gpriv;
  float sin_angle = 0;
  int key = event->key();

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
  
  // prepare to rotate - CTRL + 1/3 are scale, CTRL + 2,4,6,7,8,9 are rotate 
  if (key != Qt::Key_End && key != Qt::Key_PageDown && (matchesModifiers(event->modifiers(), Qt::ControlModifier, true))) {
    rotate = true;
    if (key == Qt::Key_Down || key == Qt::Key_Right || key == Qt::Key_PageUp) { // CCW rotations, positive angles 
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
  else if (key != Qt::Key_End && key != Qt::Key_PageDown && (matchesModifiers(event->modifiers(), Qt::AltModifier, true))) {
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
  case Qt::Key_End:                    // zoom in
    vfov -= vfov / map3d_info.scaleincrement;
    if (vfov < 0.01)
      vfov = 0.01f;
    break;
  case Qt::Key_Down:
    if (translate) {            // translate down
      vec[1] = -map3d_info.transincrement;
    }
    else                        // rotate CCW about X-axis
    {
      q.x = 1 * sin_angle;
    }
    break;
  case Qt::Key_PageDown:                    // zoom out
    vfov += vfov / map3d_info.scaleincrement;
    if (vfov > 179)
      vfov = 179.0f;
    break;
  case Qt::Key_Left:                    // translate left
    if (translate) {
      vec[0] = -map3d_info.transincrement;
    }
    else                        // rotate CW around Y-axis
    {
      q.y = 1 * sin_angle;
    }
    break;
  case Qt::Key_Right:                    // translate right
    if (translate) {
      vec[0] = map3d_info.transincrement;
    }
    else                        // rotate CCW around Y-axis
    {
      q.y = 1 * sin_angle;
    }
    break;
  case Qt::Key_Home:
    if (translate) {            // translate to the back
      vec[2] = map3d_info.transincrement;
    }
    else                        // rotate CW around Z-axis
    {
      q.z = 1 * sin_angle;
    }
    break;
  case Qt::Key_Up:
    if (translate) {            // translate up
      vec[1] = map3d_info.transincrement;
    }
    else                        // rotate CW about X-axis
    {
      q.x = 1 * sin_angle;
    }
    break;
  case Qt::Key_PageUp:
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
      curmesh->fidMaps[i]->buildContours();
    }
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
    if (clip->lock_with_object) {
      BallData *bd = &(clip->bd);
      bd->qNow = Qt_Mul(q, bd->qNow);
      Qt_ToMatrix(Qt_Conj(bd->qNow), bd->mNow);
      Ball_EndDrag(bd);
    }

  }
}
