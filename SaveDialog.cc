#include "SaveDialog.h"
#include "MeshList.h"
#include "Map3d_Geom.h"
#include "WindowManager.h"
#include "map3d-struct.h"
#include "savescreen.h"
#include "savestate.h"
#include <gtk/gtk.h>

SaveDialog* savedialog = NULL;
extern FilesDialog* filedialog;
extern Map3d_Info map3d_info;
extern FilePicker *fp;

void SaveGeoms(gpointer data)
{
  bool together = !gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(savedialog->save_separate)->check_button.toggle_button);
  
  GtkTable* table = GTK_TABLE(data);

  // gtk_table_attach stores table elements in a linked list
  // where each attached element is stored at the beginning of
  // the list.  Let's start at the end, and bypass the header
  // row, and then get the data we need.
  GList *list = g_list_last(table->children);
  list = g_list_nth_prev(list, 4); // there are 4 header cols
  
  Mesh_List ml;
  vector<bool> transforms;
  char *filename = 0;
  
  for (unsigned i = 0; i < filedialog->rowData.size(); i++) {
    FilesDialogRowData* rowdata = filedialog->rowData[i];
    GtkWidget* surfnum = rowdata->surfnum;
    GtkWidget* geomname = rowdata->save_filename;
    GtkWidget* s1 = rowdata->save_this_surface;
    GtkWidget* s2 = rowdata->save_with_transforms;
    
    int surf = atoi(gtk_entry_get_text(GTK_ENTRY(surfnum)));
    char* geom = (char*)gtk_entry_get_text(GTK_ENTRY(geomname));
    bool save =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s1)) != 0;
    bool savetransform =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s2)) != 0;
    
    //printf ("Save: %d, %s, %d %d\n", surf, geom, save?1:0, savetransform?1:0);
    if (!save) 
      continue;
    
    if (!together) {
      ml.clear();
      transforms.clear();
    }

    if (!filename || !together)
      filename = geom;

    Mesh_Info* mesh;
    bool mesh_found = false;
    for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
      mesh = mi.getMesh();
      if (mesh->geom->surfnum == surf) {
        mesh_found = true;
    
        // this will also save the subsurfs together, even if together is
        // not checked, which is what we want
        ml.push_back(mesh);
        transforms.push_back(savetransform);
      }
    }

    if (!together)
      SaveMeshes(ml,transforms, filename);
    if (!mesh_found) {
      printf("Warning - Mesh not found for surface %d\n", surf);
    }
  }
  if (together)
    SaveMeshes(ml, transforms, filename);
}

void destroySaveDialog(SaveDialog *sd)
{
  gtk_table_resize(GTK_TABLE(sd->table),1,5);
  sd->destroyed = true;
  gtk_widget_hide(sd->window);
}

