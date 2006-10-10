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

#include "colormaps.h"
#include "eventdata.h"
#include "pickinfo.h"
#include "reportstate.h"
#include "scalesubs.h"
#include "savescreen.h"
#include "savestate.h"

#ifdef OSX
#  include <stdlib.h>
#else
#  include <malloc.h>
#endif
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <math.h>
#ifdef OSX
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

extern Map3d_Info map3d_info;
extern vector<Surface_Group> surf_group;
extern MainWindow* masterWindow;
extern int fstep;
extern ScaleDialog *scaledialog;
extern FrameDialog *framedialog;
extern FidDialog *fiddialog;
extern ContourDialog *contourdialog;
extern FogDialog *fogdialog;
extern SaveDialog *savedialog;

bool menulock = false;

void GeomWindowHandleMenu(menu_data * data)
{
  GeomWindow *priv = (GeomWindow *) data->priv;
  Clip_Planes *clip = priv->clip;
  LegendWindow *lpriv = 0;
  PickWindow *ppriv = 0;
  int length = priv->meshes.size();
  int loop = 0;
  int loop2;
  int numconts = 50;
  Mesh_Info *mesh = 0;
  
  gdk_gl_drawable_gl_begin(priv->gldrawable, priv->glcontext);
  
  /* Do two passes - 
    one for those that apply depending on the general lock status, 
    one for those that apply only once per window (clipping stuff) 
    PUT GLOBAL THINGS IN GeomWindowMenuGlobalOptions */
  
  //If frame command work off of frame lock instead of general lock.
  if(length > 1 && !map3d_info.lockframes && (data->data == frame_reset || data->data == frame_zero)){
    loop = priv->secondarysurf;
    length = loop +1;
  }
  else if (length > 1 && !map3d_info.lockgeneral) {
    loop = priv->dominantsurf;
    length = loop + 1;
  }
  
  
  for (; loop < length; loop++) {
    mesh = priv->meshes[loop];
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;
    if (mesh->legendwin != 0)
      lpriv = mesh->legendwin;
    else
      lpriv = 0;
    
    //switch for every mesh in the window, based on general lock status
    switch ((data->data)) {
      case surface_color_rainbow:
      {
        mesh->cmap = &Rainbow;
        if (lpriv) {
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
        }
        priv->surf_color.setActive(0);
        break;
      }
      case surface_color_red2green:
      {
        mesh->cmap = &Green2Red;
        if (lpriv) {
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
        }
        priv->surf_color.setActive(1);
        break;
      }
      case surface_color_grayscale:
      {
        mesh->cmap = &Grayscale;
        if (lpriv) {
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
        }
        menulock = true;
        priv->surf_color.setActive(2);
        break;
      }
      case surface_color_jet:
      {
        mesh->cmap = &Jet;
        if (lpriv) {
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
        }
        menulock = true;
        priv->surf_color.setActive(3);
        break;
      }
      case surface_color_invert:
        MAP3D_MESH_LOCK_TOGGLE(map3d_info.lockgeneral, invert);
        menulock = true;
        priv->surf_invert.setValue(0, mesh->invert);
        menulock = false;
        
        if (lpriv) {
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
        }
          break;
      case surface_render_none:
        mesh->shadingmodel = SHADE_NONE;
        priv->surf_render.setActive(0);
        if (mesh->legendwin != 0)
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
          break;
      case surface_render_flat:
        mesh->shadingmodel = SHADE_FLAT;
        
        if (mesh->legendwin != 0)
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
          priv->surf_render.setActive(1);
        break;
      case surface_render_gouraud:
        mesh->shadingmodel = SHADE_GOURAUD;
        if (mesh->legendwin != 0)
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
          priv->surf_render.setActive(2);
        break;
      case surface_render_banded:
        mesh->shadingmodel = SHADE_BANDED;
        
        if (mesh->legendwin != 0)
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
          priv->surf_render.setActive(3);
        break;
      case fid_dialog:
        fidDialogCreate(true);
        break;
      case fid_map_dialog:
        fidMapDialogCreate(true);
        break;
      case fid_map_toggle:
        switch (mesh->fidshadingmodel) {
          case SHADE_NONE:
            mesh->fidshadingmodel = SHADE_GOURAUD;
            mesh->drawcont = true;
            break;
          case SHADE_FLAT:
            mesh->fidshadingmodel = SHADE_GOURAUD;
            mesh->drawcont = true;
            break;
          case SHADE_GOURAUD:
            mesh->fidshadingmodel = SHADE_BANDED;
            mesh->drawcont = true;
            break;
          case SHADE_BANDED:
            mesh->fidshadingmodel = SHADE_NONE;
            mesh->drawcont = false;
            break;
        }
        break;
      case no_fid_cont:
        mesh->drawactcont = false;
        mesh->drawreccont = false;
        menulock = true;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->actFidCont), 0);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->recFidCont), 0);
        menulock = false;
        break;
      case act_fid_cont:
        mesh->drawactcont = true;
        menulock = true;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->noFidCont), 0);
        menulock = false;
        break;
      case rec_fid_cont:
        mesh->drawreccont = true;
        menulock = true;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->noFidCont), 0);
        menulock = false;
        break;
      case no_fid_map:
        mesh->drawfidmapcont = -1;
        //       mesh->drawactmapcont = false;
        //       mesh->drawrecmapcont = false;
        mesh->drawcont = true;
        menulock = true;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->actFidMap), 0);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->recFidMap), 0);
        menulock = false;
        break;
      case act_fid_map:
        mesh->drawfidmapcont = 10;
        //       mesh->drawactmapcont = true;
        //       mesh->drawrecmapcont = false;
        mesh->drawcont = false;
        menulock = true;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->noFidMap), 0);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->recFidMap), 0);
        menulock = false;
        break;
      case rec_fid_map:
        mesh->drawfidmapcont = 13;
        //       mesh->drawrecmapcont = true;
        //       mesh->drawactmapcont = false;
        mesh->drawcont = false;
        menulock = true;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->noFidMap), 0);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->actFidMap), 0);
        menulock = false;
        break;
        
      case scaling_dialog:
        gtk_widget_show(scaledialog->window);
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
        priv->mesh_render.setActive(0);
        break;
      case mesh_render_points:
        mesh->drawmesh = RENDER_MESH_PTS;
        priv->mesh_render.setActive(1);
        break;
      case mesh_render_elements:
        mesh->drawmesh = RENDER_MESH_ELTS;
        priv->mesh_render.setActive(2);
        break;
      case mesh_render_connectivity:
        mesh->drawmesh = RENDER_MESH_CONN;
        priv->mesh_render.setActive(3);
        break;
      case mesh_render_elements_connectivity:
        mesh->drawmesh = RENDER_MESH_ELTS_CONN;
        priv->mesh_render.setActive(4);
        break;
      case mesh_render_points_connectivity:
        mesh->drawmesh = RENDER_MESH_PTS_CONN;
        priv->mesh_render.setActive(5);
        break;
      case mesh_render_nondata_elements: 
        mesh->drawmesh = RENDER_MESH_NONDATA_ELTS;
        priv->mesh_render.setActive(6);
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
        if (loop == 0 || priv->meshes[loop-1]->mysurf != mesh->mysurf) {
          FindAndReadGeom(mesh->mysurf, priv->findMeshesFromSameInput(mesh), RELOAD_GEOM);
          GeomWindowUpdateAndRedraw(priv);
        }
        break;
      case mesh_data_reload:
        if (loop == 0 || priv->meshes[loop-1]->mysurf != mesh->mysurf) {
          FindAndReadGeom(mesh->mysurf, priv->findMeshesFromSameInput(mesh), RELOAD_DATA);
          GeomWindowUpdateAndRedraw(priv);
        }
        break;
      case mesh_both_reload:
        if (loop == 0 || priv->meshes[loop-1]->mysurf != mesh->mysurf) {
          FindAndReadGeom(mesh->mysurf, priv->findMeshesFromSameInput(mesh), RELOAD_BOTH);
          GeomWindowUpdateAndRedraw(priv);
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
        priv->cont_num.setActive(numconts/5);
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
            
            gtk_widget_queue_draw(lpriv->drawarea);
          }
        }
          numconts = 50;
        break;
      case contour_spacing_user:
        priv->cont_num.setActive(0);
        /* get rid of old contours */
        
        /* generate contours/bands */
        //mesh->use_spacing = true;
        numconts = mesh->cont->buildContours();
        
        // if legend window matches contours, change it.
        if (mesh->legendwin != 0) {
          lpriv = mesh->legendwin;
          if (lpriv->matchContours)
            lpriv->nticks = numconts + 2;
          
          gtk_widget_queue_draw(lpriv->drawarea);
        }
          numconts = 50;
        break;
      case contour_dialog:
        //if(mesh->data){
          contourDialogCreate(true);
        //}
        break;
      case contour_style_dashed:
        mesh->negcontdashed = 1;
        mesh->drawcont = 1;
        priv->cont_draw_style.setActive(0);
        break;
      case contour_style_solid:
        mesh->negcontdashed = 0;
        mesh->drawcont = 1;
        priv->cont_draw_style.setActive(1);
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
        priv->node_all_sphere.setValue(0, mesh->mark_all_sphere);
        break;
      case mark_all_sphere_value:
        mesh->mark_all_sphere_value = !mesh->mark_all_sphere_value;
        if (mesh->mark_all_sphere_value) {
          // turn on mark_all_sphere too
          mesh->mark_all_sphere = true;
          mesh->qshowpnts = 1;
          priv->node_all_sphere.setValue(0, mesh->mark_all_sphere);
        }
          mesh->qshowpnts = 1;
        priv->node_all_sphere.setValue(1, mesh->mark_all_sphere_value);
        break;
      case mark_all_node:
        if (mesh->mark_all_number == 1) {
          // toggle the value
          mesh->mark_all_number = 0;
          for (int i = 0; i < 4; i++)
            priv->node_all.setValue(i, false);
        }
        else {
          mesh->mark_all_number = 1;
          mesh->qshowpnts = 1;
          priv->node_all.setActive(0);
        }
        break;
      case mark_all_channel:
        if (mesh->mark_all_number == 2) {
          // toggle the value
          mesh->mark_all_number = 0;
          for (int i = 0; i < 4; i++)
            priv->node_all.setValue(i, false);
        }
        else {
          mesh->mark_all_number = 2;
          mesh->qshowpnts = 1;
          priv->node_all.setActive(1);
        }
        break;
      case mark_all_value:
        if (mesh->mark_all_number == 3) {
          // toggle the value
          mesh->mark_all_number = 0;
          for (int i = 0; i < 4; i++)
            priv->node_all.setValue(i, false);
        }
        else {
          mesh->mark_all_number = 3;
          mesh->qshowpnts = 1;
          priv->node_all.setActive(2);
        }
        break;
      case mark_all_fid:
        if (mesh->mark_all_number == 4) {
          // toggle the value
          mesh->mark_all_number = 0;
          for (int i = 0; i < 4; i++)
            priv->node_all.setValue(i, false);
        }
        else {
          mesh->mark_all_number = 4;
          mesh->qshowpnts = 1;
          priv->node_all.setActive(3);
        }
        break;
      case mark_all_clear:
        mesh->mark_all_number = 0;
        mesh->mark_all_sphere = 0;
        priv->node_all_sphere.setValue(0, false);
        for (int i = 0; i < 4; i++)
          priv->node_all.setValue(i, false);
          //mesh->qshowpnts=1;
          break;
      case mark_all_color:
        PickColor(mesh->mark_all_color);
        //mesh->qshowpnts=0;
        break;
      case mark_all_size:
        PickSize(&mesh->mark_all_size, 10, "All Node Marks Size");
        //mesh->qshowpnts=0;
        break;
      case mark_extrema_sphere:
        mesh->mark_extrema_sphere = !mesh->mark_extrema_sphere;
        mesh->qshowpnts = 1;
        priv->node_ext_sphere.setValue(0, mesh->mark_extrema_sphere);
        break;
      case mark_extrema_node:
        if (mesh->mark_extrema_number == 1) {
          // toggle the value
          mesh->mark_extrema_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_ext.setValue(i, false);
        }
        else {
          mesh->mark_extrema_number = 1;
          mesh->qshowpnts = 1;
          priv->node_ext.setActive(0);
        }
        break;
      case mark_extrema_channel:
        if (mesh->mark_extrema_number == 2) {
          // toggle the value
          mesh->mark_extrema_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_ext.setValue(i, false);
        }
        else {
          mesh->mark_extrema_number = 2;
          mesh->qshowpnts = 1;
          priv->node_ext.setActive(1);
        }
        break;
      case mark_extrema_value:
        if (mesh->mark_extrema_number == 3) {
          // toggle the value
          mesh->mark_extrema_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_ext.setValue(i, false);
        }
        else {
          mesh->mark_extrema_number = 3;
          mesh->qshowpnts = 1;
          priv->node_ext.setActive(2);
        }
        break;
      case mark_extrema_size:
        PickSize(&mesh->mark_extrema_size, 10, "Extrema Node Mark Size");
        mesh->qshowpnts = 1;
        break;
      case mark_extrema_clear:
        mesh->mark_extrema_number = 0;
        mesh->mark_extrema_sphere = 0;
        priv->node_ext_sphere.setValue(0, false);
        for (int i = 0; i < 3; i++)
          priv->node_ext.setValue(i, false);
          //mesh->qshowpnts=1;
          break;
      case mark_ts_sphere:
        mesh->mark_ts_sphere = !mesh->mark_ts_sphere;
        mesh->qshowpnts = 1;
        priv->node_pick_sphere.setValue(0, mesh->mark_ts_sphere);
        break;
      case mark_ts_node:
        if (mesh->mark_ts_number == 1) {
          // toggle the value
          mesh->mark_ts_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_pick.setValue(i, false);
        }
        else {
          mesh->mark_ts_number = 1;
          mesh->qshowpnts = 1;
          priv->node_pick.setActive(0);
        }
        break;
      case mark_ts_channel:
        if (mesh->mark_ts_number == 2) {
          // toggle the value
          mesh->mark_ts_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_pick.setValue(i, false);
        }
        else {
          mesh->mark_ts_number = 2;
          mesh->qshowpnts = 1;
          priv->node_pick.setActive(1);
        }
        break;
      case mark_ts_value:
        if (mesh->mark_ts_number == 3) {
          // toggle the value
          mesh->mark_ts_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_pick.setValue(i, false);
        }
        else {
          mesh->mark_ts_number = 3;
          mesh->qshowpnts = 1;
          priv->node_pick.setActive(2);
        }
        break;
      case mark_ts_color:
        PickColor(mesh->mark_ts_color);
        //mesh->qshowpnts=1;
        break;
      case mark_ts_size:
        PickSize(&mesh->mark_ts_size, 10, "Time Series Node Mark Size");
        //mesh->qshowpnts=1;
        break;
      case mark_ts_clear:
        mesh->mark_ts_number = 0;
        mesh->mark_ts_sphere = 0;
        priv->node_pick_sphere.setValue(0, false);
        for (int i = 0; i < 3; i++)
          priv->node_pick.setValue(i, false);
          //mesh->qshowpnts=1;
          break;
      case mark_lead_sphere:
        mesh->mark_lead_sphere = !mesh->mark_lead_sphere;
        mesh->qshowpnts = 1;
        priv->node_lead_sphere.setValue(0, mesh->mark_lead_sphere);
        break;
      case mark_lead_node:
        if (mesh->mark_lead_number == 1) {
          // toggle the value
          mesh->mark_lead_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_lead.setValue(i, false);
        }
        else {
          mesh->mark_lead_number = 1;
          mesh->qshowpnts = 1;
          priv->node_lead.setActive(0);
        }
        break;
      case mark_lead_channel:
        if (mesh->mark_lead_number == 2) {
          // toggle the value
          mesh->mark_lead_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_lead.setValue(i, false);
        }
        else {
          mesh->mark_lead_number = 2;
          mesh->qshowpnts = 1;
          priv->node_lead.setActive(1);
        }
        break;
      case mark_lead_value:
        if (mesh->mark_lead_number == 3) {
          // toggle the value
          mesh->mark_lead_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_lead.setValue(i, false);
        }
        else {
          mesh->mark_lead_number = 3;
          mesh->qshowpnts = 1;
          priv->node_lead.setActive(2);
        }
        break;
      case mark_lead_labels:
        if (mesh->mark_lead_number == 4) {
          // toggle the value
          mesh->mark_lead_number = 0;
          for (int i = 0; i < 3; i++)
            priv->node_lead.setValue(i, false);
        }
        else {
          mesh->mark_lead_number = 4;
          mesh->qshowpnts = 1;
          priv->node_lead.setActive(3);
        }
        break;
      case mark_lead_color:
        PickColor(mesh->mark_lead_color);
        //mesh->qshowpnts=1;
        break;
      case mark_lead_size:
        PickSize(&mesh->mark_lead_size, 10, "Leadlink Node Mark Size");
        //mesh->qshowpnts=1;
        break;
      case mark_lead_clear:
        mesh->mark_lead_number = 0;
        mesh->mark_lead_sphere = 0;
        priv->node_lead_sphere.setValue(0, false);
        for (int i = 0; i < 4; i++)
          priv->node_lead.setValue(i, false);
          //mesh->qshowpnts=1;
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
        priv->rgb_axes=false;
        PickColor(mesh->axescolor);
        break;
      case graphics_light_above:
        mesh->lighting = 1;
        priv->light_position[0] = 0.f;
        priv->light_position[1] = fabs(2 * priv->l2norm);
        priv->light_position[2] = 0.f;
        priv->light_position[3] = 1.f;
        priv->lighting.setActive(0);
        break;
      case graphics_light_below:
        mesh->lighting = 1;
        priv->light_position[0] = 0.f;
        priv->light_position[1] = -fabs(2 * priv->l2norm);
        priv->light_position[2] = 0.f;
        priv->light_position[3] = 1.f;
        priv->lighting.setActive(1);
        break;
      case graphics_light_left:
        mesh->lighting = 1;
        priv->light_position[0] = -fabs(2 * priv->l2norm);
        priv->light_position[1] = 0.f;
        priv->light_position[2] = 0.f;
        priv->light_position[3] = 1.f;
        priv->lighting.setActive(2);
        break;
      case graphics_light_right:
        mesh->lighting = 1;
        priv->light_position[0] = fabs(2 * priv->l2norm);
        priv->light_position[1] = 0.f;
        priv->light_position[2] = 0.f;
        priv->light_position[3] = 1.f;
        priv->lighting.setActive(3);
        break;
      case graphics_light_front:
        mesh->lighting = 1;
        priv->light_position[0] = 0.f;
        priv->light_position[1] = 0.f;
        priv->light_position[2] = fabs(2 * priv->l2norm);
        priv->light_position[3] = 1.f;
        priv->lighting.setActive(4);
        break;
      case graphics_light_back:
        mesh->lighting = 1;
        priv->light_position[0] = 0.f;
        priv->light_position[1] = 0.f;
        priv->light_position[2] = -fabs(2 * priv->l2norm);
        priv->light_position[3] = 1.f;
        priv->lighting.setActive(5);
        break;
      case graphics_light_none:
        mesh->lighting = 0;
        priv->lighting.setActive(6);
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
      case graphics_fog_adjust:
        fogDialogCreate(priv);
        break;
      case window_small_font:
        PickSize(&priv->small_font, 10, "Small Font Size");
        break;
      case window_med_font:
        PickSize(&priv->med_font, 10, "Medium Font Size");
        break;
      case window_large_font:
        PickSize(&priv->large_font, 10, "Large Font Size");
        break;
      case window_attr_bg:
        PickColor(priv->bgcolor);
        if (lpriv)
          PickColor(lpriv->bgcolor);
          for (loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
            ppriv = mesh->pickstack[loop2]->pickwin;
            PickColor(ppriv->bgcolor);
          }
            break;
      case window_attr_fg:
        PickColor(priv->fgcolor);
        if (lpriv)
          PickColor(lpriv->fgcolor);
          for (loop2 = 0; loop2 <= mesh->pickstacktop; loop2++) {
            ppriv = mesh->pickstack[loop2]->pickwin;
            PickColor(ppriv->fgcolor);
          }
            break;
      case window_attr_four:
        priv->width = 400;
        priv->height = 400;
        if (masterWindow)
          gtk_widget_set_size_request(priv->drawarea, priv->width, priv->height);
        else
          gtk_window_resize(GTK_WINDOW(priv->window), priv->width, priv->height);
        break;
      case window_attr_six:
        priv->width = 640;
        priv->height = 480;
        if (masterWindow)
          gtk_widget_set_size_request(priv->drawarea, priv->width, priv->height);
        else
          gtk_window_resize(GTK_WINDOW(priv->window), priv->width, priv->height);
        break;
      case window_attr_eight:
        priv->width = 800;
        priv->height = 600;
        if (masterWindow)
          gtk_widget_set_size_request(priv->drawarea, priv->width, priv->height);
        else
          gtk_window_resize(GTK_WINDOW(priv->window), priv->width, priv->height);
        break;
      case window_attr_ten:
        priv->width = 1024;
        priv->height = 768;
        if (masterWindow)
          gtk_widget_set_size_request(priv->drawarea, priv->width, priv->height);
        else
          gtk_window_resize(GTK_WINDOW(priv->window), priv->width, priv->height);
        break;
      case window_attr_twelve:
        priv->width = 1280;
        priv->height = 1024;
        if (masterWindow)
          gtk_widget_set_size_request(priv->drawarea, priv->width, priv->height);
        else
          gtk_window_resize(GTK_WINDOW(priv->window), priv->width, priv->height);
        break;
      case window_attr_sixteen:
        priv->width = 1600;
        priv->height = 1200;
        if (masterWindow)
          gtk_widget_set_size_request(priv->drawarea, priv->width, priv->height);
        else
          gtk_window_resize(GTK_WINDOW(priv->window), priv->width, priv->height);
        break;
        //    case window_attr_double:
        //      break;
        //    case window_attr_half:
        //      break;
      case window_attr_info_on:
        priv->showinfotext = 1;
        if (lpriv) {
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
        }
          break;
      case window_attr_info_off:
        priv->showinfotext = 0;
        if (lpriv) {
          gtk_widget_queue_draw(mesh->legendwin->drawarea);
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
              mesh->legendwin->positionWindow(mesh->legendwin->width,
                                              mesh->legendwin->height,
                                              mesh->legendwin->x, mesh->legendwin->y, -1, -1);
            else
              mesh->legendwin->positionWindow(-1,-1,0,0,mesh->legendwin->width,mesh->legendwin->height);
            mesh->legendwin->setPosAndShow();
            break;
          }
          if (masterWindow)
            gtk_widget_show(mesh->legendwin->drawarea);
          else
            gtk_widget_show(mesh->legendwin->window);
        }
          break;
      case window_lwindow_hide:
        mesh->showlegend = false;
        if (mesh->legendwin != 0) {
          if (masterWindow)
            gtk_widget_hide(mesh->legendwin->drawarea);
          else
            gtk_widget_hide(mesh->legendwin->window);
        }
          break;
      case window_save_dialog:
      {
        saveDialogCreate();
        break;
      }
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
        frameDialogCreate();
        gtk_widget_show(framedialog->window);
        break;
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
        }
          for(unsigned i = 0; i<mesh->fidMaps.size();i++){      
            mesh->fidMaps[i]->buildContours();
          }
 //         if(mesh->fidactcont)
