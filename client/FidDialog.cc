#include "FidDialog.h"
#include "Contour_Info.h"
#include "MeshList.h"
#include "Surf_Data.h"
#include "Map3d_Geom.h"
#include "WindowManager.h"
#include "eventdata.h"
#include <math.h>

#include <QCheckBox>
#include <QRadioButton>

enum fidTableCols{
  Fiducial, DrawMap, DrawContour, ContColor, ContSize, NumCols
};

// ------------------- //
//fid dialog callbacks, create, and accessor functions
FidDialog::FidDialog(QWidget* parent) : QDialog(parent)
{
  setupUi(this);

  for (MeshIterator mi(0,0); !mi.isDone(); ++mi)
  {
    surfComboBox->addItem(QString::number(mi.getMesh()->geom->surfnum));
  }

  on_surfComboBox_currentIndexChanged(0);
}

void FidDialog::on_surfComboBox_currentIndexChanged(int index)
{
  // clear
  dataLabel->setText("No Data");
  spacingSpinBox->setValue(1);

  foreach (QLabel* widget, _fiducialLabels)
    widget->deleteLater();
  foreach (ColorWidget* widget, _fiducialColors)
    widget->deleteLater();
  foreach (QSpinBox* widget, _fiducialSizes)
    widget->deleteLater();
  foreach (QCheckBox* widget, _fiducialContourCheckBox)
    widget->deleteLater();
  foreach (QRadioButton* widget, _fiducialMapRadio)
    widget->deleteLater();

  // reset
  int i = 0;
  MeshIterator mi(0,0); 
  for (; !mi.isDone() && i < index; ++mi, i++)
  {
    // just iterate until we get to index
  }

  if (!mi.isDone())
  {
    _currentMesh = mi.getMesh();
    if (_currentMesh->data)
    {
      Surf_Data* data = _currentMesh->data;
      dataLabel->setText(_currentMesh->mysurf->potfilename);

      if (data->numfs == 0)
      {
        QLabel* label = new QLabel("No fiducials", fidFrame);
        _fiducialLabels.append(label);
        gridLayout->addWidget(label, 1, Fiducial, 1, 1);
      }
      else
      {
        int selectedMap = 0;
        if (_currentMesh->fidmapindex > 0)
          selectedMap = _currentMesh->fidmapindex - 1;

        int row = 1;
        for (int i = 0; i < data->numfs; i++)
        {
          for(int j = 0; j < data->fids[i].numfidtypes; j++)
          {
            QLabel* label = new QLabel(QString(data->fids[i].fidnames[j]) + " " + data->fids[i].fidlabel, fidFrame);
            _fiducialLabels.append(label);
            gridLayout->addWidget(label, row, Fiducial, 1, 1);

            ColorWidget* cw = new ColorWidget(this, QColor(_currentMesh->fidConts[row-1]->fidcolor));
            gridLayout->addWidget(cw, row, ContColor, 1, 1);
			connect(cw, SIGNAL(doubleClicked()), this, SLOT(pickColor()));

            SizeWidget* sw = new SizeWidget(this, _currentMesh->fidConts[row-1]->fidContSize);
            gridLayout->addWidget(sw, row, ContSize, 1, 1);
			connect(sw, SIGNAL(doubleClicked()), this, SLOT(pickSize()));

            QRadioButton* rb = new QRadioButton(this);
            _fiducialMapRadio.append(rb);
            gridLayout->addWidget(rb, row, DrawMap, 1, 1);

            if (row-1 == selectedMap)
              rb->setChecked(true);

            QCheckBox* cb = new QCheckBox(this);
            _fiducialContourCheckBox.append(cb);
            gridLayout->addWidget(cb, row, DrawContour, 1, 1);

            // TODO - size widget
            row++;
          }
        }
      }
    }
  }
}


void FidDialog::on_applyButton_clicked()
{
  close();
}

void FidDialog::on_cancelButton_clicked()
{
  close();
}

void FidDialog::pickColor()
{
	ColorWidget* cw = dynamic_cast<ColorWidget*>(sender());
	Q_ASSERT(cw);

	PickColor(cw->_color, true);
}

