#include "FileDialog.h"
#include "MainWindow.h"
#include "Map3d_Geom.h"
#include "MeshList.h"
#include "PickWindow.h"
#include "ProcessCommandLineOptions.h"
#include "Surf_Data.h"
#include "Transforms.h"
#include "WindowManager.h"
#include "eventdata.h"
#include "map3d-struct.h"
#include "pickinfo.h"
#include "ContourDialog.h"
#include "Contour_Info.h"
#include <math.h>

#include <QFileDialog>
#include <QDebug>

FileDialog* filedialog;
//extern SaveDialog *savedialog;
//extern ContourDialog *contourdialog;
//extern FidDialog *fiddialog;
//extern FidMapDialog *fidmapdialog;
extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
//extern FilePicker *fp;
// ------------------- //
// files dialog helpers, callbacks, create, and accessor functions

const char* surfPropName = "SurfaceNum";

#if 0
void addRMSData(FilesDialogRowData* data)
{
  char* datafilename = (char*) gtk_entry_get_text(GTK_ENTRY(data->dataname));
  char* geomfilename = (char*) gtk_entry_get_text(GTK_ENTRY(data->geomname));
  //char* chfilename = data->mesh->mysurf->chfilename;
  int ds = getGtkComboIndex(data->datafile,(char*) gtk_entry_get_text(GTK_ENTRY(data->dataname)));  
  
  int numframes = GetNumFrames(datafilename, ds);
  
  // we need to load separate surfaces here so the user can browse different data while keeping
  // the originals displayed.
  if (strcmp(geomfilename, "") != 0 && strcmp(datafilename, "") != 0) {
    // load geom and data so we can update the rms curve
    Surf_Input* input = new Surf_Input;
    Init_Surf_Input(input);
    input->parent = map3d_info.gi;
    input->geomfilename = new char[256];
    input->potfilename = new char[256];

    if (data->mesh->mysurf && data->mesh->mysurf->chfilename) {
      input->chfilename = new char[256];
      strcpy(input->chfilename,data->mesh->mysurf->chfilename);
    }
    else {
      char* chfilename = (char*) gtk_entry_get_text(GTK_ENTRY(data->channelsfilename));
      if (strcmp(chfilename,"") != 0) {
        input->chfilename = new char[256];
        strcpy(input->chfilename,chfilename);
      }
    }
    input->geomsurfnum = atoi(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(data->geomseries)->entry)));
    input->timeseries = ds;
    input->displaysurfnum = atoi(gtk_entry_get_text(GTK_ENTRY(data->surfnum)));
    strcpy(input->geomfilename,geomfilename);
    strcpy(input->potfilename,datafilename);
    input->ts_end = numframes-1;
    input->ts_start = 0;
    
    Mesh_Info* mesh = new Mesh_Info;
    Mesh_List currentMeshes;
    currentMeshes.push_back(mesh);
    
    // turn off report level, since we'll just be duplicating what we've seen already
    int reportlevel = map3d_info.reportlevel;
    map3d_info.reportlevel = 0;
    Mesh_List returnedMeshes = FindAndReadGeom(input,currentMeshes,LOAD_RMS_DATA);
    map3d_info.reportlevel = reportlevel;
    
    if (returnedMeshes.size() > 0) {
      if (data->rms_curve->mesh)
        delete data->rms_curve->mesh;
      data->rms_curve->mesh = mesh;
    }
    else {
      delete mesh;
    }
    
  }
}
#endif

void FileDialogWidget::on_geomLineEdit_editingFinished ()
{
  int num = GetNumGeoms(geomLineEdit->text().toAscii().data());
  qDebug() << __FUNCTION__;

  geomIndexComboBox->clear();
  QStringList indexEntries;
  for (int i = 1; i <= num; i++)
    indexEntries << QString::number(i);

  if (num > 1)
    indexEntries << "*";

  geomIndexComboBox->addItems(indexEntries);
  reload_geom = true;    
}

void FileDialogWidget::on_geomIndexComboBox_activated ( const QString & text )
{
  reload_geom = true;    
}

void FileDialogWidget::on_dataLineEdit_editingFinished ()
{
  int num = GetNumGeoms(geomLineEdit->text().toAscii().data());

  QString filename = dataLineEdit->text();

  if (filename.endsWith(".tsdfc") || filename.endsWith(".mat"))
  {
    int numseries = GetNumTimeSeries(filename.toAscii().data());
    
    QStringList items;
    for(int i = 0; i < numseries; i++){
      char label[100];
      GetTimeSeriesLabel(filename.toAscii().data(), i, label);
      items << label;
    }

    dataIndexComboBox->clear();
    dataIndexComboBox->addItems(items);
  }
  
  int ds = 0;  // if changing the first filename, always grab the first index
  int numframes = GetNumFrames(filename.toAscii().data(), ds);
  startFrameSpinBox->setRange(1, numframes);
  endFrameSpinBox->setRange(1, numframes);

  startFrameSpinBox->setValue(1);
  endFrameSpinBox->setValue(numframes);
  
  addRMSData(); 
  reload_data = true;
}
  
  
void FileDialogWidget::on_dataIndexComboBox_activated ( const QString & text )
{
  char* filename = dataLineEdit->text().toAscii().data();
  if(strcmp(filename,"")!=0)
  {  
    int ds = dataIndexComboBox->currentIndex();
    int dstart = startFrameSpinBox->value()-1;
    //int dend = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dataend))-1;
    
    int numframes = GetNumFrames(filename, ds);

    startFrameSpinBox->setRange(1, numframes);
    endFrameSpinBox->setRange(1, numframes);

    startFrameSpinBox->setValue(1);
    endFrameSpinBox->setValue(numframes);
    
    addRMSData(); 
    reload_data = true;
  }
}
  