void saveDialogCreate(bool show /*= true*/)
{
  GtkWidget* table;
  // table for geometries to save
  int NumRows = 1;
  int NumCols = 5;
  
  if (!savedialog) {
    savedialog = new SaveDialog;
  }
  else if (show) {
    gtk_widget_show_all(savedialog->window);
    return;
  }
  else
    return;
  
  GtkWidget *vbox;
  savedialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  vbox = gtk_vbox_new(FALSE, 5);
  
  gtk_window_set_title(GTK_WINDOW(savedialog->window), "Save...");
  gtk_window_set_resizable(GTK_WINDOW(savedialog->window), false);
  gtk_widget_set_sensitive(savedialog->window, true);
  if (fp && fp->active)
    gtk_widget_set_sensitive(savedialog->window, false);
  g_signal_connect_swapped(G_OBJECT(savedialog->window), "delete_event", G_CALLBACK(destroySaveDialog),
                            gpointer(savedialog));
  
  gtk_container_add(GTK_CONTAINER(savedialog->window), vbox);
  
  savedialog->destroyed = false;
  
  // add save sections - first column will be save geometry column
  GtkWidget* hbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
  
  GtkWidget *geometry_frame = gtk_frame_new("Geometry");
  gtk_container_set_border_width(GTK_CONTAINER(geometry_frame), 10);
  
  gtk_box_pack_start(GTK_BOX(hbox), geometry_frame, FALSE, FALSE, 2);
  
  GtkWidget* geometry_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(geometry_frame), geometry_vbox);
  
  table = gtk_table_new(NumRows, NumCols, FALSE);
  gtk_box_pack_start(GTK_BOX(geometry_vbox), table, FALSE, FALSE, 2);
  gtk_widget_show(table);
  savedialog->table = table;

  //set header row
  GtkWidget* surfnumtitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(surfnumtitle), "Surf"); 
  gtk_editable_set_editable(GTK_EDITABLE(surfnumtitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(surfnumtitle), 6);
  gtk_table_attach_defaults(GTK_TABLE(table), surfnumtitle, 0, 1, 0,1);
  gtk_widget_show(surfnumtitle);
  
  GtkWidget* geominputnametitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geominputnametitle), "Input File"); 
  gtk_editable_set_editable(GTK_EDITABLE(geominputnametitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(geominputnametitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), geominputnametitle, 1, 2, 0,1);
  gtk_widget_show(geominputnametitle);
  
  GtkWidget* geomnametitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geomnametitle), "Output File"); 
  gtk_editable_set_editable(GTK_EDITABLE(geomnametitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(geomnametitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), geomnametitle, 2, 4, 0,1);
  gtk_widget_show(geomnametitle);

  GtkWidget* savetitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(savetitle), "Save?"); 
  gtk_editable_set_editable(GTK_EDITABLE(savetitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(savetitle), 7);
  gtk_table_attach_defaults(GTK_TABLE(table), savetitle, 4, 5, 0,1);
  gtk_widget_show(savetitle);
  
  GtkWidget* savetranstitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(savetranstitle), "Transform?"); 
  gtk_editable_set_editable(GTK_EDITABLE(savetranstitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(savetranstitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), savetranstitle, 5, 6, 0,1);
  gtk_widget_show(savetranstitle);
  
  // fill the table in fileDialogCreate - that way when we add new surfaces we can
  // update the surfaces in the table as well.  Make sure the file window is created
  // to copy from
  filesDialogCreate(false);
  
  GtkWidget* geom_options_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(geometry_vbox), geom_options_hbox, FALSE, FALSE, 2);

  GtkWidget* geom_separate = gtk_radio_button_new_with_label((NULL),
                                                                "Save separately");
  GtkWidget* geom_together = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(geom_separate)),
                                                              "Save in 1 multi-surf file");
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(geom_separate), TRUE);
  gtk_box_pack_start(GTK_BOX(geom_options_hbox), geom_separate, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(geom_options_hbox), geom_together, FALSE, TRUE, 0);

  savedialog->save_separate = geom_separate;
  savedialog->save_together = geom_together;

  g_signal_connect_swapped(G_OBJECT(geom_separate), "clicked", G_CALLBACK(saveDialogGeomsSeparate), savedialog);
  g_signal_connect_swapped(G_OBJECT(geom_together), "clicked", G_CALLBACK(saveDialogGeomsTogether), savedialog);
  GtkWidget *save_button;
  save_button = gtk_button_new_with_label("Save");
  g_signal_connect_swapped(G_OBJECT(save_button), "clicked", G_CALLBACK(SaveGeoms), table);
  gtk_box_pack_end(GTK_BOX(geom_options_hbox), save_button, FALSE, FALSE, 2);
  
  //   --- IMAGE ---
  
  GtkWidget *image_frame = gtk_frame_new("Image");
  gtk_container_set_border_width(GTK_CONTAINER(image_frame), 10);
  
  gtk_box_pack_start(GTK_BOX(hbox), image_frame, FALSE, FALSE, 2);
  
  GtkWidget* image_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(image_frame), image_vbox);
  
  GtkWidget* image_filename_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(image_vbox), image_filename_hbox, FALSE, FALSE, 2);
  
  GtkWidget* imagename = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(imagename), map3d_info.imagefile); 
  gtk_editable_set_editable(GTK_EDITABLE(imagename), true);
  gtk_entry_set_width_chars(GTK_ENTRY(imagename), 30);
  gtk_box_pack_start(GTK_BOX(image_filename_hbox), imagename, FALSE, FALSE, 2);
  gtk_widget_show(imagename);
  savedialog->image_filename = imagename;
  
  GtkWidget* imagechange = gtk_button_new_with_label("...");
  gtk_widget_show(imagechange);
  gtk_box_pack_start(GTK_BOX(image_filename_hbox), imagechange, FALSE, FALSE, 2);
  g_signal_connect_swapped(G_OBJECT(imagechange), "clicked", G_CALLBACK(PickFileWidget), imagename);
  // add it to the list of file picker widgets so we can remember the directory
  fp->assignEntry(imagename, fp->file_selector_save);
  
  GtkWidget* save_image_res_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(image_vbox), save_image_res_hbox, FALSE, FALSE, 2);
  GtkWidget* save_image_res = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_image_res), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_image_res), false);
  gtk_box_pack_start(GTK_BOX(save_image_res_hbox), save_image_res, FALSE, FALSE, 2);
  GtkWidget* save_image_default_res_label = gtk_label_new("Use Image Size:");
  gtk_box_pack_start(GTK_BOX(save_image_res_hbox), save_image_default_res_label, FALSE, FALSE, 2);
  
  savedialog->image_default_res = save_image_res;
  g_signal_connect_swapped(G_OBJECT(save_image_res), "clicked", G_CALLBACK(saveDialogImageResolution),
                            savedialog);
  
  GtkAdjustment* image_width_adj = (GtkAdjustment*)
    gtk_adjustment_new(640,20,map3d_info.screenWidth,5,20,20);
  GtkAdjustment* image_height_adj = (GtkAdjustment*)
    gtk_adjustment_new(480,20,map3d_info.screenHeight,5,20,20);
  GtkWidget* save_image_res_width = gtk_spin_button_new(image_width_adj,1,0); 
  gtk_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(save_image_res_width)->entry), 5);
  gtk_box_pack_start(GTK_BOX(save_image_res_hbox), save_image_res_width, FALSE, FALSE, 2);
  GtkWidget* save_image_res_label = gtk_label_new("X");
  gtk_box_pack_start(GTK_BOX(save_image_res_hbox), save_image_res_label, FALSE, FALSE, 2);
  gtk_widget_set_sensitive(save_image_res_width, false);
  gtk_widget_set_sensitive(save_image_res_label, false);
  
  savedialog->image_res_width = save_image_res_width;
  savedialog->image_res_label = save_image_res_label;
  
  GtkWidget* save_image_res_height = gtk_spin_button_new(image_height_adj,1,0); 
  gtk_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(save_image_res_height)->entry), 5);
  gtk_box_pack_start(GTK_BOX(save_image_res_hbox), save_image_res_height, FALSE, FALSE, 2);
  gtk_widget_set_sensitive(save_image_res_height, false);
  
  savedialog->image_res_height = save_image_res_height;
  
  // put the button in a box so it doesn't expand the entire column
  GtkWidget* image_save_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(image_vbox), image_save_hbox, FALSE, FALSE, 2);
  GtkWidget* imagesave = gtk_button_new_with_label("Save Image");
  g_signal_connect_swapped(G_OBJECT(imagesave), "clicked", G_CALLBACK(SaveScreen), NULL);
  gtk_box_pack_end(GTK_BOX(image_save_hbox), imagesave, FALSE, FALSE, 2);
  
  
  //   --- ANIMATION SAVING ---
  
  GtkWidget *animation_frame = gtk_frame_new("Animation Control");
  gtk_container_set_border_width(GTK_CONTAINER(animation_frame), 10);
  
  gtk_box_pack_start(GTK_BOX(hbox), animation_frame, FALSE, FALSE, 2);
  
  
  GtkWidget* animation_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(animation_frame), animation_vbox);
  
  // Options group
  // for each checkbox, we have an hbox, checkbox and a label
  GtkWidget* save_anim_transform_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(animation_vbox), save_anim_transform_hbox, FALSE, FALSE, 2);
  GtkWidget* save_anim_transform = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_anim_transform), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_anim_transform), true);
  gtk_box_pack_start(GTK_BOX(save_anim_transform_hbox), save_anim_transform, FALSE, FALSE, 2);
  GtkWidget* save_anim_transform_label = gtk_label_new("Save Frame on Transformation");
  gtk_box_pack_start(GTK_BOX(save_anim_transform_hbox), save_anim_transform_label, FALSE, FALSE, 2);
  
  savedialog->animation_transformation = save_anim_transform;
  
  GtkWidget* save_anim_frame_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(animation_vbox), save_anim_frame_hbox, FALSE, FALSE, 2);
  GtkWidget* save_anim_frame = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_anim_frame), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_anim_frame), true);
  gtk_box_pack_start(GTK_BOX(save_anim_frame_hbox), save_anim_frame, FALSE, FALSE, 2);
  GtkWidget* save_anim_frame_label = gtk_label_new("Save Frame on Frame Advance");
  gtk_box_pack_start(GTK_BOX(save_anim_frame_hbox), save_anim_frame_label, FALSE, FALSE, 2);
  
  savedialog->animation_frame_advance = save_anim_frame;
  
  GtkWidget* save_anim_other_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(animation_vbox), save_anim_other_hbox, FALSE, FALSE, 2);
  GtkWidget* save_anim_other = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_anim_other), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_anim_other), true);
  gtk_box_pack_start(GTK_BOX(save_anim_other_hbox), save_anim_other, FALSE, FALSE, 2);
  GtkWidget* save_anim_other_label = gtk_label_new("Save Frame on Other Events");
  gtk_box_pack_start(GTK_BOX(save_anim_other_hbox), save_anim_other_label, FALSE, FALSE, 2);
  
  savedialog->animation_other_events = save_anim_other;
  
  GtkWidget* save_anim_idle_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(animation_vbox), save_anim_idle_hbox, FALSE, FALSE, 2);
  GtkWidget* save_anim_idle = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_anim_idle), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_anim_idle), false);
  gtk_box_pack_start(GTK_BOX(save_anim_idle_hbox), save_anim_idle, FALSE, FALSE, 2);
  GtkWidget* save_anim_idle_label = gtk_label_new("Save Frame Every ...");
  gtk_box_pack_start(GTK_BOX(save_anim_idle_hbox), save_anim_idle_label, FALSE, FALSE, 2);
  
  savedialog->animation_idle = save_anim_idle;
  g_signal_connect_swapped(G_OBJECT(save_anim_idle), "clicked", G_CALLBACK(saveDialogAnimationsIdle),
                            savedialog);
  
  GtkWidget* save_anim_idle_ms_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(animation_vbox), save_anim_idle_ms_hbox, FALSE, FALSE, 2);
  GtkAdjustment* idle_ms_adj = (GtkAdjustment*)
    gtk_adjustment_new(500,0,5000,20,100,100);
  GtkWidget* save_anim_idle_ms = gtk_spin_button_new(idle_ms_adj,1,0); 
  gtk_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(save_anim_idle_ms)->entry), 5);
  gtk_box_pack_start(GTK_BOX(save_anim_idle_ms_hbox), save_anim_idle_ms, FALSE, FALSE, 2);
  GtkWidget* save_anim_idle_ms_label = gtk_label_new("milliseconds");
  gtk_widget_set_sensitive(save_anim_idle_ms, false);
  gtk_widget_set_sensitive(save_anim_idle_ms_label, false);
  gtk_box_pack_start(GTK_BOX(save_anim_idle_ms_hbox), save_anim_idle_ms_label, FALSE, FALSE, 2);
  
  savedialog->animation_idle_ms = save_anim_idle_ms;
  savedialog->animation_idle_ms_label = save_anim_idle_ms_label;
  
  
  // put the button in a box so it doesn't expand the entire column
  GtkWidget* animation_save_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(animation_vbox), animation_save_hbox, FALSE, FALSE, 2);
  GtkWidget* animation_start = gtk_button_new_with_label("Start saving animation");
  g_signal_connect_swapped(G_OBJECT(animation_start), "clicked", G_CALLBACK(saveDialogStartAnimations), savedialog);
  GtkWidget* animation_stop = gtk_button_new_with_label("Stop saving animation");
  g_signal_connect_swapped(G_OBJECT(animation_stop), "clicked", G_CALLBACK(saveDialogStopAnimations), savedialog);
  gtk_widget_set_sensitive(animation_stop, false);
  gtk_box_pack_end(GTK_BOX(animation_save_hbox), animation_stop, FALSE, FALSE, 2);
  gtk_box_pack_end(GTK_BOX(animation_save_hbox), animation_start, FALSE, FALSE, 2);
  
  savedialog->animation_start_button = animation_start;
  savedialog->animation_stop_button = animation_stop;
  
  
  //   --- STATE SAVING ---
  
  GtkWidget *settings_frame = gtk_frame_new("Settings");
  gtk_container_set_border_width(GTK_CONTAINER(settings_frame), 10);
  
  gtk_box_pack_start(GTK_BOX(hbox), settings_frame, FALSE, FALSE, 2);
  
  
  GtkWidget* settings_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(settings_frame), settings_vbox);
  
  GtkWidget* settings_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_vbox), settings_hbox, FALSE, FALSE, 2);
  
  GtkWidget* settings_type_vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_hbox), settings_type_vbox, FALSE, FALSE, 2);
  
  GtkWidget* settings_options_vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_hbox), settings_options_vbox, FALSE, FALSE, 2);
  
  // Options group
  // for each checkbox, we have an hbox, checkbox and a label
  GtkWidget* save_global_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_options_vbox), save_global_hbox, FALSE, FALSE, 2);
  GtkWidget* save_global_options = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_global_options), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_global_options), true);
  gtk_box_pack_start(GTK_BOX(save_global_hbox), save_global_options, FALSE, FALSE, 2);
  GtkWidget* save_global_label = gtk_label_new("Save Global Options");
  gtk_box_pack_start(GTK_BOX(save_global_hbox), save_global_label, FALSE, FALSE, 2);
  
  savedialog->settings_global = save_global_options;
  
  GtkWidget* save_transform_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_options_vbox), save_transform_hbox, FALSE, FALSE, 2);
  GtkWidget* save_mesh_transformations = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_mesh_transformations), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_mesh_transformations), true);
  gtk_box_pack_start(GTK_BOX(save_transform_hbox), save_mesh_transformations, FALSE, FALSE, 2);
  GtkWidget* save_transform_label = gtk_label_new("Save Mesh Transformations");
  gtk_box_pack_start(GTK_BOX(save_transform_hbox), save_transform_label, FALSE, FALSE, 2);
  
  savedialog->settings_transform = save_mesh_transformations;
  
  GtkWidget* save_meshoptions_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_options_vbox), save_meshoptions_hbox, FALSE, FALSE, 2);
  GtkWidget* save_mesh_options = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(save_mesh_options), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(save_mesh_options), true);
  gtk_box_pack_start(GTK_BOX(save_meshoptions_hbox), save_mesh_options, FALSE, FALSE, 2);
  GtkWidget* save_meshoptions_label = gtk_label_new("Save Mesh Options");
  gtk_box_pack_start(GTK_BOX(save_meshoptions_hbox), save_meshoptions_label, FALSE, FALSE, 2);
  
  savedialog->settings_meshoptions = save_mesh_options;
  
  // File Type group
  
  GtkWidget* settings_map3drc = gtk_radio_button_new_with_label((NULL),
                                                                "Save .map3drc file");
  GtkWidget* settings_batch = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(settings_map3drc)),
                                                              "Save Windows batch file");
  GtkWidget* settings_script = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(settings_map3drc)),
                                                                "Save UNIX shell script");
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(settings_map3drc), TRUE);
  gtk_box_pack_start(GTK_BOX(settings_type_vbox), settings_map3drc, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(settings_type_vbox), settings_batch, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(settings_type_vbox), settings_script, FALSE, TRUE, 0);
  
  g_signal_connect_swapped(G_OBJECT(settings_map3drc), "clicked", G_CALLBACK(saveDialogSettingsRadio),
                            savedialog);
  
  savedialog->settings_map3drc = settings_map3drc;
  savedialog->settings_batch = settings_batch;
  savedialog->settings_script = settings_script;
  
  GtkWidget* settings_file_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_type_vbox), settings_file_hbox, FALSE, FALSE, 2);
  
  GtkWidget* settingsname = gtk_entry_new(); 
