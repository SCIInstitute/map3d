#include "ContourDialog.h"
#include "Contour_Info.h"
#include "LegendWindow.h"
#include "MeshList.h"
#include "Surf_Data.h"
#include "WindowManager.h"
#include "eventdata.h"
#include <gtk/gtk.h>
#include <math.h>

ContourDialog *contourdialog = NULL;
extern FilesDialog* filedialog;
extern Map3d_Info map3d_info;

enum contTableCols{
  SurfNum, GeomName, Spacing, NumConts, DefaultRange, LowRange, HighRange, OccGrad, NumCols
};

// ------------------- //
//contour dialog callbacks, create, and accessor functions
void contourDialogCreate(bool show /*=true*/)
{
  if (!contourdialog){
    contourdialog = new ContourDialog;
  }
  else if (show) {
    gtk_widget_show(contourdialog->window);
    return;
  }
  else
    return;
  
  contourdialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  contourdialog->field_lock = false;

  gtk_window_set_title(GTK_WINDOW(contourdialog->window), "Contour Spacing");
  //  gtk_container_set_border_width (GTK_CONTAINER (map3d_info.window), 5);
  gtk_window_set_resizable(GTK_WINDOW(contourdialog->window), FALSE);
  
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_container_add(GTK_CONTAINER(contourdialog->window), vbox);
  gtk_widget_show(vbox);
  
  GtkWidget *hbox1 = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 2);
  GtkWidget* const_label = gtk_label_new("When range changes: ");
  GtkWidget* spacing_const = gtk_radio_button_new_with_label((NULL), "Keep spacing constant");
  GtkWidget* num_const = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(spacing_const)),
                                                         "Keep num contours constant");

  if (map3d_info.use_spacing) {
    contourdialog->orig_spacing_check = true;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spacing_const),TRUE);
  }
  else {
    contourdialog->orig_spacing_check = false;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(num_const),TRUE);
  }
  gtk_box_pack_start(GTK_BOX(hbox1), const_label, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox1), spacing_const, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox1), num_const, FALSE, FALSE, 2);
  contourdialog->lock_contspacing = spacing_const;
  contourdialog->lock_numconts = num_const;
    
  gtk_widget_show(hbox1);
  gtk_widget_show(const_label);
  gtk_widget_show(spacing_const);
  gtk_widget_show(num_const);
  
  GtkWidget* table = gtk_table_new(1, NumCols, FALSE);
  contourdialog->table = table;
  gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
  gtk_widget_show(table);
  
  GtkWidget* surfnumtitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(surfnumtitle), "Surf"); 
  gtk_editable_set_editable(GTK_EDITABLE(surfnumtitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(surfnumtitle), 6);
  gtk_table_attach_defaults(GTK_TABLE(table), surfnumtitle, SurfNum, SurfNum+1, 0,1);
  gtk_widget_show(surfnumtitle);
  
  GtkWidget* geomnametitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geomnametitle), "Geom File"); 
  gtk_editable_set_editable(GTK_EDITABLE(geomnametitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(geomnametitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), geomnametitle, GeomName, GeomName+1, 0,1);
  gtk_widget_show(geomnametitle);
  
  GtkWidget* spacing = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(spacing), "Spacing"); 
  gtk_editable_set_editable(GTK_EDITABLE(spacing), false);
  gtk_entry_set_width_chars(GTK_ENTRY(spacing), 10);
  gtk_table_attach_defaults(GTK_TABLE(table), spacing, Spacing, Spacing+1, 0,1);
  gtk_widget_show(spacing);
  
  GtkWidget* num_conts = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(num_conts), "Num Conts"); 
  gtk_editable_set_editable(GTK_EDITABLE(num_conts), false);
  gtk_entry_set_width_chars(GTK_ENTRY(num_conts), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), num_conts, NumConts, NumConts+1, 0,1);
  gtk_widget_show(num_conts);
  
  GtkWidget* default_range = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(default_range), "Default Range?"); 
  gtk_editable_set_editable(GTK_EDITABLE(default_range), false);
  gtk_entry_set_width_chars(GTK_ENTRY(default_range), 18);
  gtk_table_attach_defaults(GTK_TABLE(table), default_range, DefaultRange, DefaultRange+1, 0,1);
  gtk_widget_show(default_range);

  GtkWidget* range_low = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(range_low), "Low Range"); 
  gtk_editable_set_editable(GTK_EDITABLE(range_low), false);
  gtk_entry_set_width_chars(GTK_ENTRY(range_low), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), range_low, LowRange, LowRange+1, 0,1);
  gtk_widget_show(range_low);

  GtkWidget* range_high = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(range_high), "High Range"); 
  gtk_editable_set_editable(GTK_EDITABLE(range_high), false);
  gtk_entry_set_width_chars(GTK_ENTRY(range_high), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), range_high, HighRange, HighRange+1, 0,1);
  gtk_widget_show(range_high);
  
  GtkWidget* occ_grad = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(occ_grad), "Occlusion Grad"); 
  gtk_editable_set_editable(GTK_EDITABLE(occ_grad), false);
  gtk_entry_set_width_chars(GTK_ENTRY(occ_grad), 16);
  gtk_table_attach_defaults(GTK_TABLE(table), occ_grad, OccGrad, OccGrad+1, 0,1);
  gtk_widget_show(occ_grad);
  
  
  
  // fill the table in fileDialogCreate - that way when we add new surfaces we can
  // update the surfaces in the table as well.  Make sure the file window is created
  // to copy from
  filesDialogCreate(false);
  
  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, TRUE, 0);
  //  gtk_container_add(GTK_CONTAINER(contourdialog->window), notebook);
  gtk_widget_show(notebook);
  
  //////////////////////////////////////////////////////////////
  // Create the horizontal box on the bottom of the dialog
  //     with the "Reset" and "Close" buttons.
  GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  
  GtkWidget *preview, *cancel, *okay;
  preview = gtk_button_new_with_label("Preview");
  cancel = gtk_button_new_with_label("Cancel");
  okay = gtk_button_new_with_label("OK");
  
  gtk_box_pack_start(GTK_BOX(hbox), preview, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), cancel, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), okay, TRUE, TRUE, 0);
  
  gtk_widget_show_all(hbox);
  
  ////
  //////////////////////////////////////////////////////////////
  
  
  g_signal_connect(G_OBJECT(cancel), "clicked", G_CALLBACK(ContourCancel), NULL);
  g_signal_connect(G_OBJECT(okay), "clicked", G_CALLBACK(ContourOkay), NULL);
  g_signal_connect_swapped(G_OBJECT(preview), "clicked", G_CALLBACK(ContourPreview), false);
    
  g_signal_connect(G_OBJECT(contourdialog->window), "delete_event", G_CALLBACK(contourdestroycallback), NULL);
  
  if (show)
    gtk_widget_show(contourdialog->window);
  
}