void FidDialog::pickSize()
{
	SizeWidget* sw = dynamic_cast<SizeWidget*>(sender());
	Q_ASSERT(sw);

	PickSize(&sw->_size, 10, "Fiducial Contour Size", true);
}

#if 0
void fidNameChange(FilesDialogRowData* rowdata){
  if (fiddialog->field_lock)
    return;
  fiddialog->field_lock = true;
  
  int index = gtk_combo_box_get_active(GTK_COMBO_BOX(rowdata->fid_name));
  gtk_range_set_value(GTK_RANGE(rowdata->fid_contSize),rowdata->mesh->fidConts[index]->fidContSize);
  
  gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_NORMAL, &rowdata->mesh->fidConts[index]->fidcolor);
  gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_ACTIVE, &rowdata->mesh->fidConts[index]->fidcolor);
  gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_PRELIGHT, &rowdata->mesh->fidConts[index]->fidcolor);
  
  Broadcast(MAP3D_UPDATE, 0);
  
  fiddialog->field_lock = false;
}

void fidContSizeChange(FilesDialogRowData* rowdata){
  if (fiddialog->field_lock)
    return;
  fiddialog->field_lock = true;
  
  //printf("Surfnum %d %d\n", rowdata->mesh->geom->subsurf, rowdata->mesh->gpriv->dominantsurf);
  int index = gtk_combo_box_get_active(GTK_COMBO_BOX(rowdata->fid_name));
  Mesh_List meshes = rowdata->mesh->gpriv->findMeshesFromSameInput(rowdata->mesh);

  if(rowdata->mesh->gpriv->dominantsurf == -1){
    for(int loop = 0; loop < meshes.size(); loop++){
      Mesh_Info *mesh = meshes[loop];
      mesh->fidConts[index]->fidContSize = (float) gtk_range_get_value(GTK_RANGE(rowdata->fid_contSize));
      //printf("%d Surfnum %d %d\n",loop, mesh->geom->subsurf, mesh->gpriv->dominantsurf);
    }
  }
  else{
    Mesh_Info *mesh = meshes[rowdata->mesh->gpriv->dominantsurf];
    mesh->fidConts[index]->fidContSize = (float) gtk_range_get_value(GTK_RANGE(rowdata->fid_contSize));
  }
  
  Broadcast(MAP3D_UPDATE, 0);
  
  fiddialog->field_lock = false;
}

void fidSelectColor(FilesDialogRowData* rowdata){
  if (fiddialog->field_lock)
    return;
  fiddialog->field_lock = true;
  
  int index = gtk_combo_box_get_active(GTK_COMBO_BOX(rowdata->fid_name));
  GtkTreeIter iter;
  gtk_combo_box_get_active_iter(GTK_COMBO_BOX(rowdata->fid_name),&iter);
  
  FidPickColor(&rowdata->mesh->fidConts[index]->fidcolor, index, iter, rowdata);
  
  fiddialog->field_lock = false;
}

void fidColorChange(GdkColor fidcolor, int index, GtkTreeIter iter, FilesDialogRowData* rowdata){
  Mesh_List meshes = rowdata->mesh->gpriv->findMeshesFromSameInput(rowdata->mesh);
  for(int loop = 0; loop < meshes.size(); loop++){
    Mesh_Info *mesh = meshes[loop];
    mesh->fidConts[index]->fidcolor.red = fidcolor.red;
    mesh->fidConts[index]->fidcolor.green = fidcolor.green;
    mesh->fidConts[index]->fidcolor.blue = fidcolor.blue;
  }

  
  GdkPixbuf* color;
  gtk_tree_model_get (GTK_TREE_MODEL(rowdata->fid_list_store), &iter, 
                      0, &color, -1);
  
  guint32 rgba = (((rowdata->mesh->fidConts[index]->fidcolor.red & 0xff00) << 8) |
                  ((rowdata->mesh->fidConts[index]->fidcolor.green & 0xff00)) |
                  ((rowdata->mesh->fidConts[index]->fidcolor.blue & 0xff00) >> 8)) << 8;
  gdk_pixbuf_fill(color, rgba);
  gtk_widget_queue_draw(rowdata->fid_name);
  
  if(index == gtk_combo_box_get_active(GTK_COMBO_BOX(rowdata->fid_name))){
    gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_NORMAL, &rowdata->mesh->fidConts[index]->fidcolor);
    gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_ACTIVE, &rowdata->mesh->fidConts[index]->fidcolor);
    gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_PRELIGHT, &rowdata->mesh->fidConts[index]->fidcolor);
  }
  
  Broadcast(MAP3D_UPDATE, 0);
}



