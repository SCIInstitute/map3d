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
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>

FilesDialog *filedialog = NULL;
FilesSecondaryDialog *fsdialog = NULL;
extern SaveDialog *savedialog;
extern ContourDialog *contourdialog;
extern FidDialog *fiddialog;
extern FidMapDialog *fidmapdialog;
extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
extern FilePicker *fp;
// ------------------- //
// files dialog helpers, callbacks, create, and accessor functions


gint gstrcmp(const void *s1, const void *s2){
  return strcmp( (char*)s1,  (char*)s2);
}


enum filesTableCols {
  SurfNum, WinNum, GeomName, GeomChange, GeomSeries, DataName, DataChange, 
  DataSeries, DataFile, DataStart, DataEnd, GraphTitle, OtherButton, NumCols
};


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

void addNumsToGtkCombo(GtkWidget* widget, int min, int max, bool addStar)
{
  GList *items = NULL;
  for (int i = min; i <= max; i++) {
    char* s = new char[4];
    sprintf(s,"%d",i);
    items = g_list_append(items, s);
  }
  if (addStar) {
    char* s = new char[4];
    sprintf(s,"*");
    items = g_list_append(items, s);
  }
  
  gtk_combo_set_popdown_strings(GTK_COMBO(widget), items);
  gtk_combo_set_value_in_list(GTK_COMBO(widget), true, false);
  gtk_widget_set_size_request(widget, 0, 0); // smallest size possible
}

// callbacks from the files dialog which require a lot of data
class RowCallback : public FilePickerCallback {
public:
  inline RowCallback(int which_func, FilesDialogRowData* data, GtkWidget* entry) : 
  which_func(which_func), data(data), FilePickerCallback(entry) {}
  enum { geomfilechange, geomentrychange, datafilechange, dataentrychange,
    tsdfcfilechange, datastartchange, dataendchange };
  void callback() {
    switch (which_func) {
      case geomfilechange: geomfilecallback(); break;
      case geomentrychange: geomentrycallback(); break;
      case datafilechange: datafilecallback(); break;
      case dataentrychange: dataentrycallback(); break;
      case tsdfcfilechange: tsdfcfilecallback(); break;
      case datastartchange: datastartcallback(); break;
      case dataendchange: dataendcallback(); break;
    }
  }
  
  void geomfilecallback() 
  {
    int num = GetNumGeoms((char*)gtk_entry_get_text(GTK_ENTRY(data->geomname)));
    addNumsToGtkCombo(data->geomseries, 1, num, num > 1);
    
    char* filename = (char*) gtk_entry_get_text(GTK_ENTRY(data->geomname));
    char * slash = shorten_filename(filename);
    gtk_entry_set_text(GTK_ENTRY(data->geomshortname),slash);
    //printf("filename: %s, slash +1: %s\n",filename,slash);
    
    
    data->reload_geom = true;
    
  }
  
  void datafilecallback() 
  {
    char* filename = (char*) gtk_entry_get_text(GTK_ENTRY(data->dataname));
    char * slash = shorten_filename(filename);
    
    gtk_entry_set_text(GTK_ENTRY(data->datashortname),slash);
    if (strcmp(GetExtension(filename),".tsdfc") == 0 ||
        strcmp(GetExtension(filename),".mat") == 0) {
      
      int numseries = GetNumTimeSeries(filename);
      
      GList *items = NULL;
      for(int i = 0; i < numseries; i++){
        char *label = new char[100];
        GetTimeSeriesLabel(filename, i, label);
        items = g_list_append(items, label);
      }
      items = g_list_sort(items, gstrcmp);
      gtk_combo_set_popdown_strings(GTK_COMBO(data->datafile), items);
      gtk_combo_set_value_in_list(GTK_COMBO(data->datafile), true, false);
      
      //      if (strcmp(GetExtension(filename),".tsdfc") == 0) {
      //        string fullname = string(get_path(filename))+
      //	  string((char*)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(data->datafile)->entry)));
      //        filename = const_cast<char*>(fullname.c_str());
      //        gtk_entry_set_text(GTK_ENTRY(data->dataname), filename);
      //      }
      //gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(data->datafile)->entry), "N/A"); // set to ts file from tsdfc 
      gtk_entry_set_text(GTK_ENTRY(data->datanametitle), "Container/Data File");
      gtk_widget_show(data->datafile);
      gtk_widget_show(data->datafiletitle);
    }
    else {
      gtk_widget_hide(data->datafile);
    }
    
    int ds = getGtkComboIndex(data->datafile, filename);
    
    int dstart = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->datastart))-1;
    //int dend = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dataend))-1;
    
    int numframes = GetNumFrames(filename, ds);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->datastart), 1, numframes);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->dataend), 1, numframes);
    
    if (numframes < dstart) 
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->datastart), numframes);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->dataend), numframes);
    
    addRMSData(data); 
    data->reload_data = true;
  }
  
  
  void tsdfcfilecallback() 
  {
    char* filename = (char*) gtk_entry_get_text(GTK_ENTRY(data->dataname));
    char* name = (char*) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(data->datafile)->entry));
    if(strcmp(name,"")!=0){
      //string fullname = string(get_path(filename))+string(name);
      //gtk_entry_set_text(GTK_ENTRY(data->dataname),const_cast<char*>(fullname.c_str()));
      
      int ds = getGtkComboIndex(data->datafile, filename);
      int dstart = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->datastart))-1;
      //int dend = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dataend))-1;
      
      int numframes = GetNumFrames(filename, ds);
      gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->datastart), 1, numframes);
      gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->dataend), 1, numframes);
      
      if (numframes < dstart) 
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->datastart), numframes);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->dataend), numframes);
      
      addRMSData(data); 
      data->reload_data = true;
    }
  }
  
  void datastartcallback()
  {
    gtk_widget_queue_draw(data->rms_curve->drawarea);
    data->de_adj->lower = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->datastart));
    data->reload_data = true;
    
  }
  
  void dataendcallback()
  {
    gtk_widget_queue_draw(data->rms_curve->drawarea);
    data->ds_adj->upper = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->dataend));
    data->reload_data = true;
    
  }
  // this will happen when you change the text in the geom short entry.
  // we need to replace the geomname and then do everything else the 
  // geomcallback does
  void geomentrycallback()
  {
    const char* oldgeom = gtk_entry_get_text(GTK_ENTRY(data->geomname));
    char geom[512];
    replaceFilenameWithShort(oldgeom, gtk_entry_get_text(GTK_ENTRY(data->geomshortname)), geom);
    if (strcmp(oldgeom, geom) != 0) {
      gtk_entry_set_text(GTK_ENTRY(data->geomname), geom);
      geomfilecallback();
    }
  }
  
  // this will happen when you change the text in the data short entry.
  // we need to replace the dataname and then do everything else the 
  // datacallback does
  void dataentrycallback()
  {
    const char* olddata = gtk_entry_get_text(GTK_ENTRY(data->dataname));
    char newdata[256];
    replaceFilenameWithShort(olddata, gtk_entry_get_text(GTK_ENTRY(data->datashortname)), newdata);
    if (strcmp(olddata, newdata) != 0) {
      gtk_entry_set_text(GTK_ENTRY(data->dataname), newdata);
      datafilecallback();
    }
  }
  
  int which_func;
  FilesDialogRowData* data;
};

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

  rowdata->cs_adj = gtk_adjustment_new(rowdata->orig_numspaces,cs_min,surfmax-surfmin,cs_min,cs_min*10,cs_min*10);
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
  
  rowdata->cog_adj = gtk_adjustment_new(surfmax-surfmin, 0, surfmax-surfmin,1,5,5);
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