void ContourCancel(){
  contourdialog->field_lock = true;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(contourdialog->lock_contspacing)) != contourdialog->orig_spacing_check) {
    if (contourdialog->orig_spacing_check) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(contourdialog->lock_contspacing),TRUE);
    }
    else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(contourdialog->lock_numconts),TRUE);
    }
  }

  for (unsigned i = 0; i < filedialog->rowData.size(); i++) {
    FilesDialogRowData* rowdata = filedialog->rowData[i];
    
    if(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->numcontours)) != rowdata->orig_numconts){
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours), rowdata->orig_numconts);
    }
    if(gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing)) != rowdata->orig_numspaces) {
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing), rowdata->orig_numspaces);
    }
    if((gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_low_range)) != rowdata->orig_lowrange)||
       (gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_high_range)) != rowdata->orig_highrange)||
       (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range)) != rowdata->orig_defaultrange)){
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range), rowdata->orig_lowrange);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range), rowdata->orig_highrange);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range), rowdata->orig_defaultrange);
    }  
    contourcallback(rowdata);
  }
  Broadcast(MAP3D_UPDATE, 0);
  contourdestroycallback();
  contourdialog->field_lock = false;
}


void ContourPreview(bool okay){
  if (okay) 
    contourdialog->orig_spacing_check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(contourdialog->lock_contspacing));

  for (unsigned i = 0; i < filedialog->rowData.size(); i++) {
    FilesDialogRowData* rowdata = filedialog->rowData[i];
    
    contourcallback(rowdata);

    if(okay){
      rowdata->orig_numconts = (float) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->numcontours));
      rowdata->orig_numspaces = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
      rowdata->orig_lowrange = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_low_range));
      rowdata->orig_highrange = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_high_range));
      rowdata->orig_defaultrange = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range));
    }

    
  }
  Broadcast(MAP3D_UPDATE, 0);
}

