/* GeomWindowMenu.cxx */

#ifdef _WIN32
#  include <windows.h>
#  pragma warning(disable:4505)
#  undef TRACE
#endif

#include "GeomWindowMenu.h"

#include "Contour_Info.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "PickWindow.h"
#include "WindowManager.h"
#include "dialogs.h"
#include "ProcessCommandLineOptions.h"
#include "FileDialog.h"
#include "ContourDialog.h"
#include "FidDialog.h"
#include "ScaleDialog.h"
#include "ImageControlDialog.h"

#include "colormaps.h"
#include "eventdata.h"
#include "pickinfo.h"
#include "reportstate.h"
#include "scalesubs.h"
#include "savescreen.h"
#include "savestate.h"

#include <QMenu>
#include <QInputDialog>
#include <stdlib.h>

#include <math.h>
#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#if defined (Q_OS_LINUX) || defined(Q_OS_MAC)
#  include <unistd.h>
#endif

extern Map3d_Info map3d_info;
extern vector<Surface_Group> surf_group;
extern MainWindow* masterWindow;
extern int fstep;

void GeomWindow::HandleMenu(int menu_data)
{
  LegendWindow *lpriv = 0;
  PickWindow *ppriv = 0;
  int length = meshes.size();
  int loop = 0;
  int loop2;
  int numconts = 50;
  Mesh_Info *mesh = 0;
  
  /* Do two passes - 
    one for those that apply depending on the general lock status, 
    one for those that apply only once per window (clipping stuff) 
    PUT GLOBAL THINGS IN GeomWindowMenuGlobalOptions */
  
  //If frame command work off of frame lock instead of general lock.
  if(length > 1 && !map3d_info.lockframes && (menu_data == frame_reset || menu_data == frame_zero)){
    loop = secondarysurf;
    length = loop +1;
  }
  else if (length > 1 && !map3d_info.lockgeneral) {
    loop = dominantsurf;
    length = loop + 1;
  }
  
  
  for (; loop < length; loop++) {
    mesh = meshes[loop];
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;
    if (mesh->legendwin != 0)
      lpriv = mesh->legendwin;
    else
      lpriv = 0;
    
    //switch for every mesh in the window, based on general lock status
    switch ((menu_data)) {
      case surface_color_rainbow:
      {
        mesh->cmap = &Rainbow;
        if (lpriv) {
          lpriv->update();
        }
        break;
      }
      case surface_color_red2green:
      {
        mesh->cmap = &Green2Red;
        if (lpriv) {
          lpriv->update();
        }
        break;
      }
      case surface_color_grayscale:
      {
        mesh->cmap = &Grayscale;
        if (lpriv) {
          lpriv->update();
        }
        break;
      }
      case surface_color_jet:
      {
        mesh->cmap = &Jet;
        if (lpriv) {
          lpriv->update();
        }
        break;
      }
      case surface_color_invert:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, invert);
        
        if (lpriv) {
          lpriv->update();
        }
          break;
      case surface_render_none:
        mesh->shadingmodel = SHADE_NONE;
        if (mesh->legendwin != 0)
          lpriv->update();
          break;
      case surface_render_flat:
        mesh->shadingmodel = SHADE_FLAT;
        
        if (mesh->legendwin != 0)
          lpriv->update();
        break;
      case surface_render_gouraud:
        mesh->shadingmodel = SHADE_GOURAUD;
        if (mesh->legendwin != 0)
          lpriv->update();
        break;
      case surface_render_banded:
        mesh->shadingmodel = SHADE_BANDED;
        
        if (mesh->legendwin != 0)
          lpriv->update();
        break;
      case fid_map_shade_toggle:
        if (mesh->fidMaps.size() == 0)
          break;
        mesh->shadefids = !mesh->shadefids;
        break;
      case fid_draw_fid:
        mesh->drawfids = !mesh->drawfids;
        if (!mesh->drawfids)
          mesh->shadefids = false;
        break;
      case scaling_group_one:
        updateGroup(mesh, 0);
        break;
      case scaling_group_two:
        updateGroup(mesh, 1);
        break;
      case scaling_group_three:
        updateGroup(mesh, 2);
        break;
      case scaling_group_four:
        updateGroup(mesh, 3);
        break;
      case mesh_files_dialog:
        filesDialogCreate();
        break;
      case mesh_size:
        PickSize(&mesh->meshsize, 10, "Mesh Line and Point Size");
        break;
      case mesh_render_none:
        mesh->drawmesh = RENDER_MESH_NONE;
        break;
      case mesh_render_points:
        mesh->drawmesh = RENDER_MESH_PTS;
        break;
      case mesh_render_elements:
        mesh->drawmesh = RENDER_MESH_ELTS;
        break;
      case mesh_render_connectivity:
        mesh->drawmesh = RENDER_MESH_CONN;
        break;
      case mesh_render_elements_connectivity:
        mesh->drawmesh = RENDER_MESH_ELTS_CONN;
        break;
      case mesh_render_points_connectivity:
        mesh->drawmesh = RENDER_MESH_PTS_CONN;
        break;
      case mesh_render_nondata_elements: 
        mesh->drawmesh = RENDER_MESH_NONDATA_ELTS;
        break;
      case mesh_color:
        PickColor(mesh->meshcolor);
        break;
      case secondary_color:
        PickColor(mesh->secondarycolor);
        break;
      case mesh_toggle:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, drawmesh);
        break;
      case mesh_geom_reload:
        if (loop == 0 || meshes[loop-1]->mysurf != mesh->mysurf) {
          FindAndReadGeom(mesh->mysurf, findMeshesFromSameInput(mesh), RELOAD_GEOM);
          UpdateAndRedraw();
        }
        break;
      case mesh_data_reload:
        if (loop == 0 || meshes[loop-1]->mysurf != mesh->mysurf) {
          FindAndReadGeom(mesh->mysurf, findMeshesFromSameInput(mesh), RELOAD_DATA);
          UpdateAndRedraw();
        }
        break;
      case mesh_both_reload:
        if (loop == 0 || meshes[loop-1]->mysurf != mesh->mysurf) {
          FindAndReadGeom(mesh->mysurf, findMeshesFromSameInput(mesh), RELOAD_BOTH);
          UpdateAndRedraw();
        }
        break;
      case contour_number_5:
        numconts -= 5;
      case contour_number_10:
        numconts -= 5;
      case contour_number_15:
        numconts -= 5;
      case contour_number_20:
        numconts -= 5;
      case contour_number_25:
        numconts -= 5;
      case contour_number_30:
        numconts -= 5;
      case contour_number_35:
        numconts -= 5;
      case contour_number_40:
        numconts -= 5;
      case contour_number_45:
        numconts -= 5;
      case contour_number_50:
        /* get rid of old contours */
        //mesh->use_spacing = false;
        mesh->drawcont = 1;
        if (mesh->data) {
          mesh->data->numconts = numconts;
          /* generate contours/bands */
          
          mesh->cont->buildContours();
          
          // if legend window matches contours, change it.
          if (mesh->legendwin != 0) {
            lpriv = mesh->legendwin;
            if (lpriv->matchContours)
              lpriv->nticks = numconts + 2;
            
            lpriv->update();
          }
        }
          numconts = 50;
        break;
      case contour_spacing_user:
        /* get rid of old contours */
        
        /* generate contours/bands */
        //mesh->use_spacing = true;
        numconts = mesh->cont->buildContours();
        
        // if legend window matches contours, change it.
        if (mesh->legendwin != 0) {
          lpriv = mesh->legendwin;
          if (lpriv->matchContours)
            lpriv->nticks = numconts + 2;
          
          lpriv->update();
        }
          numconts = 50;
        break;
      case contour_style_dashed:
        mesh->negcontdashed = 1;
        mesh->drawcont = 1;
        break;
      case contour_style_solid:
        mesh->negcontdashed = 0;
        mesh->drawcont = 1;
        break;
      case contour_size:
        PickSize(&mesh->contsize, 10, "Contour Size");
        break;
      case contour_toggle:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, drawcont);
        break;
      case mark_all_sphere:
        mesh->mark_all_sphere = !mesh->mark_all_sphere;
        mesh->qshowpnts = 1;
        break;
      case mark_all_sphere_value:
        mesh->mark_all_sphere_value = !mesh->mark_all_sphere_value;
        if (mesh->mark_all_sphere_value) {
          // turn on mark_all_sphere too
          mesh->mark_all_sphere = true;
          mesh->qshowpnts = 1;
        }
          mesh->qshowpnts = 1;
        break;
      case mark_all_node:
        if (mesh->mark_all_number == 1) {
          // toggle the value
          mesh->mark_all_number = 0;
        }
        else {
          mesh->mark_all_number = 1;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_all_channel:
        if (mesh->mark_all_number == 2) {
          // toggle the value
          mesh->mark_all_number = 0;
        }
        else {
          mesh->mark_all_number = 2;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_all_value:
        if (mesh->mark_all_number == 3) {
          // toggle the value
          mesh->mark_all_number = 0;
        }
        else {
          mesh->mark_all_number = 3;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_all_fid:
        if (mesh->mark_all_number == 4) {
          // toggle the value
          mesh->mark_all_number = 0;
        }
        else {
          mesh->mark_all_number = 4;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_all_clear:
        mesh->mark_all_number = 0;
        mesh->mark_all_sphere = 0;
        break;
      case mark_all_color:
        PickColor(mesh->mark_all_color);
        break;
      case mark_all_size:
        PickSize(&mesh->mark_all_size, 10, "All Node Marks Size");
        break;
      case mark_extrema_sphere:
        mesh->mark_extrema_sphere = !mesh->mark_extrema_sphere;
        mesh->qshowpnts = 1;
        break;
      case mark_extrema_node:
        if (mesh->mark_extrema_number == 1) {
          // toggle the value
          mesh->mark_extrema_number = 0;
        }
        else {
          mesh->mark_extrema_number = 1;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_extrema_channel:
        if (mesh->mark_extrema_number == 2) {
          // toggle the value
          mesh->mark_extrema_number = 0;
        }
        else {
          mesh->mark_extrema_number = 2;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_extrema_value:
        if (mesh->mark_extrema_number == 3) {
          // toggle the value
          mesh->mark_extrema_number = 0;
        }
        else {
          mesh->mark_extrema_number = 3;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_extrema_size:
        PickSize(&mesh->mark_extrema_size, 10, "Extrema Node Mark Size");
        mesh->qshowpnts = 1;
        break;
      case mark_extrema_clear:
        mesh->mark_extrema_number = 0;
        mesh->mark_extrema_sphere = 0;
        break;
      case mark_ts_sphere:
        mesh->mark_ts_sphere = !mesh->mark_ts_sphere;
        mesh->qshowpnts = 1;
        break;
      case mark_ts_node:
        if (mesh->mark_ts_number == 1) {
          // toggle the value
          mesh->mark_ts_number = 0;
        }
        else {
          mesh->mark_ts_number = 1;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_ts_channel:
        if (mesh->mark_ts_number == 2) {
          // toggle the value
          mesh->mark_ts_number = 0;
        }
        else {
          mesh->mark_ts_number = 2;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_ts_value:
        if (mesh->mark_ts_number == 3) {
          // toggle the value
          mesh->mark_ts_number = 0;
        }
        else {
          mesh->mark_ts_number = 3;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_ts_color:
        PickColor(mesh->mark_ts_color);
        break;
      case mark_ts_size:
        PickSize(&mesh->mark_ts_size, 10, "Time Series Node Mark Size");
        break;
      case mark_ts_clear:
        mesh->mark_ts_number = 0;
        mesh->mark_ts_sphere = 0;
        break;
      case mark_lead_sphere:
        mesh->mark_lead_sphere = !mesh->mark_lead_sphere;
        mesh->qshowpnts = 1;
        break;
      case mark_lead_node:
        if (mesh->mark_lead_number == 1) {
          // toggle the value
          mesh->mark_lead_number = 0;
        }
        else {
          mesh->mark_lead_number = 1;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_lead_channel:
        if (mesh->mark_lead_number == 2) {
          // toggle the value
          mesh->mark_lead_number = 0;
        }
        else {
          mesh->mark_lead_number = 2;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_lead_value:
        if (mesh->mark_lead_number == 3) {
          // toggle the value
          mesh->mark_lead_number = 0;
        }
        else {
          mesh->mark_lead_number = 3;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_lead_labels:
        if (mesh->mark_lead_number == 4) {
          // toggle the value
          mesh->mark_lead_number = 0;
        }
        else {
          mesh->mark_lead_number = 4;
          mesh->qshowpnts = 1;
        }
        break;
      case mark_lead_color:
        PickColor(mesh->mark_lead_color);
        break;
      case mark_lead_size:
        PickSize(&mesh->mark_lead_size, 10, "Leadlink Node Mark Size");
        break;
      case mark_lead_clear:
        mesh->mark_lead_number = 0;
        mesh->mark_lead_sphere = 0;
        break;
      case pick_tri_node_mark:
        PickSize(&mesh->mark_triangulate_size, 10, "Triangulation Mark Size");
        break;
        
      case mark_toggle:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, qshowpnts);
        break;
      case window_axes:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, axes);
        break;
      case window_axes_color:
        rgb_axes=false;
        PickColor(mesh->axescolor);
        break;
      case graphics_light_above:
        mesh->lighting = 1;
        lighting_direction = menu_data;
        light_position[0] = 0.f;
        light_position[1] = fabs(2 * l2norm);
        light_position[2] = 0.f;
        light_position[3] = 1.f;
        break;
      case graphics_light_below:
        mesh->lighting = 1;
        lighting_direction = menu_data;
        light_position[0] = 0.f;
        light_position[1] = -fabs(2 * l2norm);
        light_position[2] = 0.f;
        light_position[3] = 1.f;
        break;
      case graphics_light_left:
        mesh->lighting = 1;
        lighting_direction = menu_data;
        light_position[0] = -fabs(2 * l2norm);
        light_position[1] = 0.f;
        light_position[2] = 0.f;
        light_position[3] = 1.f;
        break;
      case graphics_light_right:
        mesh->lighting = 1;
        lighting_direction = menu_data;
        light_position[0] = fabs(2 * l2norm);
        light_position[1] = 0.f;
        light_position[2] = 0.f;
        light_position[3] = 1.f;
        break;
      case graphics_light_front:
        mesh->lighting = 1;
        lighting_direction = menu_data;
        light_position[0] = 0.f;
        light_position[1] = 0.f;
        light_position[2] = fabs(2 * l2norm);
        light_position[3] = 1.f;
        break;
      case graphics_light_back:
        mesh->lighting = 1;
        lighting_direction = menu_data;
        light_position[0] = 0.f;
        light_position[1] = 0.f;
        light_position[2] = -fabs(2 * l2norm);
        light_position[3] = 1.f;
        break;
      case graphics_light_none:
        mesh->lighting = 0;
        break;
      case landmark_showcor:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowcor);
        break;
      case landmark_wirecor:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.drawtype);
        break;
      case landmark_corcolor:
        PickColor(mesh->landmarkdraw.coronarycolor);
        break;
        
      case landmark_showcath:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowcath);
        break;
        
        
      case landmark_showocclus:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowocclus);
        break;
      case landmark_showstitch:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowstitch);
        break;
      case landmark_showstim:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowstim);
        break;
      case landmark_showlead:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowlead);
        break;
      case landmark_showplane:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowplane);
        break;
      case landmark_transplane:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qtransplane);
        break;
      case landmark_showrod:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowrod);
        break;
      case landmark_showpaceneedle:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowpaceneedle);
        break;
      case landmark_showrecneedle:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowrecneedle);
        break;
      case landmark_showfiber:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowfiber);
        break;
      case landmark_showcannula:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowcannula);
        break;
        
      case landmark_cathcolor:
        PickColor(mesh->landmarkdraw.cathcolor);
        break;
      case landmark_occluscolor:
        PickColor(mesh->landmarkdraw.occluscolor);
        break;
      case landmark_stitchcolor:
        PickColor(mesh->landmarkdraw.stitchcolor);
        break;
      case landmark_stimcolor:
        PickColor(mesh->landmarkdraw.stimcolor);
        break;
      case landmark_leadcolor:
        PickColor(mesh->landmarkdraw.leadcolor);
        break;
      case landmark_planecolor:
        PickColor(mesh->landmarkdraw.planecolor);
        break;
      case landmark_rodcolor:
        PickColor(mesh->landmarkdraw.rodcolor);
        break;
      case landmark_paceneedlecolor:
        PickColor(mesh->landmarkdraw.paceneedlecolor);
        break;
      case landmark_recneedlecolor:
        PickColor(mesh->landmarkdraw.recneedlecolor);
        break;
      case landmark_fibercolor:
        PickColor(mesh->landmarkdraw.fibercolor);
        break;
      case landmark_cannulacolor:
        PickColor(mesh->landmarkdraw.cannulacolor);
        break;
        
      case landmark_togglepoints:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowpoint);
        break;
      case landmark_togglerods:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowrods);
        break;
      case landmark_toggleall:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowlmark);
        break;
      case landmark_togglelabels:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, landmarkdraw.qshowlabels);
        break;
        
      case graphics_fog:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, fogging);
        break;
      case window_small_font:
        PickSize(&small_font, 10, "Small Font Size");
        break;
      case window_med_font:
        PickSize(&med_font, 10, "Medium Font Size");
        break;
      case window_large_font:
        PickSize(&large_font, 10, "Large Font Size");
        break;
      case window_attr_bg:
        PickColor(bgcolor);
        if (lpriv)
          PickColor(lpriv->bgcolor);
          for (loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
            ppriv = mesh->pickstack[loop2]->pickwin;
            PickColor(ppriv->bgcolor);
          }
            break;
      case window_attr_fg:
        PickColor(fgcolor);
        if (lpriv)
          PickColor(lpriv->fgcolor);
          for (loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
            ppriv = mesh->pickstack[loop2]->pickwin;
            PickColor(ppriv->fgcolor);
          }
            break;
      case window_attr_four:
        resize(400, 400);
        break;
      case window_attr_six:
        resize(640, 480);
        break;
      case window_attr_eight:
        resize(800, 600);
        break;
      case window_attr_ten:
        resize(1024, 768);
        break;
      case window_attr_twelve:
        resize(1280, 1024);
        break;
      case window_attr_sixteen:
        resize(1600, 1200);
        break;
        //    case window_attr_double:
        //      break;
        //    case window_attr_half:
        //      break;
      case window_attr_info_on:
        showinfotext = 1;
        if (lpriv) {
          lpriv->update();
        }
          break;
      case window_attr_info_off:
        showinfotext = 0;
        if (lpriv) {
          lpriv->update();
        }
          break;
      case window_lwindow_show:
        mesh->showlegend = true;
        if (mesh->legendwin != 0) {
          // this is if we start up map3d with the -slw 0 option, to hide the lw
          // we need to reposition it.
          if (mesh->legendwin->startHidden) {
            mesh->legendwin->startHidden = false;
            if (mesh->legendwin->specifiedCoordinates)
              mesh->legendwin->positionWindow(mesh->legendwin->width(),
                                              mesh->legendwin->height(),
                                              mesh->legendwin->x(), mesh->legendwin->y(), -1, -1);
            else
              mesh->legendwin->positionWindow(-1,-1,0,0,mesh->legendwin->width(),mesh->legendwin->height());
            mesh->legendwin->show();
            break;
          }
          mesh->legendwin->show();
        }
          break;
      case window_lwindow_hide:
        mesh->showlegend = false;
        if (mesh->legendwin)
          mesh->legendwin->setVisible(false);
        break;
      case window_save:
      {
        SaveGeomToDisk(mesh, false);
        break;
      }
      case window_save_transform:
      {
        SaveGeomToDisk(mesh, true);
        break;
      }
      case frame_dialog:
      {
        bool ok;
        // They renamed this method in Qt 4.5.
#if defined(QT_VERSION) && QT_VERSION < 0x040500
# define QtGetInt QInputDialog::getInteger
#else
# define QtGetInt QInputDialog::getInt
#endif
        int val = QtGetInt(NULL, "Set Frame Step", "Enter New Frame Step", 1,
                           1, 100, 5, &ok);
        if (ok)
          fstep = val;
        break;
      }
      case frame_reset:
        if (mesh->data){
          mesh->data->framenum = 0;
        }
        if (!mesh->cont){
          mesh->cont = new Contour_Info(mesh);
        }
        mesh->cont->buildContours();
        for(unsigned i = 0; i<mesh->fidConts.size();i++){      
          mesh->fidConts[i]->buildContours();
          mesh->fidMaps[i]->buildContours();
        }
          
        break;
      case frame_zero:
        if (mesh->data){
          mesh->data->zerotimeframe = mesh->data->framenum;
          for (int i = 0; i <= mesh->pickstacktop; i++) {
            mesh->pickstack[i]->pickwin->update();
          }
        }
        break;
      case pick_info_display:
        length = mesh->pickstacktop;
        for (loop = 0; loop <= length; loop++) {
          PickWindow *ppriv = mesh->pickstack[loop]->pickwin;
          if (ppriv->showinfotext) {
            // FIX PickWindowStyle(ppriv, 0);
          }
          else {
            // FIX PickWindowStyle(ppriv, 1);
          }
        }
          Broadcast(MAP3D_REPAINT_ALL, 0);
        break;
    }
  }
  /* switch for things that are once per window not per surface */
  switch ((menu_data)) {
    case clip_front:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->front_enabled);
      
      if (clip->front_enabled) {
        clip->lock_with_object = true;
      }
        break;
    case clip_back:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->back_enabled);
      
      if (clip->back_enabled) {
        clip->lock_with_object = true;
      }
        break;
    case clip_together:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->lock_together);
      break;
    case clip_with_object:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->lock_with_object);
      break;
    case window_locks:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, showlocks);
      break;
      
    case window_winaxes:
      all_axes = 0;
      break;
    case window_meshaxes:
      all_axes = 1;
      break;
    case screen_save:
      //we need to pop window here
      MAP3D_SLEEP(350 * MULTIPLIER);
      SaveScreen();
      break;
  }

  update();
  
}