// ------------------- //
//fidmap dialog callbacks, create, and accessor functions
void fidMapDialogCreate(bool show /*=true*/)
{
  if (!fidmapdialog){
    fidmapdialog = new FidMapDialog;
  }
  else if (show) {
    gtk_widget_show(fidmapdialog->window);
    return;
  }
  else
    return;
  
  fidmapdialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  fidmapdialog->field_lock = false;
  
  gtk_window_set_title(GTK_WINDOW(fidmapdialog->window), "Fiducial Maps");
  //  gtk_container_set_border_width (GTK_CONTAINER (map3d_info.window), 5);
  gtk_window_set_resizable(GTK_WINDOW(fidmapdialog->window), FALSE);
  
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_container_add(GTK_CONTAINER(fidmapdialog->window), vbox);
  gtk_widget_show(vbox);
  
  GtkWidget *hbox1 = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 2);
  GtkWidget* const_label = gtk_label_new("When range changes: ");
  GtkWidget* spacing_const = gtk_radio_button_new_with_label((NULL), "Keep spacing constant");
  GtkWidget* num_const = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(spacing_const)),
                                                         "Keep num contours constant");
  
  if (map3d_info.use_spacing_fid) {
    fidmapdialog->orig_spacing_check = true;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spacing_const),TRUE);
  }
  else {
    fidmapdialog->orig_spacing_check = false;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(num_const),TRUE);
  }
  
  gtk_box_pack_start(GTK_BOX(hbox1), const_label, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox1), spacing_const, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox1), num_const, FALSE, FALSE, 2);
  fidmapdialog->lock_contspacing = spacing_const;
  fidmapdialog->lock_numconts = num_const;  
  
  gtk_widget_show(hbox1);
  gtk_widget_show(spacing_const);
  gtk_widget_show(num_const);
  
  GtkWidget* table = gtk_table_new(1, NumCols, FALSE);
  fidmapdialog->table = table;
  gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
  gtk_widget_show(table);
  
  GtkWidget* surfnumtitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(surfnumtitle), "Surf"); 
  gtk_editable_set_editable(GTK_EDITABLE(surfnumtitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(surfnumtitle), 6);
  gtk_table_attach_defaults(GTK_TABLE(table), surfnumtitle, SurfNumMap, SurfNumMap+1, 0,1);
  gtk_widget_show(surfnumtitle);
  
  GtkWidget* geomnametitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geomnametitle), "Data File"); 
  gtk_editable_set_editable(GTK_EDITABLE(geomnametitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(geomnametitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), geomnametitle, GeomNameMap, GeomNameMap+1, 0,1);
  gtk_widget_show(geomnametitle);
  
  GtkWidget* fidnametitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(fidnametitle), "Fiducial"); 
  gtk_editable_set_editable(GTK_EDITABLE(fidnametitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(fidnametitle), 15);
  gtk_table_attach_defaults(GTK_TABLE(table), fidnametitle, FiducialMap, FiducialMap+1, 0,1);
  gtk_widget_show(fidnametitle);
  
  GtkWidget* spacing = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(spacing), "Spacing"); 
  gtk_editable_set_editable(GTK_EDITABLE(spacing), false);
  gtk_entry_set_width_chars(GTK_ENTRY(spacing), 20);
  gtk_table_attach_defaults(GTK_TABLE(table), spacing, SpacingMap, SpacingMap+1, 0,1);
  gtk_widget_show(spacing);
  
  GtkWidget* num_conts = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(num_conts), "Num Conts"); 
  gtk_editable_set_editable(GTK_EDITABLE(num_conts), false);
  gtk_entry_set_width_chars(GTK_ENTRY(num_conts), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), num_conts, NumContsMap, NumContsMap+1, 0,1);
  gtk_widget_show(num_conts);
  
  GtkWidget* default_range = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(default_range), "Default Range?"); 
  gtk_editable_set_editable(GTK_EDITABLE(default_range), false);
  gtk_entry_set_width_chars(GTK_ENTRY(default_range), 18);
  gtk_table_attach_defaults(GTK_TABLE(table), default_range, DefaultRangeMap, DefaultRangeMap+1, 0,1);
  gtk_widget_show(default_range);
  
  GtkWidget* range_low = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(range_low), "Low Range"); 
  gtk_editable_set_editable(GTK_EDITABLE(range_low), false);
  gtk_entry_set_width_chars(GTK_ENTRY(range_low), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), range_low, LowRangeMap, LowRangeMap+1, 0,1);
  gtk_widget_show(range_low);
  
  GtkWidget* range_high = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(range_high), "High Range"); 
  gtk_editable_set_editable(GTK_EDITABLE(range_high), false);
  gtk_entry_set_width_chars(GTK_ENTRY(range_high), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), range_high, HighRangeMap, HighRangeMap+1, 0,1);
  gtk_widget_show(range_high);
  
  GtkWidget* og = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(og), "Occlusion Grad"); 
  gtk_editable_set_editable(GTK_EDITABLE(og), false);
  gtk_entry_set_width_chars(GTK_ENTRY(og), 16);
  gtk_table_attach_defaults(GTK_TABLE(table), og, OcclusionMap, OcclusionMap+1, 0,1);
  gtk_widget_show(og);

  // fill the table in fileDialogCreate - that way when we add new surfaces we can
  // update the surfaces in the table as well.  Make sure the file window is created
  // to copy from
  filesDialogCreate(false);
  
  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, TRUE, 0);
  //  gtk_container_add(GTK_CONTAINER(fidmapdialog->window), notebook);
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
  
  
  g_signal_connect(G_OBJECT(cancel), "clicked", G_CALLBACK(FidMapCancel), NULL);
  g_signal_connect(G_OBJECT(okay), "clicked", G_CALLBACK(FidMapOkay), NULL);
  g_signal_connect_swapped(G_OBJECT(preview), "clicked", G_CALLBACK(FidMapPreview), false);
  
  g_signal_connect(G_OBJECT(fidmapdialog->window), "delete_event", G_CALLBACK(fidmapdestroycallback), NULL);
  
  if (show)
    gtk_widget_show(fidmapdialog->window);
  
}


