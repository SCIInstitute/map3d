/* GeomWindowMenu.h */

#ifndef GEOMWINDOWMENU_H
#define GEOMWINDOWMENU_H

#include <vector>

using std::vector;
typedef struct _GtkWidget GtkWidget;

// a menu group will be a list of Menu items, 
//   we can thus do things like have a set of check 
//     items where only 1 may be active at once
//   or have a dynamically growing menu.
struct MenuGroup {
  vector<GtkWidget*> items;
  inline void add(GtkWidget* item) {items.push_back(item);}

  // sets the assigned check item at index to true, and the rest to false
  void setActive(unsigned index);

  // sets the assigned check item at index to val, and the rest are unchanged.
  void setValue(unsigned index, bool val);
};

class GeomWindow;
struct menu_data;
void GeomBuildMenus(GeomWindow * priv);
void GeomWindowHandleMenu(menu_data * data);

// look for a global scaling option, and return whether it found one or not
bool GeomWindowMenuGlobalOptions(menu_data * data);

enum menuitem
{
  //  NOTE TO THOSE WHO WOULD EDIT THIS FILE //
  //    You must add your entry to not only this file, but to regressionlist.h
  //    if you wish the regression tester to remain stable and sane.
  //    It must be done in the correct order.
  surface_color_rainbow = 1, surface_color_red2green, surface_color_grayscale, surface_color_jet, 
  surface_color_invert, surface_render_none, surface_render_flat,
  surface_render_gouraud, surface_render_banded,
  scaling_global_frame, scaling_global_surface, scaling_dialog,
  scaling_global_global, scaling_local, scaling_function_linear,
  scaling_local_group, scaling_global_group, scaling_local_slave,
  scaling_global_slave, scaling_command_line,
  scaling_function_exponential, scaling_function_logarithmic,
  scaling_function_lab, scaling_function_lab13,
  scaling_mapping_symmetric, scaling_mapping_true, scaling_mapping_separate, scaling_mapping_midpoint,
  scaling_group_one, scaling_group_two, scaling_group_three, scaling_group_four,
  mesh_size, mesh_render_elements, mesh_select_contsize, mesh_select_meshsize,
  mesh_select_marksize, mesh_select_extremasize, mesh_select_tssize,
  mesh_select_leadsize, mesh_select_picksize, mesh_select_transsize, mesh_select_scalesize,
  mesh_select_rotsize, mesh_select_large_size, mesh_select_med_size, mesh_select_small_size, 
  mesh_render_none, mesh_render_connectivity, mesh_render_points, mesh_color, mesh_toggle, mesh_render_points_connectivity,
  mesh_render_elements_connectivity, mesh_render_nondata_elements, secondary_color, mesh_disp_lwindow, 
  mesh_geom_reload, mesh_data_reload, mesh_both_reload, mesh_files_dialog,
  contour_spacing_user, contour_number, contour_dialog, contour_number_5, contour_number_10,
  contour_number_15, contour_number_20, contour_number_25, contour_number_30,
  contour_number_35, contour_number_40, contour_number_45, contour_number_50,
  contour_style_dashed, contour_style_solid, contour_size, contour_toggle,
  pick_timesignal_single, pick_timesignal_multiple, pick_nodeinfo, pick_triinfo,
  pick_triangulate, pick_flip, pick_edit_node, pick_edit_lm, pick_del_node, mark_all_sphere,
  pick_aperture, pick_tri_node_mark, pick_info_display, pick_show_all, pick_hide_all, pick_reference,
  pick_mean_reference,pick_clear_reference, mark_all_sphere_value,
  mark_all_node, mark_all_channel, mark_all_value, mark_all_fid, mark_all_color,
  mark_all_size, mark_all_clear, mark_extrema_sphere, mark_extrema_node,
  mark_extrema_channel, mark_extrema_value, mark_extrema_color,
  mark_extrema_size, mark_extrema_clear, mark_ts_sphere, mark_ts_node,
  mark_ts_channel, mark_ts_value, mark_ts_color, mark_ts_size, mark_ts_clear,
  mark_lead_sphere, mark_lead_node, mark_lead_labels,
  mark_lead_channel, mark_lead_value, mark_lead_color, mark_lead_size, mark_lead_clear,
  mark_toggle, graphics_light_above, graphics_light_below, graphics_light_left, graphics_light_right, 
  graphics_light_front, graphics_light_back, graphics_light_none, graphics_fog, graphics_fog_adjust,graphics_cont_smoothing,
  landmark_showcor, landmark_wirecor, landmark_corcolor,
  landmark_showcath, landmark_cathcolor,
  landmark_showocclus, landmark_showstitch, landmark_showstim,
  landmark_showlead, landmark_occluscolor, landmark_stitchcolor,
  landmark_stimcolor, landmark_leadcolor,
  landmark_showplane, landmark_planecolor, landmark_transplane,
  landmark_showrod, landmark_showpaceneedle, landmark_showrecneedle,
  landmark_showfiber, landmark_rodcolor, landmark_paceneedlecolor,
  landmark_recneedlecolor, landmark_fibercolor, landmark_showcannula,
  landmark_cannulacolor, landmark_toggleall, landmark_togglelabels, landmark_togglepoints, landmark_togglerods,
  window_axes, window_axes_color, window_attr_bg, window_attr_fg,
  window_attr_four, window_attr_six, window_attr_eight, window_attr_ten,
  window_attr_twelve, window_attr_sixteen,window_small_font, window_med_font, window_large_font,
//  window_attr_double, window_attr_half,
  window_attr_info_on, window_attr_info_off, window_locks, save_image, save_map3drc, save_script, save_batch, window_save,
  window_save_transform, window_lwindow_show, window_lwindow_hide,
  window_meshaxes, window_winaxes, clip_front, clip_back, clip_together, clip_with_object,
  frame_lock, trans_lock, frame_reset, frame_dialog, frame_step_1, frame_step_2, frame_step_4,
  frame_step_5, frame_step_10, frame_step_45, frame_step_90, frame_step_user, frame_loop,
  frame_align, frame_zero, screen_save,
  fid_dialog, fid_map_dialog,no_fid_cont, act_fid_cont, rec_fid_cont, no_fid_map, act_fid_map, rec_fid_map, fid_map_shade_toggle, fid_draw_fid
};

// these macro is to align toggleable features together
  // if they were unaligned while the global lock was off.
#define MAP3D_MESH_LOCK_TOGGLE(lockname, var) \
  if (!lockname || mesh == getFirstMesh()) mesh->var = !mesh->var; \
  else mesh->var = getFirstMesh()->var;  

#define MAP3D_WINDOW_LOCK_TOGGLE(lockname, var) \
  if (!lockname || this == GetGeomWindow(0)) this->var = !this->var; \
  else this->var = GetGeomWindow(0)->var;  


#endif