#ifdef _WIN32
  gtk_entry_set_text(GTK_ENTRY(settingsname), "map3d.bat"); 
#else
  gtk_entry_set_text(GTK_ENTRY(settingsname), "map3d.sh"); 
#endif
  gtk_editable_set_editable(GTK_EDITABLE(settingsname), true);
  gtk_entry_set_width_chars(GTK_ENTRY(settingsname), 30);
  gtk_box_pack_start(GTK_BOX(settings_file_hbox), settingsname, FALSE, FALSE, 2);
  gtk_widget_set_sensitive(settingsname, false);
  gtk_widget_show(settingsname);
  savedialog->settings_filename = settingsname;
  
  GtkWidget* settingschange = gtk_button_new_with_label("...");
  gtk_widget_show(settingschange);
  gtk_box_pack_start(GTK_BOX(settings_file_hbox), settingschange, FALSE, FALSE, 2);
  g_signal_connect_swapped(G_OBJECT(settingschange), "clicked", G_CALLBACK(PickFileWidget), settingsname);
  savedialog->settings_change = settingschange;
  // add it to the list of file picker widgets so we can remember the directory
  fp->assignEntry(settingsname, fp->file_selector_save);
  
  // put the button in a box so it doesn't expand the entire column
  GtkWidget* settings_save_box = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(settings_vbox), settings_save_box, FALSE, FALSE, 2);
  GtkWidget* settingssave = gtk_button_new_with_label("Save Settings/Script");
  g_signal_connect_swapped(G_OBJECT(settingssave), "clicked", G_CALLBACK(SaveSettings), savedialog);
  gtk_box_pack_end(GTK_BOX(settings_save_box), settingssave, FALSE, FALSE, 2);
  
  
  // add cancel button
  GtkWidget* hbox2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 2);
  
  GtkWidget *cancel;
  cancel = gtk_button_new_with_label("Close");
  
  savedialog->close_button = cancel;
  
  g_signal_connect_swapped(G_OBJECT(cancel), "clicked", G_CALLBACK(destroySaveDialog), (gpointer)savedialog);
  
  gtk_box_pack_end(GTK_BOX(hbox2), cancel, FALSE, FALSE, 2);
  
  if (show)
    gtk_widget_show_all(savedialog->window);
}