void FileDialogWidget::on_startFrameSpinBox_valueChanged ( int i )
{
  updateRMS();
  reload_data = true;
}
  
void FileDialogWidget::on_endFrameSpinBox_valueChanged ( int i )
{
  updateRMS();
  reload_data = true;
}

// these happen when you click the '...' buttons
void FileDialogWidget::on_geomBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select geometry file");
    qDebug() << __FUNCTION__ << newFile;

  geomLineEdit->setText(newFile);
  on_geomLineEdit_editingFinished();
}
  
void FileDialogWidget::on_dataBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select data file");
  dataLineEdit->setText(newFile);
  on_dataLineEdit_editingFinished();
}

void FileDialogWidget::on_channelsBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select channels file");
  channelsLineEdit->setText(newFile); 
}

void FileDialogWidget::on_leadlinksBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select leadlinks file");
  leadlinksLineEdit->setText(newFile);
}

void FileDialogWidget::on_fiducialBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select Fiducial file");
  fiducialLineEdit->setText(newFile);
}

void FileDialogWidget::on_landmarksBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select Landmarks file");
  landmarksLineEdit->setText(newFile);
}

void FileDialogWidget::on_expandButton_clicked ()
{
  bool visible = otherOptionsFrame->isVisible();
  if (visible)
  {
    expandButton->setText("+");
    otherOptionsFrame->setVisible(false);
  }
  else
  {
    expandButton->setText("-");
    otherOptionsFrame->setVisible(true);
  }

}