//            mesh->fidactcont->buildContours();
//          if(mesh->fidreccont)
//            mesh->fidreccont->buildContours();
//            if(mesh->fidactmapcont)
//              mesh->fidactmapcont->buildContours();
//              if(mesh->fidrecmapcont)
//                mesh->fidrecmapcont->buildContours();
                //ComputeLockFrameData();
        break;
      case frame_zero:
        if (mesh->data){
          mesh->data->zerotimeframe = mesh->data->framenum;
          for (int i = 0; i <= mesh->pickstacktop; i++) {
            gtk_widget_queue_draw(mesh->pickstack[i]->pickwin->drawarea);
          }
        }
        break;
      case pick_info_display:
        length = mesh->pickstacktop;
        for (loop = 0; loop <= length; loop++) {
          PickWindow *ppriv = mesh->pickstack[loop]->pickwin;
          if (ppriv->showinfotext) {
            PickWindowStyle(ppriv, 0);
          }
          else {
            PickWindowStyle(ppriv, 1);
          }
        }
          Broadcast(MAP3D_REPAINT_ALL, 0);
        break;
    }
  }
  /* switch for things that are once per window not per surface */
  switch ((data->data)) {
    case clip_front:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->front_enabled);
      menulock = true;
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->front_plane),clip->front_enabled );
      menulock = false;
      
      if (clip->front_enabled) {
        clip->lock_with_object = true;
        glEnable(GL_CLIP_PLANE0);
      }
        else {
          glDisable(GL_CLIP_PLANE0);
        }
        break;
    case clip_back:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->back_enabled);
      menulock = true;
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->back_plane), clip->back_enabled);
      menulock = false;
      
      if (clip->back_enabled) {
        clip->lock_with_object = true;
        glEnable(GL_CLIP_PLANE1);
      }
        else {
          glDisable(GL_CLIP_PLANE1);
        }
        break;
    case clip_together:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->lock_together);
      menulock = true;
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->c_together),clip->lock_together );
      menulock = false;
      break;
    case clip_with_object:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, clip->lock_with_object);
      menulock = true;
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->c_with_object),clip->lock_with_object );
      menulock = false;
      break;
    case window_locks:
      MAP3D_WINDOW_LOCK_TOGGLE(map3d_info.lockgeneral, showlocks);
      break;
      
    case window_winaxes:
      priv->all_axes = 0;
      break;
    case window_meshaxes:
      priv->all_axes = 1;
      break;
    case screen_save:
      //we need to pop window here
      MAP3D_SLEEP(350 * MULTIPLIER);
      SaveScreen();
      break;
  }
  gdk_gl_drawable_gl_end(priv->gldrawable);
  gtk_widget_queue_draw(priv->drawarea);
  
}