// This one is called directly from the callback, to handle changes that
// affect everything (either one value, or all values of something).
bool GeomWindow::MenuGlobalOptions(int menu)
{
  bool retval = true;
  
  // values to be set *once*
  switch (menu) {
    case contour_dialog:
    {
      ContourDialog dialog;
      dialog.exec();
      break;
    }
    case frame_align:
      //set frame number to align to
      if (dominantsurf != -1) {
        if (meshes[dominantsurf]->data){
          map3d_info.framenum = meshes[dominantsurf]->data->framenum;
        }
      }
      else if (secondarysurf != -1) {
        if (meshes[secondarysurf]->data){
          map3d_info.framenum = meshes[secondarysurf]->data->framenum;
        }
      }
      else {
        if (meshes[0]->data){
          map3d_info.framenum = meshes[0]->data->framenum;
        }
      }
      for (MeshIterator mi; !mi.isDone(); ++mi) {
        Mesh_Info* mesh = mi.getMesh();
        if (mesh->data)
          mesh->data->framenum = map3d_info.framenum;
      }
      Broadcast(MAP3D_UPDATE, 0);
      break;
    case pick_timesignal_multiple:
      map3d_info.pickmode = NEW_WINDOW_PICK_MODE;
      break;
    case pick_timesignal_single:
      map3d_info.pickmode = REFRESH_WINDOW_PICK_MODE;
      break;
    case pick_nodeinfo:
      map3d_info.pickmode = INFO_PICK_MODE;
      break;
    case pick_triinfo:
      map3d_info.pickmode = TRIANGLE_INFO_PICK_MODE;
      break;
    case pick_triangulate:
      map3d_info.pickmode = TRIANGULATE_PICK_MODE;
      // reset the selected points in all meshes
      for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
        Mesh_Info* mesh = mi.getMesh();
        mesh->num_selected_pts = 0;
      }
      break;
    case pick_flip:
      map3d_info.pickmode = FLIP_TRIANGLE_PICK_MODE;
      break;
    case pick_edit_node:
      map3d_info.pickmode = EDIT_NODE_PICK_MODE;
      // reset the selected points in all meshes
      for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
        Mesh_Info* mesh = mi.getMesh();
        mesh->num_selected_pts = 0;
      }
      break;
    case pick_edit_lm:
      map3d_info.pickmode = EDIT_LANDMARK_PICK_MODE;
      // reset the selected points in all meshes
      for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
        Mesh_Info* mesh = mi.getMesh();
        mesh->landmarkdraw.picked_ptnum = -1;
        mesh->landmarkdraw.picked_segnum = -1;
      }
      break;
    case pick_del_node:
      map3d_info.pickmode = DELETE_NODE_PICK_MODE;
      break;
    case pick_reference:
      map3d_info.pickmode = REFERENCE_PICK_MODE;
      break;
    case pick_aperture:
      PickSize(&map3d_info.picksize, 20, "Pick Aperture Size");
      break;
    case trans_lock:
      if (map3d_info.lockrotate == LOCK_FULL && surf_group.size() > 1)
        map3d_info.lockrotate = LOCK_GROUP;
      else if (map3d_info.lockrotate != LOCK_OFF)
        map3d_info.lockrotate = LOCK_OFF;
      else
        map3d_info.lockrotate = LOCK_FULL;
      if (map3d_info.lockrotate == LOCK_OFF) {
        for (unsigned i = 0; i < numGeomWindows(); i++) {
          GeomWindow* win = GetGeomWindow(i);
          if (win->secondarysurf == -1)
            win->secondarysurf = 0;
        }
      }
      else {
        if (map3d_info.lockgeneral == LOCK_OFF || map3d_info.lockframes != LOCK_OFF) {
          for (unsigned i = 0; i < numGeomWindows(); i++) {
            GetGeomWindow(i)->secondarysurf = -1;
          }
        }
      }
      Broadcast(MAP3D_REPAINT_ALL, 0);
      break;
    case frame_lock:
      if (map3d_info.lockframes == LOCK_FULL && surf_group.size() > 1)
        map3d_info.lockframes = LOCK_GROUP;
      else if (map3d_info.lockframes != LOCK_OFF)
        map3d_info.lockframes = LOCK_OFF;
      else
        map3d_info.lockframes = LOCK_FULL;
        
      if (!map3d_info.lockframes) {
        for (unsigned i = 0; i < numGeomWindows(); i++) {
          GeomWindow* win = GetGeomWindow(i);
          if (win->secondarysurf == -1)
            win->secondarysurf = 0;
        }
      }
      else {
        if (map3d_info.lockgeneral == LOCK_OFF|| map3d_info.lockrotate != LOCK_OFF) {
          for (unsigned i = 0; i < numGeomWindows(); i++) {
            GetGeomWindow(i)->secondarysurf = -1;
          }
        }
      }
      
      Broadcast(MAP3D_REPAINT_ALL, 0);
      //ComputeLockFrameData();
      break;
    case scaling_local:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])->check_button.toggle_button,true);
      setScalingRange(LOCAL_SCALE);
      break;
    case scaling_global_surface:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GLOBAL_SURFACE])->check_button.toggle_button,true);
      setScalingRange(GLOBAL_SURFACE);
      break;
    case scaling_global_frame:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GLOBAL_FRAME])->check_button.toggle_button,true);
      setScalingRange(GLOBAL_FRAME);
      break;
    case scaling_global_global:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GLOBAL_GLOBAL])->check_button.toggle_button,true);
      setScalingRange(GLOBAL_GLOBAL);
      break;
    case scaling_local_group:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GROUP_FRAME])->check_button.toggle_button,true);
      setScalingRange(GROUP_FRAME);
      break;
    case scaling_global_group:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GROUP_GLOBAL])->check_button.toggle_button,true);
      setScalingRange(GROUP_GLOBAL);
      break;
    case scaling_local_slave:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[SLAVE_FRAME])->check_button.toggle_button,true);
      setScalingRange(SLAVE_FRAME);
      break;
    case scaling_global_slave:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[SLAVE_GLOBAL])->check_button.toggle_button,true);
      setScalingRange(SLAVE_GLOBAL);
      break;
    case scaling_function_linear:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LINEAR])->check_button.toggle_button,true);
      setScalingFunction(LINEAR);
      break;
    case scaling_function_exponential:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[EXP])->check_button.toggle_button,true);
      setScalingFunction(EXP);
      break;
    case scaling_function_logarithmic:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LOG])->check_button.toggle_button,true);
      setScalingFunction(LOG);
      break;
    case scaling_function_lab:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LAB7])->check_button.toggle_button,true);
      setScalingFunction(LAB7);
      break;
    case scaling_function_lab13:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LAB13])->check_button.toggle_button,true);
      setScalingFunction(LAB13);
      break;
    case scaling_mapping_true:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[TRUE_MAP])->check_button.toggle_button,true);
      setScalingMapping(TRUE_MAP);
      break;
    case scaling_mapping_midpoint:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[MID_MAP])->check_button.toggle_button,true);
      setScalingMapping(MID_MAP);
      break;
    case scaling_mapping_symmetric:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[SYMMETRIC])->check_button.toggle_button,true);
      setScalingMapping(SYMMETRIC);
      break;
    case scaling_mapping_separate:
      // gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[SEPARATE])->check_button.toggle_button,true);
      setScalingMapping(SEPARATE);
      break;
    case graphics_cont_smoothing:
      map3d_info.contour_antialiasing = !map3d_info.contour_antialiasing;
      Broadcast(MAP3D_REPAINT_ALL, 0);
      break;
    case frame_step_1:
      fstep = 1;
      break;
    case frame_step_2:
      fstep = 2;
      break;
    case frame_step_4:
      fstep = 4;
      break;
    case frame_step_5:
      fstep = 5;
      break;
    case frame_step_10:
      fstep = 10;
      break;
    case frame_step_45:
      fstep = 45;
      break;
    case frame_step_90:
      fstep = 90;
      break;
    case frame_step_user:
      fstep = map3d_info.user_fstep;
      break;
	case frame_loop:
      map3d_info.frame_loop = !map3d_info.frame_loop;
      break;
    case pick_mean_reference:
      // the other pick modes are in the global function
      // this one needs to be here because it changes the 
      // active mesh(es)'s properties.
      map3d_info.pickmode = MEAN_REFERENCE_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++) {
        GeomWindow* geom = GetGeomWindow(i);
        for (unsigned j = 0; j < geom->meshes.size(); j++) {
          Mesh_Info* mesh = geom->meshes[j];
          if(mesh && mesh->data)
            mesh->data->ChangeReferenceMean(mesh->geom);
        }
      }
      break;
    case pick_clear_reference:
      for (MeshIterator mi; !mi.isDone(); ++mi) {
        Mesh_Info* mesh = mi.getMesh();
        if(mesh->data)
          mesh->data->ChangeBackReference(mesh->geom);
      }
      break;
      
    case save_image:
      ImageControlDialog().exec();
      break;
    case save_map3drc:
      SaveSettings(".map3drc", true, false);
      break;
    case save_script:
    {
      QString filename = PickFile(this, true);
      if (filename == "")
        break;
      char file[256];
      QStringToCharPtr(filename, file, 256);
      SaveSettings(file, false, false);
      break;
    }
    case save_batch:
    {
      QString filename = PickFile(this, true);
      if (filename == "")
        break;
      char file[256];
      QStringToCharPtr(filename, file, 256);
      SaveSettings(file, false, true);
      break;
    }
    case scaling_dialog:
      ScaleDialog(0).exec();
      break;
    case fid_dialog:
      {
        FidDialog dialog;
        dialog.exec();
		Broadcast(MAP3D_UPDATE, 0);
      }
      break;


    default:
      retval = false;
  }
  
  if (retval)
    return retval;
  
  // values to be set *once per mesh*
  for (MeshIterator mi; !mi.isDone(); ++mi) {
    Mesh_Info* mesh = mi.getMesh();
    switch (menu) {
      case pick_show_all:
        for (int i = 0; i <= mesh->pickstacktop; i++) {
          mesh->pickstack[i]->show = 1;
          mesh->pickstack[i]->pickwin->show();
        }
        break;
      case pick_hide_all:
        for (int i = 0; i <= mesh->pickstacktop; i++) {
          mesh->pickstack[i]->show = 0;
          mesh->pickstack[i]->pickwin->hide();
        }
        break;
      case mesh_select_large_size:
        mesh->current_size_selector = &large_font;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = 5;
        mesh->current_size_mesh_based = false;
        break;
      case mesh_select_med_size:
        mesh->current_size_selector = &med_font;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = 5;
        mesh->current_size_mesh_based = false;
        break;
      case mesh_select_small_size:
        mesh->current_size_selector = &small_font;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = 5;
        mesh->current_size_mesh_based = false;
        break;
      case mesh_select_contsize:
        mesh->current_size_selector = &mesh->contsize;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        break;
      case mesh_select_meshsize:
        mesh->current_size_selector = &mesh->meshsize;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        break;
      case mesh_select_marksize:
        mesh->current_size_selector = &mesh->mark_all_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        break;
      case mesh_select_extremasize:
        mesh->current_size_selector = &mesh->mark_extrema_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        break;
      case mesh_select_tssize:
        mesh->current_size_selector = &mesh->mark_ts_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        break;
      case mesh_select_leadsize:
        mesh->current_size_selector = &mesh->mark_lead_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        break;
      case mesh_select_transsize:
        mesh->current_size_selector = &map3d_info.transincrement;
        mesh->current_size_increment = .5;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = false;
        break;
      case mesh_select_scalesize:
        mesh->current_size_selector = &map3d_info.scaleincrement;
        mesh->current_size_increment = 20;
        mesh->current_size_midpoint = 24;
        mesh->current_size_mesh_based = false;
        break;
      case mesh_select_rotsize:
        mesh->current_size_selector = &map3d_info.rotincrement;
        mesh->current_size_increment = .5;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = false;
        break;
    }
  }
  return retval;
}