void addRowToDialogTables(gpointer data)
{
  FilesDialog* fd = (FilesDialog*) data;
  FilesDialogRowData* temprowdata = fd->newSurfaceRowData;
  
  int window = temprowdata->window;
  Mesh_Info* mesh = temprowdata->mesh;
  Mesh_Info* prevrowmesh = 0;
  GtkWidget* datafiletitle = temprowdata->datafiletitle;
  GtkWidget* dataseriestitle = temprowdata->dataseriestitle;
  GtkWidget* datanametitle = temprowdata->datanametitle;
  GtkWidget* table = fd->table;
  int NumRows = ((GtkTable*)table)->nrows;
  
  // don't let there be more than MAX_SURFS surfaces
  if (NumRows >= MAX_SURFS && fd->newsurfacebutton) {
    gtk_widget_hide(fd->newsurfacebutton);
  }
  
  if (fd->rowData.size() > 0)
    prevrowmesh = fd->rowData[fd->rowData.size()-1]->mesh;
  
  // we create a new one here, because on clicking the "New surface" button, we're
  // not guaranteed a new pointer for the row data.  Set the fields manually
  FilesDialogRowData* rowdata = new FilesDialogRowData(mesh, window, dataseriestitle, datafiletitle, datanametitle);
  fd->rowData.push_back(rowdata);
  rowdata->reload_data = rowdata->reload_geom = rowdata->removed = false;
  
  NumRows++;
  gtk_table_resize(GTK_TABLE(table), NumRows, NumCols);
  gtk_table_set_row_spacing(GTK_TABLE(table),NumRows - 1, 2);
  
  int surfnumber;
  
  // we clicked the addRow button
  bool empty_mesh = false;
  if (!mesh) {
    surfnumber = NumRows-1;
    window = numGeomWindows(); // put in in a new window by default
    mesh = new Mesh_Info; // initialize an empty mesh
    rowdata->mesh = mesh;
    empty_mesh = true;
    gtk_widget_set_sensitive(fd->newsurfacebutton, false);
  }
  else {
    surfnumber = mesh->geom->surfnum;
  }
  
  GtkWidget* surfnum = gtk_entry_new(); 
  rowdata->surfnum = surfnum;
  char surf[20];
  sprintf(surf, "%d",surfnumber);
  gtk_entry_set_text(GTK_ENTRY(surfnum), surf); 
  gtk_editable_set_editable(GTK_EDITABLE(surfnum), false);
  gtk_entry_set_width_chars(GTK_ENTRY(surfnum), 3);
  gtk_table_attach_defaults(GTK_TABLE(table), surfnum, SurfNum, SurfNum+1, NumRows-1, NumRows);
  gtk_widget_show(surfnum);
  
  char win[20];
  sprintf(win, "%d",window+1);
  GtkWidget* winnum = gtk_combo_new(); 
  rowdata->winnum = winnum;
  addNumsToGtkCombo(winnum, 1, numGeomWindows()+1, false);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(winnum)->entry), win); 
  gtk_table_attach_defaults(GTK_TABLE(table), winnum, WinNum, WinNum+1, NumRows-1, NumRows);
  gtk_widget_show(winnum);
  
  
  GtkWidget* geomname = gtk_entry_new();
  rowdata->geomname = geomname;
  GtkWidget* geomshortname = gtk_entry_new();
  rowdata->geomshortname = geomshortname;
  
  if (!empty_mesh) {
    gtk_entry_set_text(GTK_ENTRY(geomname), mesh->geom->basefilename); 
    char * slash = shorten_filename(mesh->geom->basefilename);
    gtk_entry_set_text(GTK_ENTRY(geomshortname), slash);
  }
  else {
    // if new row, give it the same geom filename as the row above it.
    if (prevrowmesh) {
      gtk_entry_set_text(GTK_ENTRY(geomname), prevrowmesh->geom->basefilename);
      char * slash = shorten_filename(prevrowmesh->geom->basefilename);
      gtk_entry_set_text(GTK_ENTRY(geomshortname), slash);
    }
  }
  
  gtk_editable_set_editable(GTK_EDITABLE(geomname), true);
  gtk_entry_set_width_chars(GTK_ENTRY(geomname), 25);
  gtk_table_attach_defaults(GTK_TABLE(table), geomname, GeomName, GeomName+1, NumRows-1, NumRows);
  //gtk_widget_show(geomname);
  
  gtk_editable_set_editable(GTK_EDITABLE(geomshortname), true);
  gtk_entry_set_width_chars(GTK_ENTRY(geomshortname), 25);
  gtk_table_attach_defaults(GTK_TABLE(table), geomshortname, GeomName, GeomName+1, NumRows-1, NumRows);
  gtk_widget_show(geomshortname);
  
  
  GtkWidget* geomchange = gtk_button_new_with_label("...");
  gtk_table_attach(GTK_TABLE(table), geomchange, GeomChange, GeomChange+1, NumRows-1, NumRows, GTK_SHRINK, GTK_SHRINK, 0,0);
  gtk_widget_show(geomchange);
  
  char gs[20];
  GtkWidget* geomseries = gtk_combo_new();
  rowdata->geomseries = geomseries;
  if (!empty_mesh) {
    int num = GetNumGeoms(mesh->geom->basefilename);
    addNumsToGtkCombo(geomseries, 1, num, num > 1);
    if (mesh->mysurf->geomsurfnum == 0 && num > 1)
      sprintf(gs, "*");
    else if (num == 1)
      sprintf(gs, "%d", 1);
    else
      sprintf(gs, "%d",mesh->mysurf->geomsurfnum);
  }
  else {
    if (prevrowmesh) {
      int num = GetNumGeoms(prevrowmesh->geom->basefilename);
      addNumsToGtkCombo(geomseries, 1, num, num > 1);
    }
    else
      addNumsToGtkCombo(geomseries, 1, 1, false);
    sprintf(gs, "%d",1);
  }
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(geomseries)->entry), gs); 
  gtk_table_attach_defaults(GTK_TABLE(table), geomseries, GeomSeries, GeomSeries+1, NumRows-1, NumRows);
  gtk_widget_show(geomseries);
  
  GtkWidget* dataname = gtk_entry_new();
  rowdata->dataname = dataname;
  if (mesh && mesh->data){
    gtk_entry_set_text(GTK_ENTRY(dataname), mesh->data->potfilename); 
  }
  else
    gtk_entry_set_text(GTK_ENTRY(dataname), ""); 
  
  gtk_editable_set_editable(GTK_EDITABLE(dataname), true);
  gtk_entry_set_width_chars(GTK_ENTRY(dataname), 25);
  gtk_table_attach_defaults(GTK_TABLE(table), dataname, DataName, DataName+1, NumRows-1, NumRows);
  //gtk_widget_show(dataname);
  
  GtkWidget* datashortname = gtk_entry_new();
  rowdata->datashortname = datashortname;
  if (mesh && mesh->data){
    char *slash = shorten_filename(mesh->data->potfilename);
    gtk_entry_set_text(GTK_ENTRY(datashortname), slash);
  }
  else
    gtk_entry_set_text(GTK_ENTRY(dataname), ""); 
  
  gtk_editable_set_editable(GTK_EDITABLE(datashortname), true);
  gtk_entry_set_width_chars(GTK_ENTRY(datashortname), 25);
  gtk_table_attach_defaults(GTK_TABLE(table), datashortname, DataName, DataName+1, NumRows-1, NumRows);
  gtk_widget_show(datashortname);
  
  
  GtkWidget* datachange = gtk_button_new_with_label("..."); 
  gtk_table_attach(GTK_TABLE(table), datachange, DataChange, DataChange+1, NumRows-1, NumRows, GTK_SHRINK, GTK_SHRINK, 0,0);
  gtk_widget_show(datachange);
  
  char ds[20]= {0};
  if (mesh && mesh->data)
    sprintf(ds, "%d",mesh->data->seriesnum+1);
  GtkWidget* dataseries = gtk_combo_new(); 
  rowdata->dataseries = dataseries;
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(dataseries)->entry), ds); 
  gtk_table_attach_defaults(GTK_TABLE(table), dataseries, DataSeries, DataSeries+1, NumRows-1, NumRows);
  
  GtkWidget* datafile = gtk_combo_new(); 
  rowdata->datafile = datafile;
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(datafile)->entry), false);
  gtk_entry_set_width_chars(GTK_ENTRY(GTK_COMBO(datafile)->entry), 25);
  gtk_table_attach_defaults(GTK_TABLE(table), datafile, DataFile, DataFile+1, NumRows-1, NumRows);
  if (mesh && mesh->data && mesh->mysurf->potfilename && 
      (strcmp(GetExtension(mesh->mysurf->potfilename),".tsdfc") == 0 ||
       strcmp(GetExtension(mesh->mysurf->potfilename),".mat") == 0)) {
    
    char *filename = mesh->mysurf->potfilename;
    
    int numseries = GetNumTimeSeries(filename);
    GList *items = NULL;
    for(int i = 0; i < numseries; i++){
      char *label = new char[100];
      GetTimeSeriesLabel(filename, i, label);
      items = g_list_append(items, label);
    }
    items = g_list_sort(items, gstrcmp);
    gtk_combo_set_popdown_strings(GTK_COMBO(datafile), items);
    gtk_combo_set_value_in_list(GTK_COMBO(datafile), true, false);
    
    gtk_entry_set_text(GTK_ENTRY(datanametitle), "Container/Data File");
    gtk_widget_show(datafile);
    gtk_widget_show(datafiletitle);
  }
  
  int start, end, max;
  if (mesh && mesh->data) {
    start = mesh->data->ts_start;
    end = mesh->data->ts_end;
    max = GetNumFrames(mesh->data->potfilename, mesh->data->seriesnum);
  }
  else {
    start = 0;
    end = 0;
    max = 0;
  }
  GtkAdjustment* ds_adj = (GtkAdjustment*)
    gtk_adjustment_new(start+1,1,max,1,5,5);
  rowdata->ds_adj = ds_adj;
  
  GtkWidget* datastart = gtk_spin_button_new(ds_adj,1,0); 
  rowdata->datastart = datastart;
  gtk_table_attach_defaults(GTK_TABLE(table), datastart, DataStart, DataStart+1, NumRows-1, NumRows);
  gtk_widget_show(datastart);
  
  GtkAdjustment* de_adj = (GtkAdjustment*)
    gtk_adjustment_new(end+1,1,max,1,5,5);
  rowdata->de_adj = de_adj;
  
  GtkWidget* dataend = gtk_spin_button_new(de_adj,1,0); 
  rowdata->dataend = dataend;
  gtk_table_attach_defaults(GTK_TABLE(table), dataend, DataEnd, DataEnd+1, NumRows-1, NumRows);
  gtk_widget_show(dataend);
  
  PickInfo *pick = new PickInfo;
  pick->type = 1;
  pick->show = 1;
  pick->mesh = mesh;
  pick->rms = true;
  
  filedialog->GraphTitle = GraphTitle;
  filedialog->NumRows = NumRows;
  PickWindow *RMS_Curve = filedialog->pickwins[filedialog->numPickwins++];
  rowdata->rms_curve = RMS_Curve;
  
  RMS_Curve->pick = pick;
  RMS_Curve->ready = true;
  RMS_Curve->mesh = 0;
  RMS_Curve->showinfotext = false;
  RMS_Curve->dialogRowData = rowdata;

  rowdata->channelsfilename = gtk_entry_new();
  rowdata->leadlinksfilename = gtk_entry_new();
  rowdata->landmarksfilename = gtk_entry_new();
  rowdata->fidfilename = gtk_entry_new();

  addRMSData(rowdata);
  
  gtk_widget_set_size_request(RMS_Curve->drawarea, 200, 25);
  gtk_table_attach_defaults(GTK_TABLE(table), RMS_Curve->drawarea, GraphTitle, GraphTitle+1, NumRows-1, NumRows);
  gtk_widget_show(RMS_Curve->drawarea);
  
  GtkWidget* others = gtk_button_new_with_label("Other Files"); 
  gtk_table_attach(GTK_TABLE(table), others, OtherButton, OtherButton+1, NumRows-1, NumRows, GTK_SHRINK, GTK_SHRINK, 0,0);
  g_signal_connect_swapped(G_OBJECT(others), "clicked", G_CALLBACK(filesSecondaryDialogCreate), rowdata);
  rowdata->others = others; 
  gtk_widget_show(others);
    
  if (!empty_mesh) {
    addRowToOtherDialogs();
  }
  
  // add advanced callbacks here
  
  // change the geom file from the file picker
  FilePickerCallback *gcb = new RowCallback(RowCallback::geomfilechange, rowdata, geomname);
  filedialog->cbs.push_back(gcb);
  g_signal_connect_swapped(G_OBJECT(geomchange), "clicked", G_CALLBACK(PickFileCb), gcb);
  // add it to the list of file picker widgets so we can remember the directory
  fp->assignEntry(geomname, fp->file_selector_geom);
  
  // change the data file from the file picker
  FilePickerCallback *dcb = new RowCallback(RowCallback::datafilechange, rowdata, dataname);
  filedialog->cbs.push_back(dcb);
  g_signal_connect_swapped(G_OBJECT(datachange), "clicked", G_CALLBACK(PickFileCb), dcb);
  // add it to the list of file picker widgets so we can remember the directory
  fp->assignEntry(dataname, fp->file_selector_data);
  
  // what happens what we change the tsdfc data file
  FilePickerCallback *tcb = new RowCallback(RowCallback::tsdfcfilechange, rowdata, dataname);
  filedialog->cbs.push_back(tcb);
  g_signal_connect_swapped(G_OBJECT(GTK_COMBO(datafile)->entry), "changed", G_CALLBACK(rowcallback), tcb);
  
  // change the geom file from the short filename
  FilePickerCallback *gecb = new RowCallback(RowCallback::geomentrychange, rowdata, geomshortname);
  filedialog->cbs.push_back(gecb);
  g_signal_connect_swapped(G_OBJECT(geomshortname), "activate", G_CALLBACK(rowcallback), gecb);
  g_signal_connect_swapped(G_OBJECT(geomshortname), "focus-out-event", G_CALLBACK(rowcallback), gecb);
  // add it to the list of file picker widgets so we can remember the directory
  fp->assignEntry(geomshortname, fp->file_selector_geom);
  
  // change the data file from the short filename
  FilePickerCallback *decb = new RowCallback(RowCallback::dataentrychange, rowdata, datashortname);
  filedialog->cbs.push_back(decb);
  g_signal_connect_swapped(G_OBJECT(datashortname), "activate", G_CALLBACK(rowcallback), decb);
  g_signal_connect_swapped(G_OBJECT(datashortname), "focus-out-event", G_CALLBACK(rowcallback), decb);
  // add it to the list of file picker widgets so we can remember the directory
  fp->assignEntry(datashortname, fp->file_selector_data);
  
  // change the data window start
  FilePickerCallback *scb = new RowCallback(RowCallback::datastartchange, rowdata, NULL);
  filedialog->cbs.push_back(scb);
  g_signal_connect_swapped(G_OBJECT(datastart),"value-changed", G_CALLBACK(rowcallback),scb);
  
  // change the data window end
  FilePickerCallback *ecb = new RowCallback(RowCallback::dataendchange, rowdata, NULL);
  filedialog->cbs.push_back(ecb);
  g_signal_connect_swapped(G_OBJECT(dataend), "value-changed", G_CALLBACK(rowcallback), ecb);
  
}