// This one is called directly from the callback, to handle changes that
// affect everything (either one value, or all values of something).
bool GeomWindowMenuGlobalOptions(menu_data * data)
{
  bool retval = true;
  GeomWindow *priv = (GeomWindow *) data->priv;
  int menu = data->data;
  
  // values to be set *once*
  switch (menu) {
    case frame_align:
      //set frame number to align to
      if (priv->dominantsurf != -1) {
        if (priv->meshes[priv->dominantsurf]->data){
          map3d_info.framenum = priv->meshes[priv->dominantsurf]->data->framenum;
        }
      }
      else if (priv->secondarysurf != -1) {
        if (priv->meshes[priv->secondarysurf]->data){
          map3d_info.framenum = priv->meshes[priv->secondarysurf]->data->framenum;
        }
      }
      else {
        if (priv->meshes[0]->data){
          map3d_info.framenum = priv->meshes[0]->data->framenum;
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
      // the index in the setActive function is based on the order it was put in the group
      // see GeomBuildMenus
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(0);
        break;
    case pick_timesignal_single:
      map3d_info.pickmode = REFRESH_WINDOW_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(1);
        break;
    case pick_nodeinfo:
      map3d_info.pickmode = INFO_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(2);
        break;
    case pick_triinfo:
      map3d_info.pickmode = TRIANGLE_INFO_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(3);
        break;
    case pick_triangulate:
      map3d_info.pickmode = TRIANGULATE_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(4);
        
        // reset the selected points in all meshes
        for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
          Mesh_Info* mesh = mi.getMesh();
          mesh->num_selected_pts = 0;
        }
          break;
    case pick_flip:
      map3d_info.pickmode = FLIP_TRIANGLE_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(5);
        break;
    case pick_edit_node:
      map3d_info.pickmode = EDIT_NODE_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(6);
        
        // reset the selected points in all meshes
        for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
          Mesh_Info* mesh = mi.getMesh();
          mesh->num_selected_pts = 0;
        }
          break;
    case pick_edit_lm:
      map3d_info.pickmode = EDIT_LANDMARK_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(7);
        
        // reset the selected points in all meshes
        for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
          Mesh_Info* mesh = mi.getMesh();
          mesh->landmarkdraw.picked_ptnum = -1;
          mesh->landmarkdraw.picked_segnum = -1;
        }
          break;
    case pick_del_node:
      map3d_info.pickmode = DELETE_NODE_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(8);
        break;
    case pick_reference:
      map3d_info.pickmode = REFERENCE_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->picking.setActive(9);
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
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])->check_button.toggle_button,true);
      setScalingRange(LOCAL_SCALE);
      break;
    case scaling_global_surface:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GLOBAL_SURFACE])->check_button.toggle_button,true);
      setScalingRange(GLOBAL_SURFACE);
      break;
    case scaling_global_frame:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GLOBAL_FRAME])->check_button.toggle_button,true);
      setScalingRange(GLOBAL_FRAME);
      break;
    case scaling_global_global:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GLOBAL_GLOBAL])->check_button.toggle_button,true);
      setScalingRange(GLOBAL_GLOBAL);
      break;
    case scaling_local_group:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GROUP_FRAME])->check_button.toggle_button,true);
      setScalingRange(GROUP_FRAME);
      break;
    case scaling_global_group:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[GROUP_GLOBAL])->check_button.toggle_button,true);
      setScalingRange(GROUP_GLOBAL);
      break;
    case scaling_local_slave:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[SLAVE_FRAME])->check_button.toggle_button,true);
      setScalingRange(SLAVE_FRAME);
      break;
    case scaling_global_slave:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->range[SLAVE_GLOBAL])->check_button.toggle_button,true);
      setScalingRange(SLAVE_GLOBAL);
      break;
    case scaling_function_linear:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LINEAR])->check_button.toggle_button,true);
      setScalingFunction(LINEAR);
      break;
    case scaling_function_exponential:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[EXP])->check_button.toggle_button,true);
      setScalingFunction(EXP);
      break;
    case scaling_function_logarithmic:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LOG])->check_button.toggle_button,true);
      setScalingFunction(LOG);
      break;
    case scaling_function_lab:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LAB7])->check_button.toggle_button,true);
      setScalingFunction(LAB7);
      break;
    case scaling_function_lab13:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->func[LAB13])->check_button.toggle_button,true);
      setScalingFunction(LAB13);
      break;
    case scaling_mapping_true:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[TRUE_MAP])->check_button.toggle_button,true);
      setScalingMapping(TRUE_MAP);
      break;
    case scaling_mapping_midpoint:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[MID_MAP])->check_button.toggle_button,true);
      setScalingMapping(MID_MAP);
      break;
    case scaling_mapping_symmetric:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[SYMMETRIC])->check_button.toggle_button,true);
      setScalingMapping(SYMMETRIC);
      break;
    case scaling_mapping_separate:
      gtk_toggle_button_set_active(&GTK_RADIO_BUTTON(scaledialog->map[SEPARATE])->check_button.toggle_button,true);
      setScalingMapping(SEPARATE);
      break;
    case graphics_cont_smoothing:
      map3d_info.contour_antialiasing = !map3d_info.contour_antialiasing;
      Broadcast(MAP3D_REPAINT_ALL, 0);
      break;
    case frame_step_1:
      fstep = 1;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(1);
        //ComputeLockFrameData();
      break;
    case frame_step_2:
      fstep = 2;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(2);
        //ComputeLockFrameData();
      break;
    case frame_step_4:
      fstep = 4;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(3);
        //ComputeLockFrameData();
      break;
    case frame_step_5:
      fstep = 5;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(4);
        //ComputeLockFrameData();
      break;
    case frame_step_10:
      fstep = 10;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(5);
        //ComputeLockFrameData();
      break;
    case frame_step_45:
      fstep = 45;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(6);
        //ComputeLockFrameData();
      break;
    case frame_step_90:
      fstep = 90;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(7);
        //ComputeLockFrameData();
      break;
    case frame_step_user:
      fstep = map3d_info.user_fstep;
      for (unsigned i = 0; i < numGeomWindows(); i++)
        GetGeomWindow(i)->frame_num.setActive(0);
        //ComputeLockFrameData();
      break;
    case pick_mean_reference:
      // the other pick modes are in the global function
      // this one needs to be here because it changes the 
      // active mesh(es)'s properties.
      map3d_info.pickmode = MEAN_REFERENCE_PICK_MODE;
      for (unsigned i = 0; i < numGeomWindows(); i++) {
        GeomWindow* geom = GetGeomWindow(i);
        geom->picking.setActive(10); 
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
      
    default:
      retval = false;
  }
  
  if (retval)
    return retval;
  
  // values to be set *once per mesh*
  for (MeshIterator mi; !mi.isDone(); ++mi) {
    Mesh_Info* mesh = mi.getMesh();
    GeomWindow* priv = mesh->gpriv;
    switch (menu) {
      case pick_show_all:
        for (int i = 0; i <= mesh->pickstacktop; i++) {
          mesh->pickstack[i]->show = 1;
          if (masterWindow)
            gtk_widget_show(mesh->pickstack[i]->pickwin->drawarea);
          else
            gtk_widget_show(mesh->pickstack[i]->pickwin->window);
        }
        break;
      case pick_hide_all:
        for (int i = 0; i <= mesh->pickstacktop; i++) {
          mesh->pickstack[i]->show = 0;
          if (masterWindow)
            gtk_widget_hide(mesh->pickstack[i]->pickwin->drawarea);
          else
            gtk_widget_hide(mesh->pickstack[i]->pickwin->window);
        }
        break;
      case mesh_select_large_size:
        mesh->current_size_selector = &priv->large_font;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = 5;
        mesh->current_size_mesh_based = false;
        priv->mesh_select.setActive(0);
        break;
      case mesh_select_med_size:
        mesh->current_size_selector = &priv->med_font;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = 5;
        mesh->current_size_mesh_based = false;
        priv->mesh_select.setActive(1);
        break;
      case mesh_select_small_size:
        mesh->current_size_selector = &priv->small_font;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = 5;
        mesh->current_size_mesh_based = false;
        priv->mesh_select.setActive(2);
        break;
      case mesh_select_contsize:
        mesh->current_size_selector = &mesh->contsize;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        priv->mesh_select.setActive(3);
        break;
      case mesh_select_meshsize:
        mesh->current_size_selector = &mesh->meshsize;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        priv->mesh_select.setActive(4);
        break;
      case mesh_select_marksize:
        mesh->current_size_selector = &mesh->mark_all_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        priv->mesh_select.setActive(5);
        break;
      case mesh_select_extremasize:
        mesh->current_size_selector = &mesh->mark_extrema_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        priv->mesh_select.setActive(6);
        break;
      case mesh_select_tssize:
        mesh->current_size_selector = &mesh->mark_ts_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        priv->mesh_select.setActive(7);
        break;
      case mesh_select_leadsize:
        mesh->current_size_selector = &mesh->mark_lead_size;
        mesh->current_size_increment = 10;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = true;
        priv->mesh_select.setActive(8);
        break;
      case mesh_select_transsize:
        mesh->current_size_selector = &map3d_info.transincrement;
        mesh->current_size_increment = .5;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = false;
        priv->mesh_select.setActive(9);
        break;
      case mesh_select_scalesize:
        mesh->current_size_selector = &map3d_info.scaleincrement;
        mesh->current_size_increment = 20;
        mesh->current_size_midpoint = 24;
        mesh->current_size_mesh_based = false;
        priv->mesh_select.setActive(10);
        break;
      case mesh_select_rotsize:
        mesh->current_size_selector = &map3d_info.rotincrement;
        mesh->current_size_increment = .5;
        mesh->current_size_midpoint = mesh->current_size_increment / 2;
        mesh->current_size_mesh_based = false;
        priv->mesh_select.setActive(11);
        break;
    }
  }
  return retval;
}