int GeomWindow::OpenMenu(QPoint point)
{
  // data to determine check state
  int surface_color = 0;
  int surface_render = 0;
  int draw_mesh = 0;
  int contour_num = 2;
  bool solid_contours = true;
  bool invert = false;
  bool all_node_sphere = false, ext_node_sphere = false, pick_node_sphere = false, lead_node_sphere = false;
  bool all_value = false;
  int all_node_num = 0, ext_node_num = 0, pick_node_num = 0, lead_node_num = 0;
  bool draw_fids = false, shade_fids = false;
  bool lighting = false; 
  for (unsigned i = 0; i < meshes.size(); i++) {
    Mesh_Info* mesh = meshes[i];
    if (mesh->cmap == &Grayscale) surface_color = 2;
    else if (mesh->cmap == &Green2Red && surface_color == 0) surface_color = 1;
    else if (mesh->cmap == &Jet && surface_color == 0) surface_color = 3;

    if (mesh->shadingmodel > surface_render) surface_render = mesh->shadingmodel;
    if (mesh->drawmesh > draw_mesh) draw_mesh = mesh->drawmesh;
//    if (mesh->use_spacing) contour_num = 0;
    if (mesh->negcontdashed) solid_contours = false;
    if (mesh->invert) invert = true;
    if (mesh->mark_all_sphere) all_node_sphere = true;
    if (mesh->mark_all_sphere_value) all_node_sphere = all_value = true;
    if (mesh->mark_extrema_sphere) ext_node_sphere = true;
    if (mesh->mark_ts_sphere) pick_node_sphere = true;
    if (mesh->mark_lead_sphere) lead_node_sphere = true;
    if (mesh->drawfids) draw_fids = true;
    if (mesh->shadefids) shade_fids = true;
    if (mesh->lighting) lighting = mesh->lighting;
    all_node_num = mesh->mark_all_number;
    ext_node_num = mesh->mark_extrema_number;
    pick_node_num = mesh->mark_ts_number;
    lead_node_num = mesh->mark_lead_number;
  }

  QMenu menu(this);
  
  // add dialog entries to main menu
  menu.addAction("Files...")->setData(mesh_files_dialog);
  QMenu* submenu = menu.addMenu("Save");

  submenu->addAction("Image/Animations...")->setData(save_image);
  submenu->addAction("Settings")->setData(save_map3drc);
  submenu->addAction("Script File")->setData(save_script);
  submenu->addAction("Windows Batch File")->setData(save_batch);
  menu.addSeparator();  // Insert a separator.
  
  
  //
  // Contours...
  //
  
  submenu = menu.addMenu("Contours");
  submenu->addAction("Contour Properties...")->setData(contour_dialog);

  submenu->addSeparator();
  QMenu* subsubmenu = submenu->addMenu("Draw style");
  submenu->addAction("Line size")->setData(contour_size);
  submenu->addAction("Toggle contours  (c)")->setData(contour_toggle);
    
  QAction* action = subsubmenu->addAction("Dashed lines for negative values");
  action->setData(contour_style_dashed); action->setCheckable(true); action->setChecked(!solid_contours);

  action = subsubmenu->addAction("Solid lines for all contours");
  action->setData(contour_style_solid); action->setCheckable(true); action->setChecked(solid_contours);

  //
  // Fiducials...
  //  
  submenu = menu.addMenu("Fiducials");
  submenu->addAction("Fiducial Settings...")->setData(fid_dialog);

  action = submenu->addAction("Draw Fiducials");
  action->setData(fid_draw_fid); action->setCheckable(true); action->setChecked(draw_fids);

  action = submenu->addAction("Shade Fiducial Map (p)");
  action->setData(fid_map_shade_toggle); action->setCheckable(true); action->setChecked(shade_fids);


  //
  // frame control menu
  //
  submenu = menu.addMenu("Frame Control");
  submenu->addAction("Lock Frames  (f)")->setData(frame_lock);
  subsubmenu = submenu->addMenu("Set Interval Between Frames");
  submenu->addAction("Reset Frames to 0")->setData(frame_reset);
  submenu->addAction("Align meshes to this frame num")->setData(frame_align);
  submenu->addAction("Set time to zero")->setData(frame_zero);
  
  submenu->addAction("Set Frame Step...")->setData(frame_dialog);
  action = submenu->addAction("Frame Looping");
  action->setData(frame_loop); action->setCheckable(true); action->setChecked(map3d_info.frame_loop);

  submenu->addSeparator();

  bool user_fstep = (fstep == map3d_info.user_fstep);
  action = subsubmenu->addAction("User-specified Interval (" + QString::number(user_fstep) + ")");
  action->setData(frame_step_user); action->setCheckable(true); action->setChecked(user_fstep);
  action = subsubmenu->addAction("1");
  action->setData(frame_step_1); action->setCheckable(true); action->setChecked(fstep == 1 && !user_fstep);
  action = subsubmenu->addAction("2");
  action->setData(frame_step_2); action->setCheckable(true); action->setChecked(fstep == 2 && !user_fstep);
  action = subsubmenu->addAction("4");
  action->setData(frame_step_4); action->setCheckable(true); action->setChecked(fstep == 4 && !user_fstep);
  action = subsubmenu->addAction("5");
  action->setData(frame_step_5); action->setCheckable(true); action->setChecked(fstep == 5 && !user_fstep);
  action = subsubmenu->addAction("10");
  action->setData(frame_step_10); action->setCheckable(true); action->setChecked(fstep == 10 && !user_fstep);
  action = subsubmenu->addAction("45");
  action->setData(frame_step_45); action->setCheckable(true); action->setChecked(fstep == 45 && !user_fstep);
  action = subsubmenu->addAction("90");
  action->setData(frame_step_90); action->setCheckable(true); action->setChecked(fstep == 90 && !user_fstep);
  
  //
  // Graphics...
  // FIX - light source check box
  submenu = menu.addMenu("Graphics");
  subsubmenu = submenu->addMenu("Light source (l)");
  action = subsubmenu->addAction("From above");
  action->setData(graphics_light_above); action->setCheckable(true); action->setChecked(lighting && lighting_direction == graphics_light_above);
  action = subsubmenu->addAction("From below");
  action->setData(graphics_light_below); action->setCheckable(true); action->setChecked(lighting && lighting_direction == graphics_light_below);
  action = subsubmenu->addAction("From left");
  action->setData(graphics_light_left); action->setCheckable(true); action->setChecked(lighting && lighting_direction == graphics_light_left);
  action = subsubmenu->addAction("From right");
  action->setData(graphics_light_right); action->setCheckable(true); action->setChecked(lighting && lighting_direction == graphics_light_right);
  action = subsubmenu->addAction("From front");
  action->setData(graphics_light_front); action->setCheckable(true); action->setChecked(lighting && lighting_direction == graphics_light_front);
  action = subsubmenu->addAction("From back");
  action->setData(graphics_light_back); action->setCheckable(true); action->setChecked(lighting && lighting_direction == graphics_light_back);
  action = subsubmenu->addAction("None");
  action->setData(graphics_light_none); action->setCheckable(true); action->setChecked(lighting == false);

  subsubmenu = menu.addMenu("Toggle Clipping");
  subsubmenu->addAction("Toggle depth cue (d)")->setData(graphics_fog);
  subsubmenu->addAction("Toggle contour smoothing")->setData(graphics_cont_smoothing);

  
  
  action = subsubmenu->addAction("Front Plane");
  action->setData(clip_front); action->setCheckable(true); action->setChecked(clip->front_enabled);
  action = subsubmenu->addAction("Back Plane");
  action->setData(clip_back); action->setCheckable(true); action->setChecked(clip->back_enabled);
  action = subsubmenu->addAction("Lock Planes Together");
  action->setData(clip_together); action->setCheckable(true); action->setChecked(clip->lock_together);
  action = subsubmenu->addAction("Lock Planes with Object");
  action->setData(clip_with_object); action->setCheckable(true); action->setChecked(clip->lock_with_object);

  //
  //  Landmarks Menu
  //
  
  submenu = menu.addMenu("Landmarks");
  subsubmenu = submenu->addMenu("Coronary/Catheter");
  subsubmenu->addAction("Toggle Coronary")->setData(landmark_showcor);
  subsubmenu->addAction("Toggle Catheter")->setData(landmark_showcath);
  subsubmenu->addAction("Wireframe Coronary")->setData(landmark_wirecor);
  subsubmenu->addAction("Coronary Color")->setData(landmark_corcolor);
  subsubmenu->addAction("Catheter Color")->setData(landmark_cathcolor);

  //Spheres - toggle and color
  subsubmenu = submenu->addMenu("Points");
  subsubmenu->addAction("Toggle Temporary Occlusions")->setData(landmark_showocclus);
  subsubmenu->addAction("Toggle Permanent Occlusions (stitch)")->setData(landmark_showstitch);
  subsubmenu->addAction("Toggle Stimulation Site")->setData(landmark_showstim);
  subsubmenu->addAction("Toggle Recording site (lead)")->setData(landmark_showlead);
  
  subsubmenu->addAction("Temporary Occlusions color")->setData(landmark_occluscolor);
  subsubmenu->addAction("Permanent Occlusions (stitch) color")->setData(landmark_stitchcolor);
  subsubmenu->addAction("Stimulation Site color")->setData(landmark_stimcolor);
  subsubmenu->addAction("Recording site (lead) color ")->setData(landmark_leadcolor);
  subsubmenu->addAction("Toggle All points")->setData(landmark_togglepoints);

  subsubmenu = submenu->addMenu("Planes");
  //Planes - toggle and color
  subsubmenu->addAction("Toggle Plane")->setData(landmark_showplane);
  subsubmenu->addAction("Toggle Plane Transparency")->setData(landmark_transplane);
  subsubmenu->addAction("Plane Color")->setData(landmark_planecolor);
  
  subsubmenu = submenu->addMenu("Rods");
  //Rods - toggle and color
  subsubmenu->addAction("Toggle Rod")->setData(landmark_showrod);
  subsubmenu->addAction("Toggle Recording Needle")->setData(landmark_showrecneedle);
  subsubmenu->addAction("Toggle Pacing Needle")->setData(landmark_showpaceneedle);
  subsubmenu->addAction("Toggle Fiber")->setData(landmark_showfiber);
  subsubmenu->addAction("Toggle Cannula")->setData(landmark_showcannula);
  
  subsubmenu->addAction("Rod Color")->setData(landmark_rodcolor);
  subsubmenu->addAction("Recording Needle Color")->setData(landmark_recneedlecolor);
  subsubmenu->addAction("Pacing Needle Color")->setData(landmark_paceneedlecolor);
  subsubmenu->addAction("Fiber Color")->setData(landmark_fibercolor);
  subsubmenu->addAction("Cannula Color")->setData(landmark_cannulacolor);
  subsubmenu->addAction("Toggle All rods")->setData(landmark_togglerods);


  submenu->addAction("Toggle all landmarks")->setData(landmark_toggleall);
  submenu->addAction("Toggle landmark labels")->setData(landmark_togglelabels);
  
  //
  // Mesh...
  //
  submenu = menu.addMenu("Mesh");
  
  subsubmenu = submenu->addMenu("Render as   (m)");
  submenu->addAction("Line/point size")->setData(mesh_size);
  submenu->addAction("Color")->setData(mesh_color);
  submenu->addAction("Secondary mesh color")->setData(secondary_color);
  //submenu->addAction("Toggle mesh display")->setData(mesh_toggle);
  submenu->addAction("Show Legend Window")->setData(window_lwindow_show);
  submenu->addAction("Hide Legend Window")->setData(window_lwindow_hide);
  submenu->addAction("Reload Geometry")->setData(mesh_geom_reload);
  submenu->addAction("Reload Surface Data")->setData(mesh_data_reload);
  submenu->addAction("Reload Both Geometry and Data")->setData(mesh_both_reload);

  // using an integer as an index to determine what was checked is legacy code...
  action = subsubmenu->addAction("None");
  action->setData(mesh_render_none); action->setCheckable(true); action->setChecked(draw_mesh == 0);
  action = subsubmenu->addAction("Points");
  action->setData(mesh_render_points); action->setCheckable(true); action->setChecked(draw_mesh == 1);
  action = subsubmenu->addAction("Elements");
  action->setData(mesh_render_elements); action->setCheckable(true); action->setChecked(draw_mesh == 2);
  action = subsubmenu->addAction("Connectivity");
  action->setData(mesh_render_connectivity); action->setCheckable(true); action->setChecked(draw_mesh == 3);
  action = subsubmenu->addAction("Elements and connectivity");
  action->setData(mesh_render_elements_connectivity); action->setCheckable(true); action->setChecked(draw_mesh == 4);
  action = subsubmenu->addAction("Points and Connectivity");
  action->setData(mesh_render_points_connectivity); action->setCheckable(true); action->setChecked(draw_mesh == 5);
  action = subsubmenu->addAction("Non-Data Elements");
  action->setData(mesh_render_nondata_elements); action->setCheckable(true); action->setChecked(draw_mesh == 6);
  
  //
  // Node Markings...
  //
  // using an integer as an index to determine what was checked is legacy code...
  submenu =  menu.addMenu("Node Marking");
  subsubmenu = submenu->addMenu("All");
  action = subsubmenu->addAction("Sphere");
  action->setData(mark_all_sphere); action->setCheckable(true); action->setChecked(all_node_sphere);
  action = subsubmenu->addAction("Map data to spheres");
  action->setData(mark_all_sphere_value); action->setCheckable(true); action->setChecked(all_value);
  action = subsubmenu->addAction("Node #");
  action->setData(mark_all_node); action->setCheckable(true); action->setChecked(all_node_num == 1);
  action = subsubmenu->addAction("Channel #");
  action->setData(mark_all_channel); action->setCheckable(true); action->setChecked(all_node_num == 2);
  action = subsubmenu->addAction("Data value");
  action->setData(mark_all_value); action->setCheckable(true); action->setChecked(all_node_num == 3);
  action = subsubmenu->addAction("Fiducial value");
  action->setData(mark_all_fid); action->setCheckable(true); action->setChecked(all_node_num == 4);
  subsubmenu->addAction("Color")->setData(mark_all_color);
  subsubmenu->addAction("Size")->setData(mark_all_size);
  subsubmenu->addAction("Clear all marks")->setData(mark_all_clear);

  subsubmenu = submenu->addMenu("Extrema");
  action = subsubmenu->addAction("Sphere");
  action->setData(mark_extrema_sphere); action->setCheckable(true); action->setChecked(ext_node_sphere);
  action = subsubmenu->addAction("Node #");
  action->setData(mark_extrema_node); action->setCheckable(true); action->setChecked(ext_node_num == 1);
  action = subsubmenu->addAction("Channel #");
  action->setData(mark_extrema_channel); action->setCheckable(true); action->setChecked(ext_node_num == 2);
  action = subsubmenu->addAction("Data value");
  action->setData(mark_extrema_value); action->setCheckable(true); action->setChecked(ext_node_num == 3);
  subsubmenu->addAction("Size")->setData(mark_extrema_size);
  subsubmenu->addAction("Clear all marks")->setData(mark_extrema_clear);

  subsubmenu = submenu->addMenu("Time signal");
  action = subsubmenu->addAction("Sphere");
  action->setData(mark_ts_sphere); action->setCheckable(true); action->setChecked(pick_node_sphere);
  action = subsubmenu->addAction("Node #");
  action->setData(mark_ts_node); action->setCheckable(true); action->setChecked(pick_node_num == 1);
  action = subsubmenu->addAction("Channel #");
  action->setData(mark_ts_channel); action->setCheckable(true); action->setChecked(pick_node_num == 2);
  action = subsubmenu->addAction("Data value");
  action->setData(mark_ts_value); action->setCheckable(true); action->setChecked(pick_node_num == 3);
  subsubmenu->addAction("Color")->setData(mark_ts_color);
  subsubmenu->addAction("Size")->setData(mark_ts_size);
  subsubmenu->addAction("Clear all marks")->setData(mark_ts_clear);

  subsubmenu = submenu->addMenu("Lead links");
  action = subsubmenu->addAction("Sphere");
  action->setData(mark_lead_sphere); action->setCheckable(true); action->setChecked(lead_node_sphere);
  action = subsubmenu->addAction("Node #");
  action->setData(mark_lead_node); action->setCheckable(true); action->setChecked(lead_node_num == 1);
  action = subsubmenu->addAction("Channel #");
  action->setData(mark_lead_channel); action->setCheckable(true); action->setChecked(lead_node_num == 2);
  action = subsubmenu->addAction("Data value");
  action->setData(mark_lead_value); action->setCheckable(true); action->setChecked(lead_node_num == 3);
  action = subsubmenu->addAction("Lead labels");
  action->setData(mark_lead_labels); action->setCheckable(true); action->setChecked(lead_node_num == 4);
  subsubmenu->addAction("Color")->setData(mark_lead_color);
  subsubmenu->addAction("Size")->setData(mark_lead_size);
  subsubmenu->addAction("Clear all marks")->setData(mark_lead_clear);

  submenu->addAction("Toggle node marking (n)")->setData(mark_toggle);
  
  //
  // Pickmode...
  //
  
  submenu = menu.addMenu("Picking");
  action = submenu->addAction("Time signal (new window)");
  action->setData(pick_timesignal_multiple); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 0);
  action = submenu->addAction("Time signal (refresh window)");
  action->setData(pick_timesignal_single); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 1);
  action = submenu->addAction("Display Node Info");
  action->setData(pick_nodeinfo); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 2);
  action = submenu->addAction("Display Triangle Info");
  action->setData(pick_triinfo); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 3);
  action = submenu->addAction("Triangle Construction/Deletion");
  action->setData(pick_triangulate); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 4);
  action = submenu->addAction("Flip Triangle");
  action->setData(pick_flip); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 5);
  action = submenu->addAction("Edit Node");
  action->setData(pick_edit_node); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 6);
  action = submenu->addAction("Edit Landmark Point");
  action->setData(pick_edit_lm); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 7);
  action = submenu->addAction("Delete Node");
  action->setData(pick_del_node); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 8);
  action = submenu->addAction("Reference lead, single value");
  action->setData(pick_reference); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 9);
  action = submenu->addAction("Reference lead, mean value");
  action->setData(pick_mean_reference); action->setCheckable(true); action->setChecked(map3d_info.pickmode == 10);

  submenu->addSeparator();
  submenu->addAction("Reset Reference")->setData(pick_clear_reference);
  submenu->addAction("Show all pick windows")->setData(pick_show_all);
  submenu->addAction("Hide all pick windows")->setData(pick_hide_all);
  submenu->addAction("Size of picking aperture")->setData(pick_aperture);
  submenu->addAction("Size of triangulation node mark")->setData(pick_tri_node_mark);
  //submenu->addAction("Geometric node")->setData(pick_geom_node);
  //submenu->addAction("Geometric element")->setData(pick_geom_element);

  // 
  // Scaling...
  //
  submenu = menu.addMenu("Scaling");
  submenu->addAction("Scaling...")->setData(scaling_dialog);
  submenu->addSeparator();
  subsubmenu = submenu->addMenu("Range");
  // make sure that these, scaling functions, and scaling maps are in the same order
  // as they are declared in scalesubs.h
  action = subsubmenu->addAction("Local");
  action->setData(scaling_local); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 0);
  action = subsubmenu->addAction("Global over all frames in one surface");
  action->setData(scaling_global_surface); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 1);
  action = subsubmenu->addAction("Global over all surfaces in one frame");
  action->setData(scaling_global_frame); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 2);
  action = subsubmenu->addAction("Global over all surfaces and frames");
  action->setData(scaling_global_global); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 3);
  action = subsubmenu->addAction("Scaling over groups in one frame");
  action->setData(scaling_local_group); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 4);
  action = subsubmenu->addAction("Scaling over groups in all frames");
  action->setData(scaling_global_group); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 5);
  action = subsubmenu->addAction("Slave scaling over one frame");
  action->setData(scaling_local_slave); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 6);
  action = subsubmenu->addAction("Slave scaling over all frames");
  action->setData(scaling_global_slave); action->setCheckable(true); action->setChecked(map3d_info.scale_scope == 7);

  subsubmenu = submenu->addMenu("Function");
  action = subsubmenu->addAction("Linear");
  action->setData(scaling_function_linear); action->setCheckable(true); action->setChecked(map3d_info.scale_model == 0);
  action = subsubmenu->addAction("Exponential");
  action->setData(scaling_function_exponential); action->setCheckable(true); action->setChecked(map3d_info.scale_model == 1);
  action = subsubmenu->addAction("Logarithmic");
  action->setData(scaling_function_logarithmic); action->setCheckable(true); action->setChecked(map3d_info.scale_model == 2);
  action = subsubmenu->addAction("Lab standard");
  action->setData(scaling_function_lab); action->setCheckable(true); action->setChecked(map3d_info.scale_model == 3);
  action = subsubmenu->addAction("Lab 13 standard");
  action->setData(scaling_function_lab13); action->setCheckable(true); action->setChecked(map3d_info.scale_model == 4);

  subsubmenu = submenu->addMenu("Mapping");
  action = subsubmenu->addAction("Symmetric about zero");
  action->setData(scaling_mapping_symmetric); action->setCheckable(true); action->setChecked(map3d_info.scale_mapping == 0);
  action = subsubmenu->addAction("Separate about zero");
  action->setData(scaling_mapping_separate); action->setCheckable(true); action->setChecked(map3d_info.scale_mapping == 0);
  action = subsubmenu->addAction("True");
  action->setData(scaling_mapping_true); action->setCheckable(true); action->setChecked(map3d_info.scale_mapping == 0);
  action = subsubmenu->addAction("Symmetric about midpoint");
  action->setData(scaling_mapping_midpoint); action->setCheckable(true); action->setChecked(map3d_info.scale_mapping == 0);
  
  subsubmenu = submenu->addMenu("Grouping");
  subsubmenu->addAction("Move to group 1")->setData(scaling_group_one);
  subsubmenu->addAction("Move to group 2")->setData(scaling_group_two);
  subsubmenu->addAction("Move to group 3")->setData(scaling_group_three);
  subsubmenu->addAction("Move to group 4")->setData(scaling_group_four);
  
  //
  // Surface...
  // 
  submenu = menu.addMenu("Surface Data");
  subsubmenu = submenu->addMenu("Color              (a)");
  action = subsubmenu->addAction("Rainbow");
  action->setData(surface_color_rainbow); action->setCheckable(true); action->setChecked(surface_color == 0);
  action = subsubmenu->addAction("Green to Red");
  action->setData(surface_color_red2green); action->setCheckable(true); action->setChecked(surface_color == 1);
  action = subsubmenu->addAction("White to Black");
  action->setData(surface_color_grayscale); action->setCheckable(true); action->setChecked(surface_color == 2);
  action = subsubmenu->addAction("Jet (Matlab)");
  action->setData(surface_color_jet); action->setCheckable(true); action->setChecked(surface_color == 3);
  subsubmenu->addSeparator();
  action = subsubmenu->addAction("Invert (i)");
  action->setData(surface_color_invert); action->setCheckable(true); action->setChecked(invert);

  subsubmenu = submenu->addMenu("Render style (s)");
  action = subsubmenu->addAction("None");
  action->setData(surface_render_none); action->setCheckable(true); action->setChecked(surface_render == 0);
  action = subsubmenu->addAction("Flat");
  action->setData(surface_render_flat); action->setCheckable(true); action->setChecked(surface_render == 1);
  action = subsubmenu->addAction("Gouraud");
  action->setData(surface_render_gouraud); action->setCheckable(true); action->setChecked(surface_render == 2);
  action = subsubmenu->addAction("Banded");
  action->setData(surface_render_banded); action->setCheckable(true); action->setChecked(surface_render == 3);
  
  // Use +/- to select from main root menu
  // FIX checks
  submenu = menu.addMenu("Use +/- to select");
  action = submenu->addAction("Large Font Size");
  action->setData(mesh_select_large_size); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Medium Font Size");
  action->setData(mesh_select_med_size); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Small Font Size");
  action->setData(mesh_select_small_size); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Contour Size");
  action->setData(mesh_select_contsize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Line/Point Size");
  action->setData(mesh_select_meshsize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Node Marks (All) Size");
  action->setData(mesh_select_marksize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Node Marks (Extrema) Size");
  action->setData(mesh_select_extremasize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Node Marks (Time Signal) Size");
  action->setData(mesh_select_tssize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Node Marks (Leads) Size");
  action->setData(mesh_select_leadsize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Change in translation");
  action->setData(mesh_select_transsize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Chagne in scaling");
  action->setData(mesh_select_scalesize); action->setCheckable(true); action->setChecked(false);
  action = submenu->addAction("Change in rotation");
  action->setData(mesh_select_rotsize); action->setCheckable(true); action->setChecked(false);
  
  //
  // window attributes...
  //

  submenu = menu.addMenu("Window Attributes");
  subsubmenu = submenu->addMenu("Screen info");
  subsubmenu->addAction("turn screen info on")->setData(window_attr_info_on);
  subsubmenu->addAction("turn screen info off")->setData(window_attr_info_off);
  subsubmenu->addAction("show/hide lock icons")->setData(window_locks);
  subsubmenu->addAction("Show Legend Window")->setData(window_lwindow_show);
  subsubmenu->addAction("Hide Legend Window")->setData(window_lwindow_hide);

  subsubmenu = submenu->addMenu("Color");
  subsubmenu->addAction("background")->setData(window_attr_bg);
  subsubmenu->addAction("foreground")->setData(window_attr_fg);

  subsubmenu = submenu->addMenu("Window Size");
  subsubmenu->addAction("400x400 size")->setData(window_attr_four);
  subsubmenu->addAction("640x480 size")->setData(window_attr_six);
  subsubmenu->addAction("800x600 size")->setData(window_attr_eight);
  subsubmenu->addAction("1024x768 size")->setData(window_attr_ten);
  subsubmenu->addAction("1280x1024 size")->setData(window_attr_twelve);
  subsubmenu->addAction("1600x1200 size")->setData(window_attr_sixteen);

  subsubmenu = submenu->addMenu("Axes");
  subsubmenu->addAction("Axes Color")->setData(window_axes_color);
  subsubmenu->addAction("Toggle Axes")->setData(window_axes);
  subsubmenu->addAction("One set per window")->setData(window_winaxes);
  subsubmenu->addAction("One set per mesh")->setData(window_meshaxes);

  subsubmenu = submenu->addMenu("Font Size");
  
  subsubmenu->addAction("Toggle Transformation lock (t)")->setData(trans_lock);
  subsubmenu->addAction("Small Font Size")->setData(window_small_font);
  subsubmenu->addAction("Medium Font Size")->setData(window_med_font);
  subsubmenu->addAction("Large Font Size")->setData(window_large_font);
  
  action = menu.exec(point);
  if (action)
    return action->data().toInt();
  else
    return -1;

}