void fidmapNameChange(GtkWidget * /*widget*/, FilesDialogRowData* rowdata){
  if (fidmapdialog->field_lock)
    return;
  fidmapdialog->field_lock = true;
  
  int index = gtk_combo_box_get_active(GTK_COMBO_BOX(rowdata->fid_map_name));
  Mesh_List meshes = rowdata->mesh->gpriv->findMeshesFromSameInput(rowdata->mesh);
  for(int loop = 0; loop < meshes.size(); loop++){
    Mesh_Info *mesh = meshes[loop];
    mesh->fidmapindex = index;

    float fmin=0,fmax=0;
    if (index > 0) {
      mesh->data->get_fid_minmax(fmin,fmax,mesh->fidMaps[mesh->fidmapindex -1]->datatype,
                                          rowdata->mesh->fidMaps[mesh->fidmapindex -1]->fidset);
      
      gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing),0.01,fmax-fmin);
      gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range),-10*fabs(fmin), 10*fabs(fmax));
      gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range),-10*fabs(fmin), 10*fabs(fmax));
      gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->fid_map_occlusion_gradient),0, fmax-fmin);
      
      mesh->drawcont = false;
      gtk_widget_show(rowdata->fid_map_contourspacing);
      gtk_widget_show(rowdata->fid_map_numcontours);
      gtk_widget_show(rowdata->fid_map_cont_low_range);
      gtk_widget_show(rowdata->fid_map_cont_high_range);
      gtk_widget_show(rowdata->fid_map_cont_default_range);
      gtk_widget_show(rowdata->fid_map_occlusion_gradient);
      
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range), rowdata->fid_map_orig_defaultrange[index]);
      if (rowdata->fid_map_orig_defaultrange[index]) {
        gtk_widget_set_sensitive(rowdata->fid_map_cont_low_range, false);
        gtk_widget_set_sensitive(rowdata->fid_map_cont_high_range, false);
      }
    }
    else {
      mesh->drawcont = true;
      printf("loop %d\n",loop);
      gtk_widget_hide(rowdata->fid_map_contourspacing);
      gtk_widget_hide(rowdata->fid_map_numcontours);
      gtk_widget_hide(rowdata->fid_map_cont_low_range);
      gtk_widget_hide(rowdata->fid_map_cont_high_range);
      gtk_widget_hide(rowdata->fid_map_cont_default_range);
      gtk_widget_hide(rowdata->fid_map_occlusion_gradient);
    }
  }
  
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours),rowdata->fid_map_orig_numconts[index]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range),rowdata->fid_map_orig_lowrange[index]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range),rowdata->fid_map_orig_highrange[index]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing),rowdata->fid_map_orig_numspaces[index]);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_occlusion_gradient),rowdata->fid_map_orig_og[index]);
  
  Broadcast(MAP3D_UPDATE, 0);
  fidmapdialog->field_lock = false;
}