void GeomBuildMenus(GeomWindow * priv)
{
  priv->menu = gtk_menu_new();
  
  GtkWidget *main_menus[12];    // main menu has 10 submenus/entries
  
  // build all menus top down
  
  // add dialog entries to main menu
  AddMenuEntry(priv->menu, "Files", mesh_files_dialog, priv, GeomWindowMenu);
  AddMenuEntry(priv->menu, "Save", window_save_dialog, priv, GeomWindowMenu);
  AddMenuEntry(priv->menu, "", -1, priv, GeomWindowMenu);  // Insert a separator.
  
  
  //
  // Contours...
  //
  
  GtkWidget *contour_menus[2];  // contours has 3 submenus
  
  main_menus[3] = AddSubMenu(priv->menu, "Contours");
  //contour_menus[0] = AddSubMenu(main_menus[3], "Number of contours");
  
  
  GtkWidget *contourbutton = AddMenuEntry(main_menus[3], "Set Contour Num/Spacing", contour_dialog, priv, GeomWindowMenu);
  GtkTooltips *contour_button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(contour_button_tips, GTK_WIDGET(contourbutton),
                       "Brings up a dialog that allows "
                       "user to adjust the number of contours.", "");
  //
  AddMenuEntry(main_menus[3], "", contour_dialog, priv, GeomWindowMenu);  // Insert a separator.
  contour_menus[1] = AddSubMenu(main_menus[3], "Draw style");
  AddMenuEntry(main_menus[3], "Line size", contour_size, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[3], "Toggle contours  (c)", contour_toggle, priv, GeomWindowMenu);
  
  
 /* priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "User-specified spacing", contour_spacing_user, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "5", contour_number_5, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "10", contour_number_10, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "15", contour_number_15, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "20", contour_number_20, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "25", contour_number_25, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "30", contour_number_30, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "35", contour_number_35, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "40", contour_number_40, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "45", contour_number_45, priv, GeomWindowMenu));
  priv->cont_num.add(AddCheckMenuEntry(contour_menus[0], "50", contour_number_50, priv, GeomWindowMenu));
  */
  
  priv->cont_draw_style.add(AddCheckMenuEntry(contour_menus[1], "Dashed lines for negative values", contour_style_dashed, priv, GeomWindowMenu));
  priv->cont_draw_style.add(AddCheckMenuEntry(contour_menus[1], "Solid lines for all contours", contour_style_solid, priv, GeomWindowMenu));
  
  //
  // Fiducials...
  //
  
  //GtkWidget *fiducial_menus[2];  
  
  main_menus[10] = AddSubMenu(priv->menu, "Fiducials");
  GtkWidget *fiducialbutton = AddMenuEntry(main_menus[10], "Fiducial Contours", fid_dialog, priv, GeomWindowMenu);
  GtkTooltips *fiducial_button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(fiducial_button_tips, GTK_WIDGET(fiducialbutton),
                       "Brings up a dialog that allows "
                       "user to control fiducial contours.", "");
  GtkWidget *fiducialmapbutton = AddMenuEntry(main_menus[10], "Fiducial Maps", fid_map_dialog, priv, GeomWindowMenu);
  GtkTooltips *fiducial_map_button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(fiducial_map_button_tips, GTK_WIDGET(fiducialmapbutton),
                       "Brings up a dialog that allows "
                       "user to control fiducial maps.", "");
  //AddMenuEntry(main_menus[10], "", fid_dialog, priv, GeomWindowMenu);  // Insert a separator.
  
  //  
  //  fiducial_menus[0] = AddSubMenu(main_menus[10], "Fiducial Contours");
  //  fiducial_menus[1] = AddSubMenu(main_menus[10], "Fiducial Maps");
  // 
  //  //
  //  priv->noFidCont =
  //    AddCheckMenuEntry(fiducial_menus[0], "No Fiducials", no_fid_cont, priv, GeomWindowMenu);
  //  priv->actFidCont =
  //    AddCheckMenuEntry(fiducial_menus[0], "Activation Time", act_fid_cont, priv, GeomWindowMenu);  
  //  priv->recFidCont =
  //    AddCheckMenuEntry(fiducial_menus[0], "Recovery Time", rec_fid_cont, priv, GeomWindowMenu);
  //  
  //  gtk_widget_set_sensitive(priv->actFidCont, false);
  //  gtk_widget_set_sensitive(priv->recFidCont, false);
  //
  //  menulock = true;
  //  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->noFidCont), 1);
  //  menulock = false;
  //  
  //
  //  priv->noFidMap =
  //    AddCheckMenuEntry(fiducial_menus[1], "No Fiducials", no_fid_map, priv, GeomWindowMenu);
  //  priv->actFidMap =
  //    AddCheckMenuEntry(fiducial_menus[1], "Activation Map", act_fid_map, priv, GeomWindowMenu);  
  //  priv->recFidMap =
  //    AddCheckMenuEntry(fiducial_menus[1], "Recovery Map", rec_fid_map, priv, GeomWindowMenu);
  //
  //  gtk_widget_set_sensitive(priv->actFidMap, false);
  //  gtk_widget_set_sensitive(priv->recFidMap, false);
  //
  //  menulock = true;
  //  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(priv->noFidMap), 1);
  //  menulock = false;
  
  //
  // frame control menu
  //
  
  GtkWidget *step_menu;
  
  main_menus[8] = AddSubMenu(priv->menu, "Frame Control");
  AddMenuEntry(main_menus[8], "Lock Frames  (f)", frame_lock, priv, GeomWindowMenu);
  step_menu = AddSubMenu(main_menus[8], "Set Interval Between Frames");
  AddMenuEntry(main_menus[8], "Reset Frames to 0", frame_reset, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[8], "Align meshes to this frame num", frame_align, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[8], "Set time to zero", frame_zero, priv, GeomWindowMenu);
  
  GtkWidget *framebutton = AddMenuEntry(step_menu, "Set Frame Step...", frame_dialog, priv, GeomWindowMenu);
  GtkTooltips *frame_button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(frame_button_tips, GTK_WIDGET(framebutton),
                       "Brings up a dialog that allows "
                       "user to adjust the frame step.", "");
  //
  AddMenuEntry(step_menu, "", frame_dialog, priv, GeomWindowMenu);  // Insert a separator.
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "User-specified Interval", frame_step_user, priv, GeomWindowMenu));
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "1", frame_step_1, priv, GeomWindowMenu));
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "2", frame_step_2, priv, GeomWindowMenu));
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "4", frame_step_4, priv, GeomWindowMenu));
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "5", frame_step_5, priv, GeomWindowMenu));
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "10", frame_step_10, priv, GeomWindowMenu));
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "45", frame_step_45, priv, GeomWindowMenu));
  priv->frame_num.add(AddCheckMenuEntry(step_menu, "90", frame_step_90, priv, GeomWindowMenu));
  
  //
  // Graphics...
  //
  
  GtkWidget *graphics_menus[2];
  
  main_menus[6] = AddSubMenu(priv->menu, "Graphics");
  graphics_menus[0] = AddSubMenu(main_menus[6], "Light source (l)");
  graphics_menus[1] = AddSubMenu(main_menus[6], "Toggle Clipping");
  AddMenuEntry(main_menus[6], "Toggle depth cue (d)", graphics_fog, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[6], "Adjust depth cue", graphics_fog_adjust, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[6], "Toggle contour smoothing", graphics_cont_smoothing, priv, GeomWindowMenu);

  
  priv->lighting.add(AddCheckMenuEntry(graphics_menus[0], "From above", graphics_light_above, priv, GeomWindowMenu));
  priv->lighting.add(AddCheckMenuEntry(graphics_menus[0], "From below", graphics_light_below, priv, GeomWindowMenu));
  priv->lighting.add(AddCheckMenuEntry(graphics_menus[0], "From left", graphics_light_left, priv, GeomWindowMenu));
  priv->lighting.add(AddCheckMenuEntry(graphics_menus[0], "From right", graphics_light_right, priv, GeomWindowMenu));
  priv->lighting.add(AddCheckMenuEntry(graphics_menus[0], "From front", graphics_light_front, priv, GeomWindowMenu));
  priv->lighting.add(AddCheckMenuEntry(graphics_menus[0], "From back", graphics_light_back, priv, GeomWindowMenu));
  priv->lighting.add(AddCheckMenuEntry(graphics_menus[0], "None", graphics_light_none, priv, GeomWindowMenu));
  
  priv->front_plane = AddCheckMenuEntry(graphics_menus[1], "Front Plane                             (<)", clip_front, priv, GeomWindowMenu);
  priv->back_plane = AddCheckMenuEntry(graphics_menus[1], "Back Plane                             (>)", clip_back, priv, GeomWindowMenu);
  priv->c_together = AddCheckMenuEntry(graphics_menus[1], "Locking Planes Together          (.)", clip_together, priv, GeomWindowMenu);
  priv->c_with_object = AddCheckMenuEntry(graphics_menus[1], "Locking Planes with Object       (,)", clip_with_object, priv, GeomWindowMenu);
  
  //
  //  Landmarks Menu
  //
  
  GtkWidget *landmark_menus[4];
  
  main_menus[7] = AddSubMenu(priv->menu, "Landmarks");
  landmark_menus[0] = AddSubMenu(main_menus[7], "Coronary/Catheter");
  landmark_menus[1] = AddSubMenu(main_menus[7], "Points");
  landmark_menus[2] = AddSubMenu(main_menus[7], "Planes");
  landmark_menus[3] = AddSubMenu(main_menus[7], "Rods");
  AddMenuEntry(main_menus[7], "Toggle all landmarks", landmark_toggleall, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[7], "Toggle landmark labels", landmark_togglelabels, priv, GeomWindowMenu);
  
  AddMenuEntry(landmark_menus[0], "Toggle Coronary", landmark_showcor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[0], "Toggle Catheter", landmark_showcath, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[0], "Wireframe Coronary", landmark_wirecor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[0], "Coronary Color", landmark_corcolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[0], "Catheter Color", landmark_cathcolor, priv, GeomWindowMenu);
  
  
  //Spheres - toggle and color
  AddMenuEntry(landmark_menus[1], "Toggle Temporary Occlusions", landmark_showocclus, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[1], "Toggle Permanent Occlusions (stitch)", landmark_showstitch, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[1], "Toggle Stimulation Site", landmark_showstim, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[1], "Toggle Recording site (lead)", landmark_showlead, priv, GeomWindowMenu);
  
  AddMenuEntry(landmark_menus[1], "Temporary Occlusions color", landmark_occluscolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[1], "Permanent Occlusions (stitch) color", landmark_stitchcolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[1], "Stimulation Site color", landmark_stimcolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[1], "Recording site (lead) color ", landmark_leadcolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[1], "Toggle All points", landmark_togglepoints, priv, GeomWindowMenu);
  
  //Planes - toggle and color
  AddMenuEntry(landmark_menus[2], "Toggle Plane", landmark_showplane, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[2], "Toggle Plane Transparency", landmark_transplane, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[2], "Plane Color", landmark_planecolor, priv, GeomWindowMenu);
  
  //Rods - toggle and color
  AddMenuEntry(landmark_menus[3], "Toggle Rod", landmark_showrod, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Toggle Recording Needle", landmark_showrecneedle, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Toggle Pacing Needle", landmark_showpaceneedle, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Toggle Fiber", landmark_showfiber, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Toggle Cannula", landmark_showcannula, priv, GeomWindowMenu);
  
  AddMenuEntry(landmark_menus[3], "Rod Color", landmark_rodcolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Recording Needle Color", landmark_recneedlecolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Pacing Needle Color", landmark_paceneedlecolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Fiber Color", landmark_fibercolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Cannula Color", landmark_cannulacolor, priv, GeomWindowMenu);
  AddMenuEntry(landmark_menus[3], "Toggle All rods", landmark_togglerods, priv, GeomWindowMenu);
  
  //
  // Mesh...
  //
  
  GtkWidget *mesh_menus[2];
  main_menus[2] = AddSubMenu(priv->menu, "Mesh");
  
  mesh_menus[0] = AddSubMenu(main_menus[2], "Render as   (m)");
  AddMenuEntry(main_menus[2], "Line/point size", mesh_size, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[2], "Color", mesh_color, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[2], "Secondary mesh color", secondary_color, priv, GeomWindowMenu);
  //AddMenuEntry(main_menus[2], "Toggle mesh display", mesh_toggle, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[2], "Show Legend Window", window_lwindow_show, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[2], "Hide Legend Window", window_lwindow_hide, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[2], "Reload Geometry", mesh_geom_reload, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[2], "Reload Surface Data", mesh_data_reload, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[2], "Reload Both Geometry and Data", mesh_both_reload, priv, GeomWindowMenu);
  
  priv->mesh_render.add(AddCheckMenuEntry(mesh_menus[0], "None", mesh_render_none, priv, GeomWindowMenu));
  priv->mesh_render.add(AddCheckMenuEntry(mesh_menus[0], "Points", mesh_render_points, priv, GeomWindowMenu));
  priv->mesh_render.add(AddCheckMenuEntry(mesh_menus[0], "Elements", mesh_render_elements, priv, GeomWindowMenu));
  priv->mesh_render.add(AddCheckMenuEntry(mesh_menus[0], "Connectivity", mesh_render_connectivity, priv, GeomWindowMenu));
  priv->mesh_render.add(AddCheckMenuEntry(mesh_menus[0], "Elements and connectivity", mesh_render_elements_connectivity, priv, GeomWindowMenu));
  priv->mesh_render.add(AddCheckMenuEntry(mesh_menus[0], "Points and Connectivity", mesh_render_points_connectivity, priv, GeomWindowMenu));
  priv->mesh_render.add(AddCheckMenuEntry(mesh_menus[0], "Non-Data Elements", mesh_render_nondata_elements, priv, GeomWindowMenu));
  
  //
  // Node Markings...
  //
  
  GtkWidget *marking_menus[4];  // marking has 4 submenus
  
  main_menus[5] = AddSubMenu(priv->menu, "Node Marking");
  marking_menus[0] = AddSubMenu(main_menus[5], "All");
  marking_menus[1] = AddSubMenu(main_menus[5], "Extrema");
  marking_menus[2] = AddSubMenu(main_menus[5], "Time signal");
  marking_menus[3] = AddSubMenu(main_menus[5], "Lead links");
  AddMenuEntry(main_menus[5], "Toggle node marking (n)", mark_toggle, priv, GeomWindowMenu);
  
  priv->node_all_sphere.add(AddCheckMenuEntry(marking_menus[0], "Sphere", mark_all_sphere, priv, GeomWindowMenu));
  priv->node_all_sphere.add(AddCheckMenuEntry(marking_menus[0], "Map data to spheres", mark_all_sphere_value, priv, GeomWindowMenu));
  priv->node_all.add(AddCheckMenuEntry(marking_menus[0], "Node #", mark_all_node, priv, GeomWindowMenu));
  priv->node_all.add(AddCheckMenuEntry(marking_menus[0], "Channel #", mark_all_channel, priv, GeomWindowMenu));
  priv->node_all.add(AddCheckMenuEntry(marking_menus[0], "Data value", mark_all_value, priv, GeomWindowMenu));
  priv->node_all.add(AddCheckMenuEntry(marking_menus[0], "Fiducial value", mark_all_fid, priv, GeomWindowMenu));
  AddMenuEntry(marking_menus[0], "Color", mark_all_color, priv, GeomWindowMenu);
  AddMenuEntry(marking_menus[0], "Size", mark_all_size, priv, GeomWindowMenu);
  AddMenuEntry(marking_menus[0], "Clear all marks", mark_all_clear, priv, GeomWindowMenu);
  
  priv->node_ext_sphere.add(AddCheckMenuEntry(marking_menus[1], "Sphere", mark_extrema_sphere, priv, GeomWindowMenu));
  priv->node_ext.add(AddCheckMenuEntry(marking_menus[1], "Node #", mark_extrema_node, priv, GeomWindowMenu));
  priv->node_ext.add(AddCheckMenuEntry(marking_menus[1], "Channel #", mark_extrema_channel, priv, GeomWindowMenu));
  priv->node_ext.add(AddCheckMenuEntry(marking_menus[1], "Data value", mark_extrema_value, priv, GeomWindowMenu));
  AddMenuEntry(marking_menus[1], "Size", mark_extrema_size, priv, GeomWindowMenu);
  AddMenuEntry(marking_menus[1], "Clear all marks", mark_extrema_clear, priv, GeomWindowMenu);
  
  priv->node_pick_sphere.add(AddCheckMenuEntry(marking_menus[2], "Spheres", mark_ts_sphere, priv, GeomWindowMenu));
  priv->node_pick.add(AddCheckMenuEntry(marking_menus[2], "Node #", mark_ts_node, priv, GeomWindowMenu));
  priv->node_pick.add(AddCheckMenuEntry(marking_menus[2], "Channel #", mark_ts_channel, priv, GeomWindowMenu));
  priv->node_pick.add(AddCheckMenuEntry(marking_menus[2], "Data value", mark_ts_value, priv, GeomWindowMenu));
  AddMenuEntry(marking_menus[2], "Color", mark_ts_color, priv, GeomWindowMenu);
  AddMenuEntry(marking_menus[2], "Size", mark_ts_size, priv, GeomWindowMenu);
  AddMenuEntry(marking_menus[2], "Clear all marks", mark_ts_clear, priv, GeomWindowMenu);
  
  priv->node_lead_sphere.add(AddCheckMenuEntry(marking_menus[3], "Spheres", mark_lead_sphere, priv, GeomWindowMenu));
  priv->node_lead.add(AddCheckMenuEntry(marking_menus[3], "Node #", mark_lead_node, priv, GeomWindowMenu));
  priv->node_lead.add(AddCheckMenuEntry(marking_menus[3], "Channel #", mark_lead_channel, priv, GeomWindowMenu));
  priv->node_lead.add(AddCheckMenuEntry(marking_menus[3], "Data value", mark_lead_value, priv, GeomWindowMenu));
  priv->node_lead.add(AddCheckMenuEntry(marking_menus[3], "Lead labels", mark_lead_labels, priv, GeomWindowMenu));
  AddMenuEntry(marking_menus[3], "Color", mark_lead_color, priv, GeomWindowMenu);
  AddMenuEntry(marking_menus[3], "Size", mark_lead_size, priv, GeomWindowMenu);
  AddMenuEntry(marking_menus[3], "Clear all marks", mark_lead_clear, priv, GeomWindowMenu);
  
  //
  // Pickmode...
  //
  
  main_menus[4] = AddSubMenu(priv->menu, "Picking");
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Time signal (new window)", pick_timesignal_multiple, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Time signal (refresh window)", pick_timesignal_single, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Display Node Info", pick_nodeinfo, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Display Triangle Info", pick_triinfo, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Triangle Contruction/Deletion", pick_triangulate, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Flip Triangle", pick_flip, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Edit Node", pick_edit_node, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Edit Landmark Point", pick_edit_lm, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4], "Delete Node", pick_del_node, priv, GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4],"Reference lead, single value",pick_reference,priv,GeomWindowMenu));
  priv->picking.add(AddCheckMenuEntry(main_menus[4],"Reference lead, mean value",pick_mean_reference,priv,GeomWindowMenu));  
  AddMenuEntry(main_menus[4],"Reset Reference",pick_clear_reference,priv,GeomWindowMenu);
  //AddMenuEntry(main_menus[4], "Show Time Signal Window Info (p)", pick_info_display, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[4], "Show all pick windows", pick_show_all, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[4], "Hide all pick windows", pick_hide_all, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[4], "Size of picking aperture", pick_aperture, priv, GeomWindowMenu);
  AddMenuEntry(main_menus[4], "Size of triangulation node mark", pick_tri_node_mark, priv, GeomWindowMenu);
  //AddMenuEntry(main_menus[4],"Geometric node",pick_geom_node,GeomWindowMenu);
  //AddMenuEntry(main_menus[4],"Geometric element",pick_geom_element,GeomWindowMenu);
  // 
  // Scaling...
  //
  
  GtkWidget *scaling_menus[5];
  
  main_menus[1] = AddSubMenu(priv->menu, "Scaling");
  GtkWidget *button = AddMenuEntry(main_menus[1], "Scaling...", scaling_dialog, priv, GeomWindowMenu);
  // Attempt at adding a tool tip:
  GtkTooltips *button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(button_tips, GTK_WIDGET(button),
                       "Brings up a dialog that allows quicker access to the options "
                       "provided by the following menu items.", "");
  //
  AddMenuEntry(main_menus[1], "", scaling_dialog, priv, GeomWindowMenu);  // Insert a separator.
  scaling_menus[0] = AddSubMenu(main_menus[1], "Range");
  scaling_menus[1] = AddSubMenu(main_menus[1], "Function");
  scaling_menus[2] = AddSubMenu(main_menus[1], "Mapping");
  scaling_menus[3] = AddSubMenu(main_menus[1], "Grouping");
  //scaling_menus[4] = AddSubMenu(main_menus[1], "Contour Spacing");
  
  // make sure that these, scaling functions, and scaling maps are in the same order
  // as they are declared in scalesubs.h
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Local", scaling_local, priv, GeomWindowMenu));
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Global over all frames in one surface", scaling_global_surface, priv, GeomWindowMenu));
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Global over all surfaces in one frame", scaling_global_frame, priv, GeomWindowMenu));
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Global over all surfaces and frames", scaling_global_global, priv, GeomWindowMenu));
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Scaling over groups in one frame", scaling_local_group, priv, GeomWindowMenu));
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Scaling over groups in all frames", scaling_global_group, priv, GeomWindowMenu));
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Slave scaling over one frame", scaling_local_slave, priv, GeomWindowMenu));
  priv->scaling_range.add(AddCheckMenuEntry(scaling_menus[0], "Slave scaling over all frames", scaling_global_slave, priv, GeomWindowMenu));
  
  AddMenuEntry(scaling_menus[3], "Move to group 1", scaling_group_one, priv, GeomWindowMenu);
  AddMenuEntry(scaling_menus[3], "Move to group 2", scaling_group_two, priv, GeomWindowMenu);
  AddMenuEntry(scaling_menus[3], "Move to group 3", scaling_group_three, priv, GeomWindowMenu);
  AddMenuEntry(scaling_menus[3], "Move to group 4", scaling_group_four, priv, GeomWindowMenu);
  
  priv->scaling_function.add(AddCheckMenuEntry(scaling_menus[1], "Linear", scaling_function_linear, priv, GeomWindowMenu));
  priv->scaling_function.add(AddCheckMenuEntry(scaling_menus[1], "Exponential", scaling_function_exponential, priv, GeomWindowMenu));
  priv->scaling_function.add(AddCheckMenuEntry(scaling_menus[1], "Logarithmic", scaling_function_logarithmic, priv, GeomWindowMenu));
  priv->scaling_function.add(AddCheckMenuEntry(scaling_menus[1], "Lab standard", scaling_function_lab, priv, GeomWindowMenu));
  priv->scaling_function.add(AddCheckMenuEntry(scaling_menus[1], "Lab 13 standard", scaling_function_lab13, priv, GeomWindowMenu));
  
  priv->scaling_map.add(AddCheckMenuEntry(scaling_menus[2], "Symmetric about zero", scaling_mapping_symmetric, priv, GeomWindowMenu));
  priv->scaling_map.add(AddCheckMenuEntry(scaling_menus[2], "Separate about zero", scaling_mapping_separate, priv, GeomWindowMenu));
  priv->scaling_map.add(AddCheckMenuEntry(scaling_menus[2], "True", scaling_mapping_true, priv, GeomWindowMenu));
  priv->scaling_map.add(AddCheckMenuEntry(scaling_menus[2], "Symmetric about midpoint", scaling_mapping_midpoint, priv, GeomWindowMenu));
  
  //  AddMenuEntry(scaling_menus[4], "True", scaling_mapping_true, priv, GeomWindowMenu);
  //   AddMenuEntry(scaling_menus[4], "Symmetric about zero", scaling_mapping_symmetric, priv, GeomWindowMenu);
  //   AddMenuEntry(scaling_menus[4], "Separate about zero", scaling_mapping_separate, priv, GeomWindowMenu);
  
  
  //
  // Surface...
  // 
  
  GtkWidget *surface_menus[2];
  
  main_menus[0] = AddSubMenu(priv->menu, "Surface Data");
  surface_menus[0] = AddSubMenu(main_menus[0], "Color              (a)");
  surface_menus[1] = AddSubMenu(main_menus[0], "Render style (s)");
  
  priv->surf_color.add(AddCheckMenuEntry(surface_menus[0], "Rainbow", surface_color_rainbow, priv, GeomWindowMenu));
  priv->surf_color.add(AddCheckMenuEntry(surface_menus[0], "Green to Red", surface_color_red2green, priv, GeomWindowMenu));
  priv->surf_color.add(AddCheckMenuEntry(surface_menus[0], "White to Black", surface_color_grayscale, priv, GeomWindowMenu));
  priv->surf_color.add(AddCheckMenuEntry(surface_menus[0], "Jet (Matlab)", surface_color_jet, priv, GeomWindowMenu));

  priv->surf_invert.add(AddCheckMenuEntry(surface_menus[0], "Invert      (i)", surface_color_invert, priv, GeomWindowMenu));
  
  priv->surf_render.add(AddCheckMenuEntry(surface_menus[1], "None", surface_render_none, priv, GeomWindowMenu));
  priv->surf_render.add(AddCheckMenuEntry(surface_menus[1], "Flat", surface_render_flat, priv, GeomWindowMenu));
  priv->surf_render.add(AddCheckMenuEntry(surface_menus[1], "Gouraud", surface_render_gouraud, priv, GeomWindowMenu));
  priv->surf_render.add(AddCheckMenuEntry(surface_menus[1], "Banded", surface_render_banded, priv, GeomWindowMenu));
  
  
  // Use +/- to select from main root menu
  main_menus[11] = AddSubMenu(priv->menu, "Use +/- to select");
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Large Font Size", mesh_select_large_size, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Medium Font Size", mesh_select_med_size, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Small Font Size", mesh_select_small_size, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Contour Size", mesh_select_contsize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Line/Point Size", mesh_select_meshsize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Node Marks (All) Size", mesh_select_marksize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Node Marks (Extrema) Size", mesh_select_extremasize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Node Marks (Time Signal) Size", mesh_select_tssize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Node Marks (Leads) Size", mesh_select_leadsize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Change in translation", mesh_select_transsize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Change in scaling", mesh_select_scalesize, priv, GeomWindowMenu));
  priv->mesh_select.add(AddCheckMenuEntry(main_menus[11], "Change in rotation", mesh_select_rotsize, priv, GeomWindowMenu));
  
  //
  // window attributes...
  //
  
  GtkWidget *attr_menus[6];
  
  main_menus[9] = AddSubMenu(priv->menu, "Window Attributes");
  attr_menus[0] = AddSubMenu(main_menus[9], "Screen info");
  attr_menus[1] = AddSubMenu(main_menus[9], "Color");
  attr_menus[2] = AddSubMenu(main_menus[9], "Window Size");
  attr_menus[3] = AddSubMenu(main_menus[9], "Axes");
  attr_menus[4] = AddSubMenu(main_menus[9], "Font Size");
  
  AddMenuEntry(main_menus[9], "Toggle Transformation lock (t)", trans_lock, priv, GeomWindowMenu);
  
  AddMenuEntry(attr_menus[0], "turn screen info on", window_attr_info_on, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[0], "turn screen info off", window_attr_info_off, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[0], "show/hide lock icons", window_locks, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[0], "Show Legend Window", window_lwindow_show, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[0], "Hide Legend Window", window_lwindow_hide, priv, GeomWindowMenu);
  
  AddMenuEntry(attr_menus[1], "background", window_attr_bg, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[1], "foreground", window_attr_fg, priv, GeomWindowMenu);
  
  AddMenuEntry(attr_menus[2], "400x400 size", window_attr_four, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[2], "640x480 size", window_attr_six, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[2], "800x600 size", window_attr_eight, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[2], "1024x768 size", window_attr_ten, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[2], "1280x1024 size", window_attr_twelve, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[2], "1600x1200 size", window_attr_sixteen, priv, GeomWindowMenu);
  //  AddMenuEntry("Double current size",window_attr_double);
  //  AddMenuEntry("Half current size",window_attr_half);
  
  
  AddMenuEntry(attr_menus[3], "Axes Color", window_axes_color, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[3], "Toggle Axes", window_axes, priv, GeomWindowMenu);
  GtkWidget *axis_menu = AddSubMenu(attr_menus[3], "Axes Placement");
  AddMenuEntry(axis_menu, "One per window", window_winaxes, priv, GeomWindowMenu);
  AddMenuEntry(axis_menu, "One per mesh", window_meshaxes, priv, GeomWindowMenu);
  
  AddMenuEntry(attr_menus[4], "Small Font Size", window_small_font, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[4], "Medium Font Size", window_med_font, priv, GeomWindowMenu);
  AddMenuEntry(attr_menus[4], "Large Font Size", window_large_font, priv, GeomWindowMenu);
  
}

void MenuGroup::setActive(unsigned index)
{
  menulock = true;
  for (unsigned i = 0; i < items.size(); i++) {
    bool val = false;
    if ( i == index)
      val = true;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(items[i]), val);
  }
  menulock = false;
}

void MenuGroup::setValue(unsigned index, bool val)
{
  menulock = true;
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(items[index]), val);
  menulock = false;
}