void destroyFileDialog(gpointer data)
{
  FilesDialog* fd = (FilesDialog*) data;
  GtkWidget* table = fd->table;
  int NumRows = ((GtkTable*)table)->nrows;
  
  // exit map3d if there you close this window with no surfaces
  if (NumRows == 2 && map3d_info.numGeomwins == 0) {
    map3d_quit(fd->window);
  }
  else {
    gtk_widget_hide(fd->window);
    //int i, win;
    //fd->destroyed = true;
    /*for (i = 0; i < fd->cbs.size(); i++)
    delete fd->cbs[i];
    fd->cbs.clear();
    if (fd->newSurfaceRowData)
    delete fd->newSurfaceRowData;
    for (i = 0; i < fd->rowData.size(); i++)
    delete fd->rowData[i];
    for (win = 0; win < MAX_SURFS; win++) {
      delete filedialog->pickwins[win];
    }
    */
  }
  
}

void UpdateFiles(gpointer data)
{
  
  FilesDialog* fd = (FilesDialog*) data;
  unsigned i;
  bool successfulNewSurf = true;
  for (i = 0; i < fd->rowData.size(); i++) {
    FilesDialogRowData* rowdata = fd->rowData[i];
    GtkWidget* surfnum = rowdata->surfnum;
    GtkWidget* winnum = rowdata->winnum;
    GtkWidget* geomname = rowdata->geomname;
    GtkWidget* geomseries = rowdata->geomseries;
    GtkWidget* dataname = rowdata->dataname;
    GtkWidget* datafile = rowdata->datafile;
    GtkWidget* datastart = rowdata->datastart;
    GtkWidget* dataend = rowdata->dataend;
    GtkWidget* channelsname = rowdata->channelsfilename;
    GtkWidget* leadlinksname = rowdata->leadlinksfilename;
    GtkWidget* landmarksname = rowdata->landmarksfilename;
    GtkWidget* fidname = rowdata->fidfilename;
    
    // as far as I can tell, surf, ds, dstart and dend are 1-based to the user,
    // but gs is not
    int surf = atoi(gtk_entry_get_text(GTK_ENTRY(surfnum)));
    int win = atoi(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(winnum)->entry)))-1;
    
    const char* geom = gtk_entry_get_text(GTK_ENTRY(geomname));
    
    // gs should be 0 if the * was selected in a multisurf case, which is what we want 
    int gs = atoi(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(geomseries)->entry)));
    char* data = (char*) gtk_entry_get_text(GTK_ENTRY(dataname));
    int ds = 0;
    
    if (strcmp(GetExtension(data), ".tsdfc") == 0 || strcmp(GetExtension(data), ".mat") == 0) {
      ds = getGtkComboIndex(datafile,(char*) gtk_entry_get_text(GTK_ENTRY(rowdata->dataname)));
      if (ds == -1)
        ds = 0;
    }
    
    int dstart = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(datastart))-1;
    int dend = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dataend))-1;
    
    char* ch = (char*)gtk_entry_get_text(GTK_ENTRY(channelsname));
    char* ll = (char*)gtk_entry_get_text(GTK_ENTRY(leadlinksname));
    char* lm = (char*)gtk_entry_get_text(GTK_ENTRY(landmarksname));
    char* fi = (char*)gtk_entry_get_text(GTK_ENTRY(fidname));
    
    if (!rowdata->mesh->mysurf && strcmp(geom, "") == 0) {
      // we clicked 'new surface' and 'apply' without putting in a filename
      successfulNewSurf = false;
      continue;
    }
    if (!rowdata->mesh->mysurf && strcmp(geom, "") != 0) {
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
      currentMeshes.push_back(rowdata->mesh);
      Mesh_List returnedMeshes = FindAndReadGeom(input,currentMeshes,RELOAD_NONE);
      if (returnedMeshes.size() > 0) {
        if (win >= map3d_info.numGeomwins) {
          // new window as well
          geomwin = GeomWindow::GeomWindowCreate(0,0,0,0, 256, 256);
          geomwin->setPosAndShow();
          
          if (masterWindow && masterWindow->startHidden)
            gtk_widget_show(GTK_WIDGET(masterWindow->window));
          
        }
        else
          geomwin = map3d_info.geomwins[win];
        for (unsigned i = 0; i < returnedMeshes.size(); i++)
          geomwin->addMesh(returnedMeshes[i]);
        map3d_info.lockgeneral = LOCK_FULL;
        geomwin->dominantsurf = -1;
        
        // update the non-files dialogs' information
        addRowToOtherDialogs();
      }
      else {
        successfulNewSurf = false;
        delete rowdata->mesh->mysurf;
        rowdata->mesh->mysurf = 0;
      }
      
      continue;
    }
    
    
    Mesh_Info* mesh = rowdata->mesh;
    
    if (mesh->gpriv != GetGeomWindow(win)) {
      // move mesh from one window to another
      if (win >= map3d_info.numGeomwins) {
        // new geom window
        GeomWindow* geompriv = GeomWindow::GeomWindowCreate(0,0,0,0, 256, 256);
        geompriv->setPosAndShow();
      }
      GeomWindow* g = mesh->gpriv;
      for (unsigned i = 0; i < g->meshes.size(); i++) {
        // move all meshes with the same surf input to other window (multi-surf geom)
        Mesh_Info* tmp = g->meshes[i];
        if (tmp->mysurf == mesh->mysurf) {
          g->removeMesh(tmp);
          map3d_info.geomwins[win]->addMesh(tmp);
          gtk_widget_show(map3d_info.geomwins[win]->window);
          tmp->gpriv = map3d_info.geomwins[win];
          i--;
        }
      }
      if (g->meshes.size() == 0) {
        gtk_widget_hide(g->window);
      }
    }
    
    // decide if we need to reload the geom - it doesn't compensate for the geomsurfnum anywhere else
    if (gs != rowdata->mesh->mysurf->geomsurfnum)
      rowdata->reload_geom = true;
    
    
    if (rowdata->reload_geom) {
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
        gtk_entry_set_text(GTK_ENTRY(rowdata->save_input_filename), shorten_filename(mesh->geom->basefilename));
        gtk_entry_set_text(GTK_ENTRY(rowdata->save_filename), mesh->geom->basefilename); 
        gtk_entry_set_text(GTK_ENTRY(rowdata->cont_surfname), shorten_filename(mesh->geom->basefilename)); 

        if (rowdata->removed) {
          rowdata->removed = false;
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
          // since there are more meshes now, we need to load their data
          rowdata->reload_data = true;
        }
      }
      else {
        rowdata->removed = true;
        mesh->gpriv->removeMesh(mesh);
      }
      mesh->gpriv = map3d_info.geomwins[win];
      mesh->gpriv->recalcMinMax();
    }
    if (rowdata->reload_data) {
      if (!mesh->mysurf->potfilename) mesh->mysurf->potfilename = new char[256];
      strcpy(mesh->mysurf->potfilename, data);
      mesh->mysurf->timeseries = ds;
      mesh->mysurf->ts_end = dend;
      mesh->mysurf->ts_start = dstart;
      map3d_info.scale_frame_set = 0;
      
      printf("Reloading Data: Surf %d: Win %d, %s@%d, %s@%d, %d-%d\n",surf, win, geom, gs, data, ds+1, dstart+1, dend+1);
      
      Mesh_List currentMeshes = mesh->gpriv->findMeshesFromSameInput(mesh);
      FindAndReadGeom(mesh->mysurf, currentMeshes, RELOAD_DATA);
      updateContourDialogValues(mesh);
      menu_data md(frame_reset, mesh->gpriv);
      Broadcast(MAP3D_MENU, &md);
    }
    
  }
  Broadcast(MAP3D_UPDATE,0);
  
  // pass through the rows and get the window# combo box to each have the max num of windows possible
  // and set the reload flags to false
  for (i = 0; i < fd->rowData.size(); i++) {
    FilesDialogRowData* rowdata = fd->rowData[i];
    rowdata->reload_data = rowdata->reload_geom = false;
    GtkWidget* winnum = rowdata->winnum;
    int window = atoi(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(winnum)->entry)))-1;
    char win[20];
    sprintf(win, "%d",window+1);
    addNumsToGtkCombo(winnum, 1, map3d_info.numGeomwins+1, false);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(winnum)->entry), win); 
  }
  //ComputeLockFrameData();
  if (successfulNewSurf)
    gtk_widget_set_sensitive(fd->newsurfacebutton, true);
}