#if 0
void addRowToOtherDialogs()
{
  // we want to keep all dialog tables consistent (with surface information)
  // but we can't always do it at the same time.  At start time we can,
  // but when we click the "New Surface" button, we need to immediately add one to
  // the files dialog, but wait until they click apply to do the others
  
  int NumRows = ((GtkTable*)savedialog->table)->nrows+1;
  // numRows counts the header and is 1-based.  We want the same row from 
  // the file window to update it from.
  FilesDialogRowData* rowdata = filedialog->rowData[NumRows - 2];
  Mesh_Info* mesh = rowdata->mesh;
  
  // - SAVE DIALOG ROWS - //
  gtk_table_resize(GTK_TABLE(savedialog->table), NumRows, 4);
  GtkWidget* save_surfnum = gtk_entry_new(); 
  rowdata->save_surfnum = save_surfnum;
  char save_surf[20];
  sprintf(save_surf, "%d",NumRows-1);
  gtk_entry_set_text(GTK_ENTRY(save_surfnum), save_surf); 
  gtk_editable_set_editable(GTK_EDITABLE(save_surfnum), false);
  gtk_entry_set_width_chars(GTK_ENTRY(save_surfnum), 3);
  gtk_table_attach_defaults(GTK_TABLE(savedialog->table), save_surfnum, SurfNum, SurfNum+1, NumRows-1, NumRows);
  gtk_widget_show(save_surfnum);
  
  GtkWidget* geomsaveinputname = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geomsaveinputname), shorten_filename(mesh->geom->basefilename)); 
  gtk_editable_set_editable(GTK_EDITABLE(geomsaveinputname), false);
  gtk_entry_set_width_chars(GTK_ENTRY(geomsaveinputname), 22);
  gtk_table_attach_defaults(GTK_TABLE(savedialog->table), geomsaveinputname, 1, 2, NumRows-1, NumRows);
  gtk_widget_show(geomsaveinputname);
  rowdata->save_input_filename = geomsaveinputname;
  
  GtkWidget* geomsavename = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geomsavename), mesh->geom->basefilename); 
  gtk_editable_set_editable(GTK_EDITABLE(geomsavename), true);
  gtk_entry_set_width_chars(GTK_ENTRY(geomsavename), 22);
  gtk_table_attach_defaults(GTK_TABLE(savedialog->table), geomsavename, 2, 3, NumRows-1, NumRows);
  gtk_widget_show(geomsavename);
  rowdata->save_filename = geomsavename;
  // add it to the list of file picker widgets so we can remember the directory
  fp->assignEntry(geomsavename, fp->file_selector_save);
  
  GtkWidget* geomsavechange = gtk_button_new_with_label("...");
  gtk_table_attach_defaults(GTK_TABLE(savedialog->table), geomsavechange, 3, 4, NumRows-1, NumRows);
  gtk_widget_show(geomsavechange);
  g_signal_connect_swapped(G_OBJECT(geomsavechange), "clicked", G_CALLBACK(PickFileWidget), geomsavename);
  
  GtkWidget* savethisgeom = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(savethisgeom), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(savethisgeom), true);
  gtk_table_attach_defaults(GTK_TABLE(savedialog->table), savethisgeom, 4, 5, NumRows-1, NumRows);
  gtk_widget_show(savethisgeom);
  rowdata->save_this_surface = savethisgeom;
  
  GtkWidget* savewithtrans = gtk_check_button_new(); 
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(savewithtrans), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(savewithtrans), false);
  gtk_table_attach_defaults(GTK_TABLE(savedialog->table), savewithtrans, 5, 6, NumRows-1, NumRows);
  gtk_widget_show(savewithtrans);
  rowdata->save_with_transforms = savewithtrans;
  
  // CONTOUR DIALOG ROWS
  float surfmin=0, surfmax=0;
  float min=0, max=0;
  if (mesh->data) {
    surfmin = mesh->data->potmin;
    surfmax = mesh->data->potmax;
    mesh->data->get_minmax(min, max);
    float spaces;
    if (map3d_info.use_spacing && mesh->contourspacing)
      spaces = mesh->contourspacing;
    else {
      spaces = (max - min)/float(mesh->data->numconts+1);
      mesh->contourspacing = spaces;
    }
    rowdata->orig_exp = floor(log10(spaces));
    rowdata->orig_numspaces = spaces;
    rowdata->orig_numconts = (float) mesh->data->numconts;
    rowdata->orig_lowrange = mesh->data->userpotmin;
    rowdata->orig_highrange = mesh->data->userpotmax;
    rowdata->orig_defaultrange = !mesh->data->user_scaling;
  }
  else {
    rowdata->orig_exp = 1;
    rowdata->orig_numspaces = 1;
    rowdata->orig_numconts = 1;
    rowdata->orig_lowrange = 0.0;
    rowdata->orig_highrange = 0.0;
    rowdata->orig_defaultrange = true;
  }
  
  GtkWidget* cont_surfnum = gtk_entry_new(); 
  rowdata->cont_surfnum = cont_surfnum;
  gtk_entry_set_text(GTK_ENTRY(cont_surfnum), save_surf); 
  gtk_editable_set_editable(GTK_EDITABLE(cont_surfnum), false);
  gtk_entry_set_width_chars(GTK_ENTRY(cont_surfnum), 3);
  gtk_widget_show(cont_surfnum);
  
  GtkWidget* cont_surfname = gtk_entry_new(); 
  rowdata->cont_surfname = cont_surfname;
  gtk_entry_set_text(GTK_ENTRY(cont_surfname), shorten_filename(mesh->geom->basefilename)); 
  gtk_editable_set_editable(GTK_EDITABLE(cont_surfname), false);
  gtk_entry_set_width_chars(GTK_ENTRY(cont_surfname), 22);
  gtk_widget_show(cont_surfname);
  
  float cs_min = (surfmax-surfmin)/1000;  // 1000 because local windows can often be a small subset of the data range
  float high = MAX(fabs(surfmax), fabs(surfmin));

  rowdata->cs_adj = gtk_adjustment_new(rowdata->orig_numspaces,1e-6,surfmax-surfmin,cs_min,cs_min*10,cs_min*10);
  rowdata->contourspacing = gtk_spin_button_new((GtkAdjustment*)rowdata->cs_adj,.1,4);
  gtk_widget_show(rowdata->contourspacing);
  
  rowdata->cn_adj = gtk_adjustment_new(rowdata->orig_numconts,1,100,1,1,1);
  rowdata->numcontours = gtk_spin_button_new((GtkAdjustment*)rowdata->cn_adj,1,0);
  gtk_widget_show(rowdata->numcontours);
  rowdata->lr_adj = gtk_adjustment_new(mesh->data?mesh->data->userpotmin:0,-3*high, 3*high,1,5,5);
  rowdata->cont_low_range = gtk_spin_button_new((GtkAdjustment*)rowdata->lr_adj,1,3);
  gtk_widget_show(rowdata->cont_low_range);
  rowdata->hr_adj = gtk_adjustment_new(mesh->data?mesh->data->userpotmax:0,-3*high, 3*high,1,5,5);
  rowdata->cont_high_range = gtk_spin_button_new((GtkAdjustment*)rowdata->hr_adj,1,3);
  gtk_widget_show(rowdata->cont_high_range);
  rowdata->cont_default_range = gtk_check_button_new();
  gtk_widget_show(rowdata->cont_default_range);
  
  rowdata->cog_adj = gtk_adjustment_new(3*(surfmax-surfmin), 0, 100*(surfmax-surfmin),1,5,5);
  rowdata->cont_occlusion_gradient = gtk_spin_button_new((GtkAdjustment*)rowdata->cog_adj,1,2);
  gtk_widget_show(rowdata->cont_occlusion_gradient);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range), !(mesh->data && mesh->data->user_scaling));
  if (rowdata->orig_defaultrange) {
    gtk_widget_set_sensitive(rowdata->cont_low_range, false);
    gtk_widget_set_sensitive(rowdata->cont_high_range, false);
  }
  
  
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), cont_surfnum, 0,1, NumRows-1, NumRows);
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), cont_surfname, 1, 2, NumRows-1, NumRows);
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), rowdata->contourspacing, 2, 3, NumRows-1, NumRows);
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), rowdata->numcontours, 3, 4, NumRows-1, NumRows);
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), rowdata->cont_default_range, 4, 5, NumRows-1, NumRows);
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), rowdata->cont_low_range, 5, 6, NumRows-1, NumRows);
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), rowdata->cont_high_range, 6, 7, NumRows-1, NumRows);
  gtk_table_attach_defaults(GTK_TABLE(contourdialog->table), rowdata->cont_occlusion_gradient, 7, 8, NumRows-1, NumRows);
  
  g_signal_connect_swapped(G_OBJECT(rowdata->contourspacing), "value_changed", G_CALLBACK(modifyContourDialogRow_ContSpaceChange),rowdata);
  g_signal_connect_swapped(G_OBJECT(rowdata->numcontours), "value_changed", G_CALLBACK(modifyContourDialogRow_NumContChange),rowdata);
  g_signal_connect_swapped(G_OBJECT(rowdata->cont_low_range), "value_changed", G_CALLBACK(modifyContourDialogRow_RangeChange),rowdata);
  g_signal_connect_swapped(G_OBJECT(rowdata->cont_high_range), "value_changed", G_CALLBACK(modifyContourDialogRow_RangeChange),rowdata);
  g_signal_connect_swapped(G_OBJECT(rowdata->cont_default_range), "clicked", G_CALLBACK(modifyContourDialogRow_RangeChange),rowdata);
  
  if(mesh->data){
    // FIDUCIAL DIALOG ROWS
    GtkWidget* fid_surfnum = gtk_entry_new(); 
    rowdata->fid_surfnum = fid_surfnum;
    gtk_entry_set_text(GTK_ENTRY(fid_surfnum), save_surf); 
    gtk_editable_set_editable(GTK_EDITABLE(fid_surfnum), false);
    gtk_entry_set_width_chars(GTK_ENTRY(fid_surfnum), 3);
    gtk_widget_show(fid_surfnum);
    
    GtkWidget* fid_surfname = gtk_entry_new(); 
    rowdata->fid_surfname = fid_surfname;
    gtk_entry_set_text(GTK_ENTRY(fid_surfname), shorten_filename(mesh->mysurf->potfilename)); 
    gtk_editable_set_editable(GTK_EDITABLE(fid_surfname), false);
    gtk_entry_set_width_chars(GTK_ENTRY(fid_surfname), 22);
    gtk_widget_show(fid_surfname);
    
    if(mesh->data->numfs == 0){
      rowdata->fid_name = gtk_combo_box_new_text ();
      gtk_combo_box_append_text (GTK_COMBO_BOX (rowdata->fid_name), "No Fiducials");
      gtk_widget_show(rowdata->fid_name);
      gtk_combo_box_set_active(GTK_COMBO_BOX (rowdata->fid_name),0);
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), fid_surfnum, 0,1, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), fid_surfname, 1, 2, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), rowdata->fid_name, 2, 3, NumRows-1, NumRows);
    }
    else{
      int index = 0;
      GtkTreeIter iter;
      GtkCellRenderer *renderer;
      
      rowdata->fid_list_store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
      
      for(int fidsets = 0; fidsets < mesh->data->numfs; fidsets++){
        for(int i = 0; i < mesh->data->fids[fidsets].numfidtypes;i++){
          GdkPixbuf* color;
          char *label2 = new char[140];
          sprintf(label2, "%s %s",mesh->data->fids[fidsets].fidnames[i],mesh->data->fids[fidsets].fidlabel);
          
          color =  gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8, 15, 15 );
          guint32 rgba = (((rowdata->mesh->fidConts[index]->fidcolor.red & 0xff00) << 8) |
                          ((rowdata->mesh->fidConts[index]->fidcolor.green & 0xff00)) |
                          ((rowdata->mesh->fidConts[index]->fidcolor.blue & 0xff00) >> 8)) << 8;
          gdk_pixbuf_fill(color, rgba);
          
          gtk_list_store_append (GTK_LIST_STORE(rowdata->fid_list_store), &iter);
          gtk_list_store_set (GTK_LIST_STORE(rowdata->fid_list_store), &iter, 0, color, 1, label2, -1);
          
          index++;
        }
      }
      rowdata->fid_name = gtk_combo_box_new_with_model (GTK_TREE_MODEL (rowdata->fid_list_store));
      renderer = gtk_cell_renderer_pixbuf_new ();
      gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (rowdata->fid_name), renderer, FALSE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (rowdata->fid_name), renderer,
                                      "pixbuf", 0,
                                      NULL);    
      renderer = gtk_cell_renderer_text_new ();
      gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (rowdata->fid_name), renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (rowdata->fid_name), renderer,
                                      "text", 1,
                                      NULL);
      
      gtk_combo_box_set_active(GTK_COMBO_BOX (rowdata->fid_name),0);
      
      gtk_widget_show(rowdata->fid_name);
      
      GtkObject* adj = gtk_adjustment_new(3,0,11,1,1,1);
      rowdata->fid_contSize = gtk_hscale_new((GtkAdjustment*)adj);
      gtk_widget_show(rowdata->fid_contSize);
      
      rowdata->fid_contColor = gtk_event_box_new();
      gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_NORMAL, &rowdata->mesh->fidConts[0]->fidcolor);
      gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_ACTIVE, &rowdata->mesh->fidConts[0]->fidcolor);
      gtk_widget_modify_bg (rowdata->fid_contColor, GTK_STATE_PRELIGHT, &rowdata->mesh->fidConts[0]->fidcolor);
      
      gtk_widget_show(rowdata->fid_contColor);
      rowdata->fid_contColor_button = gtk_button_new();
      gtk_container_add(GTK_CONTAINER(rowdata->fid_contColor_button),rowdata->fid_contColor);
      gtk_widget_show(rowdata->fid_contColor_button);
      
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), fid_surfnum, 0,1, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), fid_surfname, 1, 2, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), rowdata->fid_name, 2, 3, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), rowdata->fid_contSize, 3, 4, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fiddialog->table), rowdata->fid_contColor_button, 4, 5, NumRows-1, NumRows);
      
      g_signal_connect_swapped(G_OBJECT(GTK_COMBO_BOX(rowdata->fid_name)), "changed", G_CALLBACK(fidNameChange),rowdata);
      
      g_signal_connect_swapped(G_OBJECT(rowdata->fid_contSize), "value_changed", G_CALLBACK(fidContSizeChange),rowdata);
      g_signal_connect_swapped(G_OBJECT(rowdata->fid_contColor_button), "clicked", G_CALLBACK(fidSelectColor),rowdata);
      
    }
    
    
    // FIDUCIAL MAP DIALOG ROWS
    GtkWidget* fid_map_surfnum = gtk_entry_new(); 
    rowdata->fid_map_surfnum = fid_map_surfnum;
    gtk_entry_set_text(GTK_ENTRY(fid_map_surfnum), save_surf); 
    gtk_editable_set_editable(GTK_EDITABLE(fid_map_surfnum), false);
    gtk_entry_set_width_chars(GTK_ENTRY(fid_map_surfnum), 3);
    gtk_widget_show(fid_map_surfnum);
    
    GtkWidget* fid_map_surfname = gtk_entry_new(); 
    rowdata->fid_map_surfname = fid_map_surfname;
    gtk_entry_set_text(GTK_ENTRY(fid_map_surfname), shorten_filename(mesh->mysurf->potfilename)); 
    gtk_editable_set_editable(GTK_EDITABLE(fid_map_surfname), false);
    gtk_entry_set_width_chars(GTK_ENTRY(fid_map_surfname), 22);
    gtk_widget_show(fid_map_surfname);
    
    if(mesh->data->numfs == 0){
      rowdata->fid_map_name = gtk_entry_new(); 
      gtk_entry_set_text(GTK_ENTRY(rowdata->fid_map_name), "No Fiducials"); 
      gtk_editable_set_editable(GTK_EDITABLE(rowdata->fid_map_name), false);
      gtk_entry_set_width_chars(GTK_ENTRY(rowdata->fid_map_name), 22);
      gtk_widget_show(rowdata->fid_map_name);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), fid_map_surfnum, 0,1, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), fid_map_surfname, 1, 2, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_name, 2, 3, NumRows-1, NumRows);
      
      rowdata->fid_map_orig_numspaces.push_back(1);
      rowdata->fid_map_orig_numconts.push_back(1);
      rowdata->fid_map_orig_lowrange.push_back(1);
      rowdata->fid_map_orig_highrange.push_back(1);
      rowdata->fid_map_orig_defaultrange.push_back(true);
      rowdata->fid_map_orig_og.push_back(1);
    }
    else{
      rowdata->fid_map_name = gtk_combo_box_new_text ();
      gtk_combo_box_append_text (GTK_COMBO_BOX (rowdata->fid_map_name), "No Fiducials");
      
      rowdata->fid_map_orig_numspaces.push_back(1);
      rowdata->fid_map_orig_numconts.push_back(1);
      rowdata->fid_map_orig_lowrange.push_back(1);
      rowdata->fid_map_orig_highrange.push_back(1);
      rowdata->fid_map_orig_defaultrange.push_back(true);
      rowdata->fid_map_orig_og.push_back(1);
      
      float fmin=0,fmax=0;
      int index = 0;
      for(int fidsets = 0; fidsets < mesh->data->numfs; fidsets++){
        for(int i = 0; i < mesh->data->fids[fidsets].numfidtypes;i++){
          char *label2 = new char[140];
          sprintf(label2, "%s %s",mesh->data->fids[fidsets].fidnames[i],mesh->data->fids[fidsets].fidlabel);
          gtk_combo_box_append_text (GTK_COMBO_BOX (rowdata->fid_map_name), label2);
          
          mesh->data->get_fid_minmax(fmin,fmax,rowdata->mesh->fidMaps[index]->datatype,rowdata->mesh->fidMaps[index]->fidset);
          rowdata->fid_map_orig_numspaces.push_back((fmax - fmin)/float(rowdata->mesh->fidMaps[index]->numfidconts+1));
          rowdata->fid_map_orig_numconts.push_back((float) rowdata->mesh->fidMaps[index]->numfidconts);
          rowdata->fid_map_orig_lowrange.push_back(fmin);
          rowdata->fid_map_orig_highrange.push_back(fmax);
          rowdata->fid_map_orig_defaultrange.push_back(true);
          rowdata->fid_map_orig_og.push_back(fmax-fmin);
          index++;
        }
      }
      gtk_widget_show(rowdata->fid_map_name);
      gtk_combo_box_set_active(GTK_COMBO_BOX (rowdata->fid_map_name),0);
      
      rowdata->fid_map_cs_adj = gtk_adjustment_new(rowdata->fid_map_orig_numspaces[0],0.01,rowdata->fid_map_orig_highrange[0]-rowdata->fid_map_orig_lowrange[0],0.01,0.1,1);
      rowdata->fid_map_contourspacing = gtk_spin_button_new((GtkAdjustment*)rowdata->fid_map_cs_adj,1,2);
      rowdata->fid_map_cn_adj = gtk_adjustment_new(rowdata->fid_map_orig_numconts[0],1,100,1,1,1);
      rowdata->fid_map_numcontours = gtk_spin_button_new((GtkAdjustment*)rowdata->fid_map_cn_adj,1,0);
      rowdata->fid_map_cont_default_range = gtk_check_button_new();
      rowdata->fid_map_lr_adj = gtk_adjustment_new(rowdata->fid_map_orig_lowrange[0],-10*fabs(rowdata->fid_map_orig_lowrange[0]), 10*fabs(rowdata->fid_map_orig_highrange[0]),1,5,5);
      rowdata->fid_map_cont_low_range = gtk_spin_button_new((GtkAdjustment*)rowdata->fid_map_lr_adj,1,2);
      rowdata->fid_map_hr_adj = gtk_adjustment_new(rowdata->fid_map_orig_highrange[0],-10*fabs(rowdata->fid_map_orig_lowrange[0]), 10*fabs(rowdata->fid_map_orig_highrange[0]),1,5,5);
      rowdata->fid_map_cont_high_range = gtk_spin_button_new((GtkAdjustment*)rowdata->fid_map_hr_adj,1,2);

      rowdata->fmog_adj = gtk_adjustment_new(rowdata->fid_map_orig_og[0], 0, rowdata->fid_map_orig_highrange[0]-rowdata->fid_map_orig_lowrange[0],1,5,5);
      rowdata->fid_map_occlusion_gradient = gtk_spin_button_new((GtkAdjustment*)rowdata->fmog_adj,1,2);
      gtk_widget_show(rowdata->fid_map_occlusion_gradient);


      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), fid_map_surfnum, 0,1, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), fid_map_surfname, 1, 2, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_name, 2, 3, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_contourspacing, 3, 4, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_numcontours, 4, 5, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_cont_default_range, 5, 6, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_cont_low_range, 6, 7, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_cont_high_range, 7, 8, NumRows-1, NumRows);
      gtk_table_attach_defaults(GTK_TABLE(fidmapdialog->table), rowdata->fid_map_occlusion_gradient, 8, 9, NumRows-1, NumRows);
      
      g_signal_connect(G_OBJECT(GTK_COMBO_BOX(rowdata->fid_map_name)), "changed", G_CALLBACK(fidmapNameChange),rowdata);
      g_signal_connect_swapped(G_OBJECT(rowdata->fid_map_contourspacing), "value_changed", G_CALLBACK(modifyfidmapDialogRow_ContSpaceChange),rowdata);
      g_signal_connect_swapped(G_OBJECT(rowdata->fid_map_numcontours), "value_changed", G_CALLBACK(modifyfidmapDialogRow_NumContChange),rowdata);
      g_signal_connect_swapped(G_OBJECT(rowdata->fid_map_cont_low_range), "value_changed", G_CALLBACK(modifyfidmapDialogRow_RangeChange),rowdata);
      g_signal_connect_swapped(G_OBJECT(rowdata->fid_map_cont_high_range), "value_changed", G_CALLBACK(modifyfidmapDialogRow_RangeChange),rowdata);
      g_signal_connect_swapped(G_OBJECT(rowdata->fid_map_cont_default_range), "clicked", G_CALLBACK(modifyfidmapDialogRow_RangeChange),rowdata);
      
    }
  }
  
}