void ContourOkay(){
  ContourPreview(true);
  contourdestroycallback();
}

// called from cancel or preview
void contourcallback(FilesDialogRowData* rowdata)
{
  int numconts;
  map3d_info.use_spacing = (bool) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(contourdialog->lock_contspacing));

  Mesh_List meshes = rowdata->mesh->gpriv->findMeshesFromSameInput(rowdata->mesh);
  for (unsigned i = 0; i < meshes.size(); i++) {

    Mesh_Info* mesh = meshes[i];

    if (mesh->data) {
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range)))
        mesh->data->user_scaling = false;
      else
        mesh->data->user_scaling = true;

      mesh->data->userpotmin = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_low_range));
      mesh->data->userpotmax = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_high_range));
    }


    if (map3d_info.use_spacing) {
      mesh->contourspacing = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
      mesh->gpriv->cont_num.setActive(0); // check in user-specified contour menu
      numconts = mesh->cont->buildContours();
    }
    else {
      int numconts = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->numcontours));
      if (mesh->data)
        mesh->data->numconts = numconts;
    }
    mesh->cont->occlusion_gradient = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_occlusion_gradient));
  }
}



void contourdestroycallback()
{
  gtk_widget_hide(contourdialog->window);
}

void updateContourDialogValues(Mesh_Info* mesh)
{
  if (!contourdialog)
    return;
  float min,max;
  int numconts;
  double spaces;
  double numspaces;
  
  contourdialog->field_lock = true;

  FilesDialogRowData* rowdata;
  if(!filedialog){
    return;
  }

  // set the value back if you haven't previewed it yet (to avoid confusion)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(contourdialog->lock_contspacing),map3d_info.use_spacing);


  for (unsigned i = 0; i < filedialog->rowData.size(); i++) {
    rowdata = filedialog->rowData[i];
    if (rowdata->mesh == mesh){
      if (mesh->data) {
        mesh->data->get_minmax(min,max);
        float spacing = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
        if (spacing == 0.f && mesh->data) {
          // initialize if we set up the mesh's first data after the mesh has been opened.
          gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->contourspacing), .01, mesh->data->potmax-mesh->data->potmin);
          gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->cont_occlusion_gradient), 0, mesh->data->potmax-mesh->data->potmin);
          rowdata->orig_numspaces  = (max - min)/float(mesh->data->numconts+1);
          rowdata->orig_numconts = (float) mesh->data->numconts;

          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),rowdata->orig_numspaces);
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),rowdata->orig_numconts);
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_occlusion_gradient),mesh->data->potmax-mesh->data->potmin);
          continue;
        }
        if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_contspacing)->check_button.toggle_button)){
          // update num conts (round up)
          float nc = (max-min)/
            (gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing)))-1;
          numconts = (int) nc;
          if (nc - (int) nc > 0)
            numconts = (int) nc+1;
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),numconts);
          
        }
        else if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_numconts)->check_button.toggle_button)) {
          // update spacing 
          spaces = (max - min)/float(mesh->data->numconts+1);
          numspaces = spaces;
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),numspaces);
        }
      }
    }
  }   
  contourdialog->field_lock = false;
}