void saveDialogSettingsRadio(SaveDialog* sd)
{
  if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(sd->settings_map3drc)->check_button.toggle_button)) {
    gtk_widget_set_sensitive(sd->settings_filename, false);
    gtk_widget_set_sensitive(sd->settings_change, false);
  }
  else {
    gtk_widget_set_sensitive(sd->settings_filename, true);
    gtk_widget_set_sensitive(sd->settings_change, true);
  }
}

void saveDialogGeomsTogether(SaveDialog* /*sd*/)
{
  // give the user a little indication that he only saves the first file
  for (unsigned i = 1; i < filedialog->rowData.size(); i++) {
    gtk_widget_set_sensitive(filedialog->rowData[i]->save_filename, false);
  }
}
void saveDialogGeomsSeparate(SaveDialog* /*sd*/)
{
  for (unsigned i = 1; i < filedialog->rowData.size(); i++) {
    gtk_widget_set_sensitive(filedialog->rowData[i]->save_filename, true);
  }
}


void saveDialogImageResolution(SaveDialog* sd)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sd->image_default_res))) {
    gtk_widget_set_sensitive(sd->image_res_height, true);
    gtk_widget_set_sensitive(sd->image_res_width, true);
    gtk_widget_set_sensitive(sd->image_res_label, true);
  }
  else {
    gtk_widget_set_sensitive(sd->image_res_height, false);
    gtk_widget_set_sensitive(sd->image_res_width, false);
    gtk_widget_set_sensitive(sd->image_res_label, false);
  }
}