#endif

void FileDialog::addRow(Mesh_Info* mesh)
{
  bool empty_mesh = false;
  int surfnumber, window;
  if (!mesh) {
    // we clicked the addRow button
    surfnumber = _widgets.size();
    window = numGeomWindows(); // put in in a new window by default
    mesh = new Mesh_Info; // initialize an empty mesh
    empty_mesh = true;
  }
  else {
    surfnumber = mesh->geom->surfnum-1;
    window = mesh->gpriv->geomWinId;
  }

  FileDialogWidget* widget = new FileDialogWidget(surfaceScrollArea, mesh);
  widget->setProperty(surfPropName, surfnumber);
  _widgets.append(widget);

  surfaceScrollAreaLayout->addWidget(widget);

  widget->surfLabel->setText("Surface #" + QString::number(surfnumber+1));
  widget->winComboBox->setCurrentIndex(window); // the combo box is one-based

  if (!empty_mesh)
  {
    widget->geomLineEdit->setText(mesh->mysurf->geomfilename);
    widget->on_geomLineEdit_editingFinished();
    if (mesh->mysurf->geomsurfnum < 0)
      widget->geomIndexComboBox->setCurrentIndex(widget->geomIndexComboBox->count()-1);
    else
      widget->geomIndexComboBox->setCurrentIndex(mesh->mysurf->geomsurfnum);

    widget->dataLineEdit->setText(mesh->mysurf->potfilename);

    // FIX widget->dataIndexComboBox->setCurrentText(mesh->mysurf->pot
    //data series

    //frame start, end, step
    if (mesh->data)
    {
      widget->startFrameSpinBox->setValue(mesh->mysurf->ts_start+1);
      widget->endFrameSpinBox->setValue(mesh->mysurf->ts_end+1);
      widget->frameStepSpinBox->setValue(mesh->mysurf->ts_sample_step);
      // FIX rms
    }

    widget->leadlinksLineEdit->setText(mesh->mysurf->llfilename);
    widget->landmarksLineEdit->setText(mesh->mysurf->lmfilename);
    widget->channelsLineEdit->setText(mesh->mysurf->chfilename);
    widget->fiducialLineEdit->setText(mesh->mysurf->fidfilename);
  }
  else
  {
    foreach(QLineEdit *lineEdit, findChildren<QLineEdit*>())
    {
      lineEdit->setText("");
    }
    foreach(QSpinBox *spinBox, findChildren<QSpinBox*>())
    {
      spinBox->setValue(1);
    }
  }
}