void cont_get_minmax(FilesDialogRowData* rowdata, float& min, float& max)
{
  // the value of mesh->data->user_scaling doesn't change until you
  // click preview/okay, so to update the values of the contour dialog
  // correctly if the default button is checked, then we temporaily set
  // user_scaling
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    if (rowdata->mesh->data) {
      bool tempval = rowdata->mesh->data->user_scaling;
      rowdata->mesh->data->user_scaling = false;
      rowdata->mesh->data->get_minmax(min, max);
      rowdata->mesh->data->user_scaling = tempval;
    }
    else {
      min = 0.0f;
      max = 0.0f;
    }
  }
  else {
    max = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_high_range));
    min = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_low_range));
    if (min == 0.0f && max == 0.0f) {
      // set it to the current min/max
      // rather than just setting it to 0
      contourdialog->field_lock = true;
      bool tempval = rowdata->mesh->data->user_scaling;
      rowdata->mesh->data->user_scaling = false;
      rowdata->mesh->data->get_minmax(min, max);
      rowdata->mesh->data->user_scaling = tempval;
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range), max);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range), min);
      contourdialog->field_lock = false;
    }
  }
}

void modifyContourDialogRow_RangeChange(FilesDialogRowData* rowdata)
{
  float min,max;
  float numconts;
  double spaces;
  double numspaces;
  
  if (contourdialog->field_lock)
    return;
  contourdialog->field_lock = true;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    gtk_widget_set_sensitive(rowdata->cont_high_range, false);
    gtk_widget_set_sensitive(rowdata->cont_low_range, false);
  }
  else {
    gtk_widget_set_sensitive(rowdata->cont_high_range, true);
    gtk_widget_set_sensitive(rowdata->cont_low_range, true);
  }

  cont_get_minmax(rowdata, min, max);

  if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_contspacing)->check_button.toggle_button)){
    // update num conts if range changed (round up)
    float nc = (max-min)/
      (gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing)))-1;
    numconts = nc;
    if (nc - (int) nc > 0)
      numconts = nc+1;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),numconts);
  }
  else if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_numconts)->check_button.toggle_button)) {    
    // update spacing if the range changed
    spaces = (max - min)/float(gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->numcontours))+1);
    numspaces = spaces;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),numspaces);
  }
  contourdialog->field_lock = false;

}

void modifyContourDialogRow_ContSpaceChange(FilesDialogRowData* rowdata)
{
  float min,max;
  int numconts;
  
  if (contourdialog->field_lock)
    return;
  contourdialog->field_lock = true;

  cont_get_minmax(rowdata, min, max);

  float spacing = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    // default range: update the num conts if the contour spacing changed (round up)
    float nc = (max-min)/spacing - 1;
    numconts = nc;
    if (nc - (int) nc > 0)
      numconts = (int) nc+1;
  }
  else {
    // user range: change the range
    numconts = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->numcontours));
    float range = (numconts+1)*spacing;
    float range_ratio = range / (max-min);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range), max*range_ratio);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range), min*range_ratio);
  }
  if (numconts > MAX_CONTOURS) {
    numconts = MAX_CONTOURS;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),(max-min)/(numconts+1));
  }
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),numconts);
  contourdialog->field_lock = false;
}


void modifyContourDialogRow_NumContChange(FilesDialogRowData* rowdata)
{
  float min,max;
  double spaces;
  
  if (contourdialog->field_lock)
    return;
  contourdialog->field_lock = true;
  cont_get_minmax(rowdata, min, max);

  int numconts = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->numcontours));
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    // update spacing if the num conts changed

    // round down at the end to make sure we get the number of conts we ask for
    spaces = (max - min)/float(numconts+1) - .005;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),spaces);
  }
  else {
    // user range: change the range
    float spacing = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
    float range = (numconts+1)*spacing;
    float range_ratio = range / (max-min);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range), max*range_ratio);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range), min*range_ratio);
  }

  contourdialog->field_lock = false;
}