// called from cancel or preview
void fidmapcontourcallback(FilesDialogRowData* rowdata)
{
  int numconts;
  Mesh_List meshes = rowdata->mesh->gpriv->findMeshesFromSameInput(rowdata->mesh);
  for (unsigned i = 0; i < meshes.size(); i++) {
    
    Mesh_Info* mesh = meshes[i];
    
    if(mesh->fidmapindex >0){
      
      if (mesh->data) {
        mesh->fidMaps[rowdata->mesh->fidmapindex -1]->userfidmin = (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range));
        mesh->fidMaps[rowdata->mesh->fidmapindex -1]->userfidmax = (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range));
      }
      
      mesh->fidMaps[rowdata->mesh->fidmapindex -1]->use_spacing_fid = (bool) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fidmapdialog->lock_contspacing));
      mesh->fidMaps[rowdata->mesh->fidmapindex -1]->occlusion_gradient = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_occlusion_gradient));
      
      if (mesh->fidMaps[rowdata->mesh->fidmapindex -1]->use_spacing_fid) {
        mesh->fidMaps[rowdata->mesh->fidmapindex -1]->fidcontourspacing = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing));
        numconts = mesh->fidMaps[rowdata->mesh->fidmapindex -1]->buildContours();
      }
      else {
        numconts = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours));
        mesh->fidMaps[rowdata->mesh->fidmapindex -1]->numfidconts = numconts;
      }
    }
  }
}


void fidmapdestroycallback()
{
  gtk_widget_hide(fidmapdialog->window);
}



void modifyfidmapDialogRow_RangeChange(FilesDialogRowData* rowdata)
{
  float min,max;
  float numconts;
  double spaces;

  
  if (fidmapdialog->field_lock)
    return;
  fidmapdialog->field_lock = true;
  
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range))) {
    gtk_widget_set_sensitive(rowdata->fid_map_cont_high_range, false);
    gtk_widget_set_sensitive(rowdata->fid_map_cont_low_range, false);
  }
  else {
    gtk_widget_set_sensitive(rowdata->fid_map_cont_high_range, true);
    gtk_widget_set_sensitive(rowdata->fid_map_cont_low_range, true);
  }
  
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range))) {
    rowdata->mesh->data->get_fid_minmax(min,max,rowdata->mesh->fidMaps[rowdata->mesh->fidmapindex -1]->datatype,
                                        rowdata->mesh->fidMaps[rowdata->mesh->fidmapindex -1]->fidset);
  }
  else{
    max = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range));
    min = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range));
  }

  
  if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(fidmapdialog->lock_contspacing)->check_button.toggle_button)){
    // update num conts if range changed
    float nc = (max-min)/
    ((float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing)))-1;
    numconts = nc;
    if (nc - (int) nc > 0)
      numconts = nc+1;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours),numconts);
  }
  else if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(fidmapdialog->lock_numconts)->check_button.toggle_button)) {    
    // update spacing if the range changed
    spaces = (max - min)/float(gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours))+1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing),spaces);
  }
  fidmapdialog->field_lock = false;
}