void filesDialogCreate(bool show /*=true*/)
{
  if (!filedialog) {
    filedialog = new FilesDialog;
    //filedialog->destroyed = true;
  }
  else if (show){
    gtk_widget_show(filedialog->window);
    return;
  }
  else
    return;
  
  filedialog->numPickwins = 0;
  
  // create this widget early so we can set its sensitivity when we create the rows
  // on an empty files window
  GtkWidget* addRow = gtk_button_new_with_label("New Surface");
  filedialog->newsurfacebutton = addRow;
  for (int win = 0; win < MAX_SURFS; win++) {
    filedialog->pickwins[win] = new PickWindow(true);
  }
  
  
  GtkWidget *vbox;
  filedialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  //gtk_window_set_keep_above(GTK_WINDOW(filedialog->window), true);
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_widget_show(vbox);
  
  gtk_window_set_title(GTK_WINDOW(filedialog->window), "Map3D Files");
  gtk_window_set_resizable(GTK_WINDOW(filedialog->window), false);
  gtk_widget_set_sensitive(filedialog->window, true);
  if (fp && fp->active)
    gtk_widget_set_sensitive(filedialog->window, false);
  
  filedialog->sensitive = true;
  g_signal_connect_swapped(G_OBJECT(filedialog->window), "delete_event", G_CALLBACK(destroyFileDialog),
                           gpointer(filedialog));
  
  gtk_container_add(GTK_CONTAINER(filedialog->window), vbox);
  
  GtkWidget* table = gtk_table_new(1, NumCols, FALSE);
  filedialog->table = table;
  gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
  gtk_widget_show(table);
  
  
  //set header row
  GtkWidget* surfnumtitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(surfnumtitle), "Surf"); 
  gtk_editable_set_editable(GTK_EDITABLE(surfnumtitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(surfnumtitle), 6);
  gtk_table_attach_defaults(GTK_TABLE(table), surfnumtitle, SurfNum, SurfNum+1, 0,1);
  gtk_widget_show(surfnumtitle);
  GtkWidget* winnumtitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(winnumtitle), "Win#"); 
  gtk_editable_set_editable(GTK_EDITABLE(winnumtitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(winnumtitle), 6);
  gtk_table_attach_defaults(GTK_TABLE(table), winnumtitle, WinNum, WinNum+1, 0,1);
  gtk_widget_show(winnumtitle);
  
  GtkWidget* geomnametitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geomnametitle), "Geom File"); 
  gtk_editable_set_editable(GTK_EDITABLE(geomnametitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(geomnametitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), geomnametitle, GeomName, GeomName+2, 0,1);
  gtk_widget_show(geomnametitle);
  
  GtkWidget* geomseriestitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(geomseriestitle), "Geom#"); 
  gtk_editable_set_editable(GTK_EDITABLE(geomseriestitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(geomseriestitle), 8);
  gtk_table_attach_defaults(GTK_TABLE(table), geomseriestitle, GeomSeries, GeomSeries+1, 0,1);
  gtk_widget_show(geomseriestitle);
  
  GtkWidget* datanametitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(datanametitle), "Time Series File"); // change to container if there are tsdfc's
  gtk_editable_set_editable(GTK_EDITABLE(datanametitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(datanametitle), 30);
  gtk_table_attach_defaults(GTK_TABLE(table), datanametitle, DataName, DataName+2, 0,1);
  gtk_widget_show(datanametitle);
  
  GtkWidget* dataseriestitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(dataseriestitle), "Data#"); 
  gtk_editable_set_editable(GTK_EDITABLE(dataseriestitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(dataseriestitle), 7);
  gtk_table_attach_defaults(GTK_TABLE(table), dataseriestitle, DataSeries, DataSeries+1, 0,1);
  //don't show this unless we have a container
  
  GtkWidget* datafiletitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(datafiletitle), "Time Series File"); 
  gtk_editable_set_editable(GTK_EDITABLE(datafiletitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(datafiletitle), 15);
  gtk_table_attach_defaults(GTK_TABLE(table), datafiletitle, DataFile, DataFile+1, 0,1);
  //don't show this unless we have a container
  
  GtkWidget* datastarttitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(datastarttitle), "Start Frame"); 
  gtk_editable_set_editable(GTK_EDITABLE(datastarttitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(datastarttitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), datastarttitle, DataStart, DataStart+1, 0,1);
  gtk_widget_show(datastarttitle);
  
  GtkWidget* dataendtitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(dataendtitle), "End Frame"); 
  gtk_editable_set_editable(GTK_EDITABLE(dataendtitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(dataendtitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), dataendtitle, DataEnd, DataEnd+1, 0,1);
  gtk_widget_show(dataendtitle);
  
  GtkWidget* graphtitle = gtk_entry_new(); 
  gtk_entry_set_text(GTK_ENTRY(graphtitle), "Graph"); 
  gtk_editable_set_editable(GTK_EDITABLE(graphtitle), false);
  gtk_entry_set_width_chars(GTK_ENTRY(graphtitle), 12);
  gtk_table_attach_defaults(GTK_TABLE(table), graphtitle, GraphTitle, GraphTitle+1, 0,1);
  gtk_widget_show(graphtitle);
  
  // make sure that other dialogs that need this data have been created.
  saveDialogCreate(false);
  contourDialogCreate(false);
  fidDialogCreate(false);
  fidMapDialogCreate(false);
  
  // use map3d_info.geomwins instead of GetGeomWin because the master window might make
  // it so the window isn't created yet...
  for (int i = 0; i < map3d_info.numGeomwins; i++) {
    GeomWindow* geomwin = map3d_info.geomwins[i];
    for (unsigned j = 0; j < geomwin->meshes.size(); j++) {
      Mesh_Info* mesh = geomwin->meshes[j];
      if (j > 0 && mesh->mysurf == geomwin->meshes[j-1]->mysurf)
        // don't include it if it was part of a multisurf geom read we already have
        continue;
      FilesDialogRowData rowdata(mesh, i, dataseriestitle, datafiletitle, datanametitle);
      filedialog->newSurfaceRowData = &rowdata;
      addRowToDialogTables(filedialog);
    }
  }
  
  FilesDialogRowData* rowdata = new FilesDialogRowData(0, -1, dataseriestitle, datafiletitle, datanametitle);
  filedialog->newSurfaceRowData = rowdata;
  
  if (map3d_info.numGeomwins == 0)
    addRowToDialogTables(filedialog);
  
  // add update/cancel buttons
  GtkWidget* hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
  gtk_widget_show(hbox);
  
  GtkWidget *update, *cancel;
  
  update = gtk_button_new_with_label("Apply");
  cancel = gtk_button_new_with_label("Close");
  gtk_widget_show(update);
  gtk_widget_show(cancel);
  
  if (((GtkTable*)table)->nrows <= MAX_SURFS) {
    gtk_widget_show(addRow);
  }
  
  g_signal_connect_swapped(G_OBJECT(update), "clicked", G_CALLBACK(UpdateFiles), (gpointer)filedialog);
  //g_signal_connect_swapped(G_OBJECT(update), "clicked", G_CALLBACK(destroyFileDialog), (gpointer)filedialog);
  g_signal_connect_swapped(G_OBJECT(cancel), "clicked", G_CALLBACK(destroyFileDialog), (gpointer)filedialog);
  g_signal_connect_swapped(G_OBJECT(addRow), "clicked", G_CALLBACK(addRowToDialogTables), (gpointer)filedialog);
  
  gtk_box_pack_end(GTK_BOX(hbox), update, FALSE, FALSE, 2);
  gtk_box_pack_end(GTK_BOX(hbox), cancel, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), addRow, FALSE, FALSE, 2);
  
  
  filedialog->destroyed = false;
  if (show)
    gtk_widget_show(filedialog->window);
}