void AnimationIdleLoop(SaveDialog* sd)
{
  unsigned long t;
  double time, time_frequency;
  
  // g_timer_elapsed returns a value in seconds, and ignores the parameter 
  // frequency is in ms
  time = g_timer_elapsed(sd->timer, &t);
  time = time - sd->animation_last_save_time;
  time_frequency = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sd->animation_idle_ms));
  
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sd->animation_idle)) && time * 1000 >= time_frequency) {
    SaveScreen();
  }
}

void saveDialogStartAnimations(SaveDialog* sd)
{
  gtk_widget_set_sensitive(sd->animation_stop_button, true);  
  gtk_widget_set_sensitive(sd->animation_start_button, false);  
  map3d_info.saving_animations = true;
  sd->timer = g_timer_new();
  sd->animation_last_save_time = 0;
  sd->animation_idle_loop = 
    gtk_idle_add_priority(GTK_PRIORITY_REDRAW,(GtkFunction) AnimationIdleLoop, sd);
}

void saveDialogStopAnimations(SaveDialog* sd)
{
  gtk_widget_set_sensitive(sd->animation_stop_button, false);  
  gtk_widget_set_sensitive(sd->animation_start_button, true);  
  map3d_info.saving_animations = false;
  gtk_idle_remove(sd->animation_idle_loop);
  g_timer_destroy(sd->timer);
}

void saveDialogAnimationsIdle(SaveDialog* sd)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sd->animation_idle))) {
    gtk_widget_set_sensitive(sd->animation_idle_ms, true);
    gtk_widget_set_sensitive(sd->animation_idle_ms_label, true);
  }
  else {
    gtk_widget_set_sensitive(sd->animation_idle_ms, false);
    gtk_widget_set_sensitive(sd->animation_idle_ms_label, false);
  }
  
}