void modifyfidmapDialogRow_ContSpaceChange(FilesDialogRowData* rowdata)
{
  float min,max;
  int numconts;
  
  if (fidmapdialog->field_lock)
    return;
  fidmapdialog->field_lock = true;
  
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range))) {
    rowdata->mesh->data->get_fid_minmax(min,max,rowdata->mesh->fidMaps[rowdata->mesh->fidmapindex -1]->datatype,
                                        rowdata->mesh->fidMaps[rowdata->mesh->fidmapindex -1]->fidset);
  }
  else{
    max = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range));
    min = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range));
  }
  
  float spacing = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing));

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range))) {
    // update the num conts if the contour spacing changed
    float nc = (max-min)/spacing - 1;
    numconts = (int) nc;
    if (nc - (int) nc > 0)
      numconts = (int) nc+1;
  }
  else{
    // user range: change the range
    numconts = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours));
    float range = (numconts+1)*spacing;
    float range_ratio = range / (max-min);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range), max*range_ratio);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range), min*range_ratio);
  }
  if (numconts > MAX_CONTOURS) {
    numconts = MAX_CONTOURS;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing),(max-min)/(numconts+1));
  }
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours),numconts);

  fidmapdialog->field_lock = false;
}


void modifyfidmapDialogRow_NumContChange(FilesDialogRowData* rowdata)
{
  float min,max;
  double spaces;

  
  if (fidmapdialog->field_lock)
    return;
  fidmapdialog->field_lock = true;
  
  int numconts = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours));
  rowdata->mesh->data->get_fid_minmax(min,max,rowdata->mesh->fidMaps[rowdata->mesh->fidmapindex -1]->datatype,
                                      rowdata->mesh->fidMaps[rowdata->mesh->fidmapindex -1]->fidset);
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range))) {

    spaces = (max - min)/float(numconts+1) - .005;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing),spaces);
  }
  else{
    // user range: change the range
    float spacing = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing));
    float range = (numconts+1)*spacing;
    float range_ratio = range / (max-min);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range), max*range_ratio);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range), min*range_ratio);
  }
 
  fidmapdialog->field_lock = false;
}

void FidMapCancel(){
  fidmapdialog->field_lock = true;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fidmapdialog->lock_contspacing)) != fidmapdialog->orig_spacing_check) {
    if (fidmapdialog->orig_spacing_check) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fidmapdialog->lock_contspacing),TRUE);
    }
    else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fidmapdialog->lock_numconts),TRUE);
    }
  }
  
  for (unsigned i = 0; i < filedialog->rowData.size(); i++) {
    FilesDialogRowData* rowdata = filedialog->rowData[i];
    if(rowdata->mesh->fidmapindex > 0){
      if(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours)) != rowdata->fid_map_orig_numconts[rowdata->mesh->fidmapindex]){
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours), rowdata->fid_map_orig_numconts[rowdata->mesh->fidmapindex]);
      }
      if(gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing)) != rowdata->fid_map_orig_numspaces[rowdata->mesh->fidmapindex]) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing), rowdata->fid_map_orig_numspaces[rowdata->mesh->fidmapindex]);
      }
      if((gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range)) != rowdata->fid_map_orig_lowrange[rowdata->mesh->fidmapindex])||
         (gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range)) != rowdata->fid_map_orig_highrange[rowdata->mesh->fidmapindex])||
         (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range)) != rowdata->fid_map_orig_defaultrange[rowdata->mesh->fidmapindex])){
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range), rowdata->fid_map_orig_lowrange[rowdata->mesh->fidmapindex]);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range), rowdata->fid_map_orig_highrange[rowdata->mesh->fidmapindex]);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range), rowdata->fid_map_orig_defaultrange[rowdata->mesh->fidmapindex]);
      }
      if (rowdata->fid_map_orig_defaultrange[rowdata->mesh->fidmapindex]) {
        gtk_widget_set_sensitive(rowdata->fid_map_cont_low_range, false);
        gtk_widget_set_sensitive(rowdata->fid_map_cont_high_range, false);
      }
      if (gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_occlusion_gradient)) != rowdata->fid_map_orig_og[rowdata->mesh->fidmapindex]) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->fid_map_occlusion_gradient), rowdata->fid_map_orig_og[rowdata->mesh->fidmapindex]);
      }
      
      fidmapcontourcallback(rowdata); 
    }
  }
  Broadcast(MAP3D_UPDATE, 0);
  fidmapdialog->field_lock = false;
}