void fileSecondaryClose(FilesSecondaryDialog* fsd)
{
  gtk_widget_set_sensitive(filedialog->window, true);
  filedialog->sensitive = true;
  gtk_widget_hide(fsd->window);
}
void fileSecondaryUpdate(FilesSecondaryDialog* fsd)
{
  FilesDialogRowData* data = fsd->data;
  gtk_entry_set_text(GTK_ENTRY(data->channelsfilename),gtk_entry_get_text(GTK_ENTRY(fsdialog->channels)));
  gtk_entry_set_text(GTK_ENTRY(data->leadlinksfilename),gtk_entry_get_text(GTK_ENTRY(fsdialog->leadlinks)));
  gtk_entry_set_text(GTK_ENTRY(data->landmarksfilename),gtk_entry_get_text(GTK_ENTRY(fsdialog->landmarks)));
  gtk_entry_set_text(GTK_ENTRY(data->fidfilename),gtk_entry_get_text(GTK_ENTRY(fsdialog->fids)));
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(fsdialog->nv)))
    map3d_info.qnovalidity = true;
  data->reload_geom = true;
  fileSecondaryClose(fsd);
}

void filesSecondaryDialogCreate(FilesDialogRowData* data)
{
  if (fsdialog == NULL) {
    fsdialog = new FilesSecondaryDialog;
    fsdialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(fsdialog->window), GTK_WINDOW(filedialog->window));
    //gtk_window_set_modal(GTK_WINDOW(fsdialog->window), true);
    gtk_window_set_title(GTK_WINDOW(fsdialog->window), "Other files...");
    gtk_window_set_resizable(GTK_WINDOW(fsdialog->window), false);
    gtk_widget_set_sensitive(fsdialog->window, true);
    if (fp && fp->active)
      gtk_widget_set_sensitive(fsdialog->window, false);
    
    g_signal_connect_swapped(G_OBJECT(fsdialog->window), "delete_event", G_CALLBACK(fileSecondaryClose),
                             fsdialog);
    
    // have this (and filepicker) be inactive when child window open
    // Files -> this or File picker, this -> file picker
    
    GtkWidget* vbox = gtk_vbox_new(FALSE, 2);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(fsdialog->window), vbox);
    
    GtkWidget* table = gtk_table_new(4, 3, FALSE);
    gtk_widget_show(table);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
    
    GtkWidget* lb1 = gtk_label_new("Channels File");
    gtk_widget_show(lb1);
    GtkWidget* b1 = gtk_button_new_with_label("...");
    gtk_widget_show(b1);
    
    fsdialog->channels = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(fsdialog->channels), 65);
    gtk_table_attach_defaults(GTK_TABLE(table), lb1, 0,1,0,1);
    gtk_table_attach_defaults(GTK_TABLE(table), fsdialog->channels, 1,2,0,1);
    gtk_table_attach_defaults(GTK_TABLE(table), b1, 2,3,0,1);
    g_signal_connect_swapped(G_OBJECT(b1), "clicked", G_CALLBACK(PickFileWidget), (gpointer)fsdialog->channels);
    // add it to the list of file picker widgets so we can remember the directory
    fp->assignEntry(fsdialog->channels, fp->file_selector_geom);
    
    gtk_widget_show(fsdialog->channels);
    
    GtkWidget* lb2 = gtk_label_new("Leadlinks File");
    gtk_widget_show(lb2);
    GtkWidget* b2 = gtk_button_new_with_label("...");
    gtk_widget_show(b2);
    
    fsdialog->leadlinks = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(fsdialog->leadlinks), 65);
    gtk_table_attach_defaults(GTK_TABLE(table), lb2, 0,1,1,2);
    gtk_table_attach_defaults(GTK_TABLE(table), fsdialog->leadlinks, 1,2,1,2);
    gtk_table_attach_defaults(GTK_TABLE(table), b2, 2,3,1,2);
    g_signal_connect_swapped(G_OBJECT(b2), "clicked", G_CALLBACK(PickFileWidget), (gpointer)fsdialog->leadlinks);
    // add it to the list of file picker widgets so we can remember the directory
    fp->assignEntry(fsdialog->leadlinks, fp->file_selector_geom);
    gtk_widget_show(fsdialog->leadlinks);
    
    GtkWidget* lb3 = gtk_label_new("Landmarks File");
    gtk_widget_show(lb3);
    GtkWidget* b3 = gtk_button_new_with_label("...");
    gtk_widget_show(b3);
    
    fsdialog->landmarks = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(fsdialog->landmarks), 65);
    gtk_table_attach_defaults(GTK_TABLE(table), lb3, 0,1,2,3);
    gtk_table_attach_defaults(GTK_TABLE(table), fsdialog->landmarks, 1,2,2,3);
    gtk_table_attach_defaults(GTK_TABLE(table), b3, 2,3,2,3);
    g_signal_connect_swapped(G_OBJECT(b3), "clicked", G_CALLBACK(PickFileWidget), (gpointer)fsdialog->landmarks);
    // add it to the list of file picker widgets so we can remember the directory
    fp->assignEntry(fsdialog->landmarks, fp->file_selector_geom);
    gtk_widget_show(fsdialog->landmarks);
    
    GtkWidget* lb4 = gtk_label_new("Fiducial File");
    gtk_widget_show(lb4);
    GtkWidget* b4 = gtk_button_new_with_label("...");
    gtk_widget_show(b4);
    
    fsdialog->fids = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(fsdialog->fids), 65);
    gtk_table_attach_defaults(GTK_TABLE(table), lb4, 0,1,3,4);
    gtk_table_attach_defaults(GTK_TABLE(table), fsdialog->fids, 1,2,3,4);
    gtk_table_attach_defaults(GTK_TABLE(table), b4, 2,3,3,4);
    g_signal_connect_swapped(G_OBJECT(b4), "clicked", G_CALLBACK(PickFileWidget), (gpointer)fsdialog->fids);
    // add it to the list of file picker widgets so we can remember the directory
    fp->assignEntry(fsdialog->fids, fp->file_selector_geom);
    gtk_widget_show(fsdialog->fids);
    
    GtkWidget* lb5 = gtk_label_new("Other Options:");
    gtk_widget_show(lb5);
    
    fsdialog->nv = gtk_check_button_new_with_label ("-nv");
    gtk_table_attach_defaults(GTK_TABLE(table), lb5, 0,1,4,5);
    gtk_table_attach_defaults(GTK_TABLE(table), fsdialog->nv, 1,2,4,5);
    
    gtk_widget_show(fsdialog->nv);
    
    GtkWidget* hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    gtk_widget_show(hbox);
    
    GtkWidget *update, *cancel;
    
    update = gtk_button_new_with_label("Apply");
    cancel = gtk_button_new_with_label("Close");
    gtk_box_pack_start(GTK_BOX(hbox), update, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), cancel, FALSE, FALSE, 2);
    gtk_widget_show(update);
    gtk_widget_show(cancel);
    
    g_signal_connect_swapped(G_OBJECT(cancel), "clicked", G_CALLBACK(fileSecondaryClose),
                             fsdialog);
    g_signal_connect_swapped(G_OBJECT(update), "clicked", G_CALLBACK(fileSecondaryUpdate),
                             fsdialog);
  }
  gtk_widget_set_sensitive(filedialog->window, false);
  filedialog->sensitive = false;
  gtk_widget_show(fsdialog->window);
  fsdialog->data = data;
  
  // copy mesh stuff to the entries
  gtk_entry_set_text(GTK_ENTRY(fsdialog->channels),gtk_entry_get_text(GTK_ENTRY(data->channelsfilename)));
  gtk_entry_set_text(GTK_ENTRY(fsdialog->leadlinks),gtk_entry_get_text(GTK_ENTRY(data->leadlinksfilename)));
  gtk_entry_set_text(GTK_ENTRY(fsdialog->landmarks),gtk_entry_get_text(GTK_ENTRY(data->landmarksfilename)));
  
  if(map3d_info.qnovalidity)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(fsdialog->nv), TRUE);
  
}