void FileDialogWidget::addRMSData()
{

}

void FileDialogWidget::updateRMS()
{

}

bool FileDialogWidget::updateFiles()
{
  bool successfulNewSurf = true;
    
  bool meshNotLoaded = (mesh->mysurf == NULL);
  // as far as I can tell, surf, ds, dstart and dend are 1-based to the user,
  // but gs is not
  int surf = property(surfPropName).toInt();
  int win = winComboBox->currentIndex(); // combo box entries are one-based

  // grabbing the char* from a QString will not stay in scope after the QByteArray (toAscii) is destroyed
  //   so we need to copy them out
  char geom[256];
  char data[256];
  char ch[256];
  char ll[256];
  char lm[256];
  char fi[256];

  qDebug() << "Geom:" << geomLineEdit->text() << geomLineEdit->text().toAscii().data();
  strncpy(geom, geomLineEdit->text().toAscii().data(), 256);
  qDebug() << "Geom post:" << geomLineEdit->text() << geomLineEdit->text().toAscii().data() << geom;
  
  // gs should be 0 if the * was selected in a multisurf case, which is what we want 
  int gs = geomIndexComboBox->currentIndex();
  if (gs == geomIndexComboBox->count() - 1)
    gs = -1;
    
  strncpy(data, dataLineEdit->text().toAscii().data(), 256);
  int ds = 0;
  
  if (strcmp(GetExtension(data), ".tsdfc") == 0 || strcmp(GetExtension(data), ".mat") == 0) {
    ds = dataIndexComboBox->currentIndex();
    if (ds == -1)
      ds = 0;
  }
  
  int dstart = startFrameSpinBox->value()-1;
  int dend = endFrameSpinBox->value()-1;
  
  strncpy(ch, channelsLineEdit->text().toAscii().data(), 256);
  strncpy(ll, leadlinksLineEdit->text().toAscii().data(), 256);
  strncpy(lm, landmarksLineEdit->text().toAscii().data(), 256);
  strncpy(fi, fiducialLineEdit->text().toAscii().data(), 256);
  
  if (mesh == 0 && strcmp(geom, "") == 0) {
    // we clicked 'new surface' and 'apply' without putting in a filename
    successfulNewSurf = false;
    return false;
  }

  if (strcmp(geom, "") == 0) {
    // not sure if this needs to be its own special case...
    return false;
  }

  if (!mesh->mysurf && strcmp(geom, "") != 0) {
    //new surface
    GeomWindow* geomwin;
    Surf_Input* input = new Surf_Input;
    Init_Surf_Input(input);
    input->parent = map3d_info.gi;
    input->geomfilename = new char[256];
    input->potfilename = new char[256];
    input->chfilename = new char[256];
    input->lmfilename = new char[256];
    input->llfilename = new char[256];
    input->fidfilename = new char[256];

    
    input->geomsurfnum = gs;
    input->timeseries = ds;
    input->displaysurfnum = surf;
    strcpy(input->geomfilename,geom);
    strcpy(input->potfilename,data);
    strcpy(input->chfilename,ch);
    strcpy(input->llfilename,ll);
    strcpy(input->lmfilename,lm);
    strcpy(input->fidfilename, fi);
    
    input->ts_end = dend;
    input->ts_start = dstart;
    map3d_info.scale_frame_set = 0;
    
    
    // careful - in these next two sections, we are adding windows.
    // we can't assume that windows that we just created have been added to the
    // window manager yet (GeomwindowInit may not have been called yet),
    // so we use map3d_info.geomwins to be on the safe side, though
    // normally not recommended
    Mesh_List currentMeshes;
    currentMeshes.push_back(mesh);
    Mesh_List returnedMeshes = FindAndReadGeom(input,currentMeshes,RELOAD_NONE);
    if (returnedMeshes.size() > 0) {
      if (win >= map3d_info.numGeomwins) {
        // new window as well
        geomwin = GeomWindow::GeomWindowCreate(0,0,0,0);
        
        if (masterWindow && masterWindow->startHidden)
          masterWindow->show();
        
      }
      else
        geomwin = GetGeomWindow(win);
      for (unsigned i = 0; i < returnedMeshes.size(); i++)
        geomwin->addMesh(returnedMeshes[i]);
      map3d_info.lockgeneral = LOCK_FULL;
      geomwin->dominantsurf = -1;
      
      // update the non-files dialogs' information
      // FIX addRowToOtherDialogs();
    }
    else {
      successfulNewSurf = false;
      delete mesh->mysurf;
      mesh->mysurf = 0;
    }
  }
  
  
  if (mesh->gpriv != GetGeomWindow(win)) {
    // move mesh from one window to another
    if (win >= map3d_info.numGeomwins) {
      // new geom window
      GeomWindow* geompriv = GeomWindow::GeomWindowCreate(0,0,0,0);
    }
    GeomWindow* g = mesh->gpriv;
    for (unsigned i = 0; i < g->meshes.size(); i++) {
      // move all meshes with the same surf input to other window (multi-surf geom)
      Mesh_Info* tmp = g->meshes[i];
      if (tmp->mysurf == mesh->mysurf) {
        g->removeMesh(tmp);
        map3d_info.geomwins[win]->addMesh(tmp);
        map3d_info.geomwins[win]->show();
        tmp->gpriv = map3d_info.geomwins[win];
        i--;
      }
    }
    if (g->meshes.size() == 0) {
      g->hide();
    }
  }
  
  // decide if we need to reload the geom - it doesn't compensate for the geomsurfnum anywhere else
  if (gs != mesh->mysurf->geomsurfnum)
    reload_geom = true;
  
  
  if (reload_geom) {
    // reload geom info
    mesh->mysurf->geomfilename = new char[256];
    if (!mesh->mysurf->potfilename) mesh->mysurf->potfilename = new char[256];
    if (!mesh->mysurf->chfilename) mesh->mysurf->chfilename = new char[256];
    if (!mesh->mysurf->lmfilename) mesh->mysurf->lmfilename = new char[256];
    if (!mesh->mysurf->llfilename) mesh->mysurf->llfilename = new char[256];
    if (!mesh->mysurf->fidfilename) mesh->mysurf->fidfilename = new char[256];
    
    strcpy(mesh->mysurf->geomfilename, geom);
    strcpy(mesh->mysurf->chfilename, ch);
    strcpy(mesh->mysurf->llfilename, ll);
    strcpy(mesh->mysurf->lmfilename, lm);
    strcpy(mesh->mysurf->fidfilename, fi);
    
    mesh->mysurf->geomsurfnum = gs;
    printf("Reloading Geom: Surf %d: Win %d, %s@%d, %s@%d, %d-%d\n",surf, win, geom, gs, data, ds, dstart, dend);
    Mesh_List currentMeshes = mesh->gpriv->findMeshesFromSameInput(mesh);
    Mesh_List returnedMeshes = FindAndReadGeom(mesh->mysurf, currentMeshes, RELOAD_GEOM);
    if (returnedMeshes.size() > 0) {
      // change the name of the surface in the save and contour dialogs
      // FIX gtk_entry_set_text(GTK_ENTRY(rowdata->save_input_filename), shorten_filename(mesh->geom->basefilename));
      // gtk_entry_set_text(GTK_ENTRY(rowdata->save_filename), mesh->geom->basefilename); 
      // gtk_entry_set_text(GTK_ENTRY(rowdata->cont_surfname), shorten_filename(mesh->geom->basefilename)); 

      if (meshNotLoaded) {
        mesh->gpriv->addMesh(mesh);
      }
      if (returnedMeshes.size() > currentMeshes.size()) {
        // this is in case we load all surfaces of a multi-surf geom over a surf with less surfaces
        Mesh_List tmp; // pop them off the main list, to keep them in order
        while (mesh->gpriv->meshes.back()->mysurf != mesh->mysurf) {
          tmp.push_back(mesh->gpriv->meshes.back());
          mesh->gpriv->meshes.pop_back();
        }
        
        for (unsigned i = currentMeshes.size(); i < returnedMeshes.size(); i++)
          mesh->gpriv->addMesh(returnedMeshes[i]);
        
        while (tmp.size() > 0) {
          mesh->gpriv->meshes.push_back(tmp.back());
          tmp.pop_back();
        }            
      }
    }
    else {
      mesh->gpriv->removeMesh(mesh);
    }
    //mesh->gpriv = map3d_info.geomwins[win];
    mesh->gpriv->recalcMinMax();
  }
  if (reload_data && strcmp(data, "") != 0) {
    if (!mesh->mysurf->potfilename) mesh->mysurf->potfilename = new char[256];
    strcpy(mesh->mysurf->potfilename, data);
    mesh->mysurf->timeseries = ds;
    mesh->mysurf->ts_end = dend;
    mesh->mysurf->ts_start = dstart;
    map3d_info.scale_frame_set = 0;
    
    printf("Reloading Data: Surf %d: Win %d, %s@%d, %s@%d, %d-%d\n",surf, win, geom, gs, data, ds+1, dstart+1, dend+1);
    
    Mesh_List currentMeshes = mesh->gpriv->findMeshesFromSameInput(mesh);
    FindAndReadGeom(mesh->mysurf, currentMeshes, RELOAD_DATA);
    // FIX updateContourDialogValues(mesh);
    Broadcast(MAP3D_MENU, frame_reset);
  }
  return true;
}

void filesDialogCreate(bool show /*=true*/)
{
  if (!filedialog) {
    filedialog = new FileDialog;
    //filedialog->destroyed = true;

    for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
      Mesh_Info* mesh = mi.getMesh();
      filedialog->addRow(mesh);
    }
  }


  if (show){
    filedialog->show();
    return;
  }
}

FileDialog::FileDialog() :
    QDialog(0, Qt::Dialog | Qt::WindowTitleHint)
{
  setupUi(this);
}

void FileDialog::on_applyButton_clicked()
{
  foreach (FileDialogWidget* widget, _widgets)
  {
    widget->updateFiles();
  }

  Broadcast(MAP3D_UPDATE,0);
  close();
}

void FileDialog::on_cancelButton_clicked()
{
  close();
}

void FileDialog::on_newSurfaceButton_clicked()
{
  addRow(0);
}


FileDialogWidget::FileDialogWidget(QWidget* parent, Mesh_Info* mesh) : QWidget(parent), mesh(mesh)
{
  setupUi(this);
  otherOptionsFrame->setVisible(false);
  reload_geom = 0;
  reload_data = 0;
  mesh = 0;
}