void FidMapPreview(bool okay){
  if (okay) 
    fidmapdialog->orig_spacing_check = (bool) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fidmapdialog->lock_contspacing));
  
  for (unsigned i = 0; i < filedialog->rowData.size(); i++) {
    FilesDialogRowData* rowdata = filedialog->rowData[i];
    if(rowdata->mesh->fidmapindex >0){
      fidmapcontourcallback(rowdata); 
      if(okay){
        rowdata->fid_map_orig_numconts[rowdata->mesh->fidmapindex] = (float) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->fid_map_numcontours));
        rowdata->fid_map_orig_numspaces[rowdata->mesh->fidmapindex] = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_contourspacing));
        rowdata->fid_map_orig_lowrange[rowdata->mesh->fidmapindex] = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_low_range));
        rowdata->fid_map_orig_highrange[rowdata->mesh->fidmapindex] = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_cont_high_range));
        rowdata->fid_map_orig_defaultrange[rowdata->mesh->fidmapindex] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->fid_map_cont_default_range));
        rowdata->fid_map_orig_og[rowdata->mesh->fidmapindex] = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->fid_map_occlusion_gradient));
      }
    }
  }
  
  Broadcast(MAP3D_UPDATE, 0);
}

void FidMapOkay(){
  FidMapPreview(true);
}


//
//Fid Color Picker Methods
//
gint FidColorPickerSetColor(GtkWidget*, GdkEvent*, void* data) {
  int colorindex = reinterpret_cast<int>(data);
  
  float r = 1;
  float g = 1;
  float b = 1;
  
  if (colorindex < 27) {
    r = colorindex * .5f;
    g = 0;
    b = 0;
    
    while (r - 1.5 >= 0) {
      r -= 1.5;
      g += .5;
    }
    while (g - 1.5 >= 0) {
      g -= 1.5;
      b += .5;
    }
  }
  
  fcp->selected_color.red = (guint16) (r * 65535);
  fcp->selected_color.green = (guint16) (g * 65535);
  fcp->selected_color.blue = (guint16) (b * 65535);
  
  gtk_widget_modify_bg(fcp->selected_color_widget, GTK_STATE_NORMAL, &fcp->selected_color);
  gtk_widget_queue_draw(fcp->selected_color_widget);
  return 0;
}

void FidColorPickerCancel(GtkWidget*)
{
  gtk_widget_hide(fcp->fidcolorpicker);
  fidColorChange(fcp->cs_orig_vals, fcp->index, fcp->iter, fcp->rowdata);
}

void FidColorPickerPreview(GtkWidget*)
{
  fidColorChange(fcp->selected_color, fcp->index, fcp->iter, fcp->rowdata);
}

void FidColorPickerOkay(GtkWidget* widget)
{
  FidColorPickerPreview(widget);
  gtk_widget_hide(fcp->fidcolorpicker);
}

void FidColorPickerCreate()
{
  if (fcp == NULL) {
    fcp = new FidColorPicker;
    
    // create the window and set window parameters
    fcp->fidcolorpicker = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(fcp->fidcolorpicker), "Select Color");
    gtk_window_set_resizable(GTK_WINDOW(fcp->fidcolorpicker), false);
    g_signal_connect(G_OBJECT(fcp->fidcolorpicker), "delete_event", G_CALLBACK(gtk_widget_hide),
                     NULL);
    
    // add sub-boxes
    GtkWidget* vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(fcp->fidcolorpicker), vbox);
    
    GtkWidget* hbox_color_cols = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_color_cols, FALSE, FALSE, 2);
    
    GtkWidget* hbox_color_cols2 = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_color_cols2, FALSE, FALSE, 2);
    
    GdkColor color;
    
    // add last row and selected color box into it.
    GtkWidget* hbox = gtk_hbox_new(TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    
    fcp->orig_color_widget = gtk_drawing_area_new();
    fcp->selected_color_widget = gtk_drawing_area_new();
    
    gtk_widget_set_size_request(fcp->selected_color_widget, 100, 25);
    gtk_widget_set_size_request(fcp->orig_color_widget, 100, 25);
    
    gtk_box_pack_start(GTK_BOX(hbox), fcp->orig_color_widget, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), fcp->selected_color_widget, FALSE, FALSE, 2);
    
    int numcolors = 28;
    // add colors
    for (int i = 0; i < numcolors; i++) {
      GtkWidget* frame = gtk_drawing_area_new();
      gtk_widget_set_size_request(frame, 22, 22);
      gtk_widget_add_events(frame, GDK_BUTTON_PRESS_MASK);
      
      // interpolate through all combinations of 0,.5,.1 for color
      float r = 1;
      float g = 1;
      float b = 1;
      
      if (i < 27) {
        r = i * .5f;
        g = 0;
        b = 0;
        
        while (r - 1.5 >= 0) {
          r -= 1.5;
          g += .5;
        }
        while (g - 1.5 >= 0) {
          g -= 1.5;
          b += .5;
        }
      }
      
      color.red = (guint16) (r * 65535);
      color.green = (guint16) (g * 65535);
      color.blue = (guint16) (b * 65535);
      
      if (i != numcolors -1) {
        gtk_widget_modify_bg(frame, GTK_STATE_NORMAL, &color);
        g_signal_connect(G_OBJECT(frame), "button_press_event", G_CALLBACK(FidColorPickerSetColor), (void*)i);
      }
      
      
      if (i < numcolors/2)
        gtk_box_pack_start(GTK_BOX(hbox_color_cols), frame, FALSE, FALSE, 2);
      else
        gtk_box_pack_start(GTK_BOX(hbox_color_cols2), frame, FALSE, FALSE, 2);
    }
    
    
    GtkWidget *preview, *cancel, *okay;
    preview = gtk_button_new_with_label("Preview");
    cancel = gtk_button_new_with_label("Cancel");
    okay = gtk_button_new_with_label("OK");
    
    gtk_box_pack_start(GTK_BOX(hbox), preview, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), cancel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), okay, FALSE, TRUE, 0);
    
    g_signal_connect(G_OBJECT(cancel), "clicked", G_CALLBACK(FidColorPickerCancel), NULL);
    g_signal_connect(G_OBJECT(okay), "clicked", G_CALLBACK(FidColorPickerOkay), NULL);
    g_signal_connect(G_OBJECT(preview), "clicked", G_CALLBACK(FidColorPickerPreview), NULL);
  }
  
  gtk_widget_show_all(fcp->fidcolorpicker);
}

void FidPickColor(GdkColor *storage, int index, GtkTreeIter iter,FilesDialogRowData* rowdata)
{  
  if (!fcp)
    FidColorPickerCreate();
  else
    gtk_widget_show(fcp->fidcolorpicker);
  
  fcp->index = index;
  fcp->iter = iter;
  fcp->rowdata = rowdata;
  
  fcp->cs_orig_vals.red = storage->red;
  fcp->cs_orig_vals.green = storage->green;
  fcp->cs_orig_vals.blue = storage->blue;
  
  fcp->selected_color.red = storage->red;
  fcp->selected_color.green = storage->green;
  fcp->selected_color.blue = storage->blue;
  
  gtk_widget_modify_bg(fcp->selected_color_widget, GTK_STATE_NORMAL, &fcp->selected_color);
  gtk_widget_modify_bg(fcp->orig_color_widget, GTK_STATE_NORMAL, &fcp->selected_color);
}

#endif