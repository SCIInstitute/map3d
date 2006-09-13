/* dialogs.cxx */

#ifdef _WIN32
#  include <windows.h>
#  pragma warning(disable:4505)
#  undef TRACE
#endif

#include <gtk/gtk.h>
#include "dialogs.h"
#include "map3d-struct.h"
#include "GeomWindowMenu.h"
#include "Contour_Info.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "MainWindow.h"
#include "Map3d_Geom.h"
#include "ProcessCommandLineOptions.h"
#include "ParseCommandLineOptions.h"
#include "Surf_Data.h"
#include "WindowManager.h"
#include "eventdata.h"
#include "Transforms.h"
#include "savescreen.h"
#include "savestate.h"
#include "PickWindow.h"
#include "pickinfo.h"
#include "scalesubs.h"
#include "tsdfcio.h"
#include "readfiles.h"
#include <math.h>
#include <stdio.h>
#include <ctype.h>

// dialog globals
GtkWidget *quitwindow = NULL;
ColorPicker *cp = NULL;
SizePicker *sp = NULL;
FilePicker *fp = NULL;
FrameDialog *framedialog = NULL;
FogDialog *fogdialog = NULL;
extern FilesDialog *filedialog;
extern FilesSecondaryDialog *fsdialog;
extern SaveDialog *savedialog;
extern int fstep;
extern FileCache file_cache;


// will bring up a dialog and either quit or do nothing
void map3d_quit(gpointer parentWindow)
{
  GtkWindow *parent = (GtkWindow *) parentWindow;
  if (!quitwindow) {
    quitwindow = (GtkWidget *) gtk_dialog_new_with_buttons(NULL, parent, (GtkDialogFlags)0,
                                                           GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    GtkWidget* label = gtk_label_new ("Really Quit?");
    gtk_widget_show(label);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(quitwindow)->vbox),
                       label);
  }
  if (gtk_dialog_run(GTK_DIALOG(quitwindow)) == GTK_RESPONSE_OK)
    exit(0);
  else
    gtk_widget_hide(quitwindow);
}

double roundedNum(double num){
  
  double x;
  double mant, ipart;
  
  x = (num/(pow(10.0,(double)(((long)log10(fabs(num)))-1))));
  mant = modf(x, &ipart);
  
  //x = round(x); not sgi compatible
  if(fabs(mant) >= 0.5){
    if(x > 0){
      x = ceil(x);
    }
    else{
      x = floor(x);
    }
  }
  else{
    if(x > 0){
      x = floor(x);
    }
    else{
      x = ceil(x);
    }
  }
  
  x = x*(pow(10.0,(double)((long)log10(fabs(num)) -1)));
  
  return x;
}

char* shorten_filename(char* x)
{
  // get rid of the leading directories in filename - find last occurrence of / or '\'
  char * slash = strrchr(x, '/');
  if (!slash) {
    slash = strrchr(x,'\\');
  }
  if (!slash)
    slash = x;
  else
    slash = slash+1; // to get rid of the /
  return slash;
}

string get_path(char* x)
{
  // get rid of the leading directories in filename - find last occurrence of / or '\'
  char * slash = strrchr(x, '/');
  string path(x);
  if (!slash) {
    slash = strrchr(x,'\\');
  }
  if (!slash)
    slash = x;
  else
    slash = slash +1; // to get rid of the /
  int i = path.find(slash);
  path.erase(i);
  return path;
}

void parse_filename(char* n, char* initials, char* date, char* info, char* num){
  int i = 0;
  char temp;
  char* ptr;
  
  //find initials
  temp = n[i];
  while(!isdigit(temp)){
    i++;
    temp = n[i];
  }
  strncpy(initials,n, i);
  initials[i] = '\0';
  
  //ptr = n(filename) - initials
  ptr = strchr(n, temp);
  //printf("%s\n",ptr);
  
  //find date
  strncpy(date,ptr,7);
  date[7] = '\0';
  
  //ptr = n(filename) - initials - date
  ptr = ptr + 8;
  //printf("%s\n",ptr);
  
  
  //find info
  i = strlen(ptr) -8;
  strncpy(info,ptr,i);
  info[i - 1] = '\0';
  
  //ptr = n(filename) - initials - date - info
  ptr = ptr +i;
  //printf("%s\n",ptr);
  
  //find run number
  i = strlen(ptr) - 5;
  strncpy(num,ptr,i);
  num[i] = '\0';
  
  //ptr = n(filename) - initials - date - info - run number
  ptr = ptr +i;
  //printf("%s\n",ptr);
  
  //   printf("filename = %s\n",n);
  //   printf("initials = %s\n",initials);
  //   printf("date = %s\n",date);
  //   printf("info = %s\n",info);
  //   printf("num = %s\n",num);
}


void replaceFilenameWithShort(const char* longname, const char* shortname, char* combinedname)
{
  char tmp[256];
  strcpy(tmp, longname);
  bool slash = false;
  int i;
  // look for a slash in longname, so we can append shortname to that current dir
  for (i = strlen(tmp)-1; i >= 0; i--)
    if (tmp[i] == '/' || tmp[i] == '\\') {
      slash = true;
      break;
    }
      tmp[i] = 0;
  
  // if shortname is an absolute path, then only use that
  // unix, if it starts with /
  // windows, if it is X:, where X is a drive letter.
  if (shortname[0] == '/' || shortname[1] == ':')
    slash = false;
  
  if (slash)
    sprintf(combinedname, "%s/%s", tmp, shortname);
  else
    strcpy(combinedname, shortname);
}



int getGtkComboIndex(GtkWidget* combo)
{
  // this is kind of hacky - I was hoping for GtkComboBox functionality
  // but that's not available until Gtk 2.4, and we want to maintain backward
  // compatibility until 2.0.6
  char* name = (char*) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
  GList* list = GTK_LIST(GTK_COMBO(combo)->list)->children;
  int index = 0;
  while (list != NULL) {
    // this should grab the label from the list
    GtkLabel* label = GTK_LABEL(GTK_BIN(&GTK_ITEM(&GTK_LIST_ITEM(list->data)->item)->bin)->child);
    if (strcmp(label->text, name) == 0)
      return index;
    index++;
    list = list->next;
  }
  return -1;
}

int getGtkComboIndex(GtkWidget* combo, char* filename)
{
  // get the correct timeseries number since the gtkcombo is sorted by name.
  char* name = (char*) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
  int index = 0;
  if (strcmp(GetExtension(filename), ".tsdfc") == 0){ 
    Container *c = file_cache.readContainerFile(string(filename));
    if(c){
      string entryName;
      for(index = 0; index < c->numEntries;index++){
        entryName = c->entryArray[index]->entryName;
        if(strcmp(entryName.c_str(), name)==0)
          return index;
      }
    }
  }
  else if (strcmp(GetExtension(filename),".mat") == 0) {
    try {
      matlabfile mf(filename, "r");
      matlabarray ma = mf.getmatlabarray(0);
      if (!ma.isempty()) {
        if (ma.isstruct()) {
          for(index = 0; index < ma.getnumelements(); index++){
            matlabarray label = ma.getfieldCI(index, "label");
            if((!label.isempty() && strcmp(name,label.getstring().c_str())==0) || index+1 == atoi(name)){
              return index;
            }
          }
        }
        else if (ma.iscell()) {
          for(index = 0; index < ma.getnumelements(); index++){
            matlabarray label = ma.getcell(index).getfieldCI(index, "label");
            if((!label.isempty() && strcmp(name,label.getstring().c_str())==0) || index+1 == atoi(name)){
              return index;
            }
          }
        }
        else if (ma.isdense()) {
          return 0;
        }
      }
    } catch (...) {
      // do nothing if matlab file fails, just use the default series label
      // (a warning will appear when you actually try to read the data
    }
  }
  return -1;
}





extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;

gint ColorPickerSetColor(GtkWidget*, GdkEvent*, gpointer data) {
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
  
  GdkColor color;
  color.red = (guint16) (r * 65535);
  color.green = (guint16) (g * 65535);
  color.blue = (guint16) (b * 65535);
  cp->selected_color[0]=r;
  cp->selected_color[1]=g;
  cp->selected_color[2]=b;
  gtk_widget_modify_bg(cp->selected_color_widget, GTK_STATE_NORMAL, &color);
  gtk_widget_queue_draw(cp->selected_color_widget);
  return 0;
}

void ColorPickerCancel(GtkWidget*)
{
  for (unsigned int i = 0; i < cp->cs_float.size(); i++) {
    cp->cs_float[i][0] = cp->cs_orig_vals[i][0];
    cp->cs_float[i][1] = cp->cs_orig_vals[i][1];
    cp->cs_float[i][2] = cp->cs_orig_vals[i][2];
    delete cp->cs_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per cs_float
  }
  cp->cs_float.clear();
  cp->cs_orig_vals.clear();
  gtk_widget_hide(cp->colorpicker);
  Broadcast(MAP3D_REPAINT_ALL, 0);
}

void ColorPickerPreview(GtkWidget*)
{
  
  for (unsigned int i = 0; i < cp->cs_float.size(); i++) {
    cp->cs_float[i][0] = cp->selected_color[0];
    cp->cs_float[i][1] = cp->selected_color[1];
    cp->cs_float[i][2] = cp->selected_color[2];
  }
  Broadcast(MAP3D_REPAINT_ALL, 0);
}

void ColorPickerOkay(GtkWidget* widget)
{
  ColorPickerPreview(widget);
  for (unsigned int i = 0; i < cp->cs_float.size(); i++) {
    delete cp->cs_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per cs_float
  }
  cp->cs_float.clear();
  cp->cs_orig_vals.clear();
  gtk_widget_hide(cp->colorpicker);
}

void ColorPickerCreate()
{
  if (cp == NULL) {
    cp = new ColorPicker;
    cp->post_change = false;
    
    // create the window and set window parameters
    cp->colorpicker = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(cp->colorpicker), "Select Color");
    gtk_window_set_resizable(GTK_WINDOW(cp->colorpicker), false);
    g_signal_connect(G_OBJECT(cp->colorpicker), "delete_event", G_CALLBACK(gtk_widget_hide),
                    NULL);
    
    // add sub-boxes
    GtkWidget* vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(cp->colorpicker), vbox);
    
    GtkWidget* hbox_color_cols = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_color_cols, FALSE, FALSE, 2);
    
    GtkWidget* hbox_color_cols2 = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_color_cols2, FALSE, FALSE, 2);
    
    GdkColor color;
    
    // add last row and selected color box into it.
    GtkWidget* hbox = gtk_hbox_new(TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    
    cp->orig_color_widget = gtk_drawing_area_new();
    cp->selected_color_widget = gtk_drawing_area_new();
    
    gtk_widget_set_size_request(cp->selected_color_widget, 100, 25);
    gtk_widget_set_size_request(cp->orig_color_widget, 100, 25);
    
    gtk_box_pack_start(GTK_BOX(hbox), cp->orig_color_widget, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), cp->selected_color_widget, FALSE, FALSE, 2);
    
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
        g_signal_connect(G_OBJECT(frame), "button_press_event", G_CALLBACK(ColorPickerSetColor), (void*)i);
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
    
    g_signal_connect(G_OBJECT(cancel), "clicked", G_CALLBACK(ColorPickerCancel), NULL);
    g_signal_connect(G_OBJECT(okay), "clicked", G_CALLBACK(ColorPickerOkay), NULL);
    g_signal_connect(G_OBJECT(preview), "clicked", G_CALLBACK(ColorPickerPreview), NULL);
  }
  
  gtk_widget_show_all(cp->colorpicker);
}

void PickColor(float *storage)
{
  float *orig = new float[3];
  orig[0] = storage[0];
  orig[1] = storage[1];
  orig[2] = storage[2];
  
  if (!cp)
    ColorPickerCreate();
  else
    gtk_widget_show(cp->colorpicker);

  // clear out the values for the next selection (set in GeomWindowMenu)
  if (cp->post_change) {
    cp->cs_float.clear();
    for (unsigned i = 0; i < cp->cs_orig_vals.size(); i++) {
      delete cp->cs_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per cs_float
    }
    cp->cs_orig_vals.clear();

    cp->post_change = false;
  }

  cp->cs_float.push_back(storage);
  cp->cs_orig_vals.push_back(orig);
  
  cp->selected_color[0] = cp->cs_float[0][0];
  cp->selected_color[1] = cp->cs_float[0][1];
  cp->selected_color[2] = cp->cs_float[0][2];
  
  GdkColor color;
  color.red = (guint16) (cp->cs_float[0][0]*65535);
  color.green = (guint16) (cp->cs_float[0][1]*65535);
  color.blue = (guint16) (cp->cs_float[0][2]*65535);
  
  gtk_widget_modify_bg(cp->selected_color_widget, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(cp->orig_color_widget, GTK_STATE_NORMAL, &color);
  
}

void SizePickerCancel(GtkWidget*)
{
  for (unsigned int i = 0; i < sp->ss_float.size(); i++) {
    *sp->ss_float[i] = sp->ss_orig_vals[i];
  }
  sp->ss_float.clear();
  sp->ss_orig_vals.clear();
  gtk_widget_hide(sp->sizepicker);
  Broadcast(MAP3D_REPAINT_ALL, 0);
}

void SizePickerPreview(GtkWidget*)
{
  
  for (unsigned int i = 0; i < sp->ss_float.size(); i++) {
    *sp->ss_float[i] = (float) gtk_range_get_value(GTK_RANGE(sp->size_slider)) * (sp->factor/10);
  }
  Broadcast(MAP3D_REPAINT_ALL, 0);
}

void SizePickerOkay(GtkWidget* widget)
{
  SizePickerPreview(widget);
  
  sp->ss_float.clear();
  sp->ss_orig_vals.clear();
  gtk_widget_hide(sp->sizepicker);
}

void SizePickerCreate()
{
  if (sp == NULL)
    sp = new SizePicker;
  
  sp->post_change = false;

  // create the window and set window parameters
  sp->sizepicker = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(sp->sizepicker), "Select size");
  gtk_window_set_resizable(GTK_WINDOW(sp->sizepicker), false);
  g_signal_connect(G_OBJECT(sp->sizepicker), "delete_event", G_CALLBACK(gtk_widget_hide),
                   NULL);
  
  // add sub-boxes
  GtkWidget* vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(sp->sizepicker), vbox);
  
  GtkObject* adj = gtk_adjustment_new(5,1,11,1,1,1);
  sp->size_slider = gtk_hscale_new((GtkAdjustment*)adj);
  gtk_box_pack_start(GTK_BOX(vbox), sp->size_slider, FALSE, FALSE, 2);
  
  
  // add last row and selected size box into it.
  GtkWidget* hbox = gtk_hbox_new(TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
  
  GtkWidget *preview, *cancel, *okay;
  preview = gtk_button_new_with_label("Preview");
  cancel = gtk_button_new_with_label("Cancel");
  okay = gtk_button_new_with_label("OK");
  
  gtk_box_pack_start(GTK_BOX(hbox), preview, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), cancel, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), okay, TRUE, TRUE, 0);
  
  g_signal_connect(G_OBJECT(cancel), "clicked", G_CALLBACK(SizePickerCancel), NULL);
  g_signal_connect(G_OBJECT(okay), "clicked", G_CALLBACK(SizePickerOkay), NULL);
  g_signal_connect(G_OBJECT(preview), "clicked", G_CALLBACK(SizePickerPreview), NULL);
  
  gtk_widget_show_all(sp->sizepicker);
}

// the value of f is the maximum size you can get.  You will
// always get x*f/10, where x is [1,10] based on the size you pick
// value returned in storage
void PickSize(float *storage, float f, char* string)
{
  if (!sp)
    SizePickerCreate();
  else
    gtk_widget_show(sp->sizepicker);
  
  gtk_window_set_title(GTK_WINDOW(sp->sizepicker), string);
  
  // clear out the values for the next selection (set in GeomWindowMenu)
  if (sp->post_change) {
    sp->ss_float.clear();
    sp->ss_orig_vals.clear();

    sp->post_change = false;
  }

  sp->ss_float.push_back(storage);
  sp->ss_orig_vals.push_back(*storage);
  
  sp->factor = f;
  
  gtk_range_set_value(GTK_RANGE(sp->size_slider), (int)(*storage * f/10));
  //gtk_widget_set_default_size(sp->selected_size_widget, *storage * f/10, 25);
  
}

// storage is a pointer to the value to change,
//   maxChange is the highest possible range of values
//   inc is what fraction (decimal) to change each time
//   midpoint is the middle possible value (*usually* maxChange / 2)
//   range: ( midpoint-(maxChange/2), midpoint+(maxChange/2) ]
void incrSize(float *storage, float maxChange, float midpoint, float inc)
{
  float s = *storage;
  float increment = maxChange * inc;
  s += increment;
  if (s < midpoint - (maxChange / 2) + fabs(inc) * maxChange) // normally to clamp it
    s = midpoint - (maxChange / 2) + fabs(inc) * maxChange; // at 1 instead of 0
  else if (s > midpoint + (maxChange / 2))
    s = midpoint + (maxChange / 2);
  *storage = s;
}

// ------------------- //
// file picker dialog callbacks, create, and accessor functions

void FilePickerClose(GtkWidget* file_selector)
{
  // if this is not also blocked by the secondary window
  if (filedialog && filedialog->window && filedialog->sensitive)
    gtk_widget_set_sensitive(filedialog->window, true);
  if (fsdialog && fsdialog->window)
    gtk_widget_set_sensitive(fsdialog->window, true);
  if (savedialog && savedialog->window)
    gtk_widget_set_sensitive(savedialog->window, true);
  gtk_widget_hide(file_selector);
  fp->active = false;
}
void store_filename(GtkWidget* /*widget*/, gpointer data)
{
  GtkWidget *file_selector = (GtkWidget *)data;
  const gchar *selected_filename;
  
  selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
  if (fp->entry) {
    gtk_entry_set_text(GTK_ENTRY(fp->entry), selected_filename); 
  }
  else if (fp->filename) {
    strcpy(fp->filename, selected_filename);
  }
  else if (fp->cb) {
    gtk_entry_set_text(GTK_ENTRY(fp->cb->entry), selected_filename); 
    fp->cb->callback();
  }
  FilePickerClose(file_selector);
}

// use this callback for callbacks like the file picker, but without
// picking a file
int rowcallback(gpointer data)
{
  FilePickerCallback* cb = (FilePickerCallback*) data;
  cb->callback();
  
  // or it might complain at runtime about not getting the signal
  return FALSE;
}

void FilePickerCreate()
{
  if (fp)
    return;
  fp = new FilePicker;
  fp->file_selector_geom = gtk_file_selection_new ("Please select a geometry file.");
  fp->file_selector_data = gtk_file_selection_new ("Please select a data file.");
  fp->file_selector_save = gtk_file_selection_new ("Please select a file to save.");
  fp->active = false;
  
  g_signal_connect(G_OBJECT(fp->file_selector_geom), "delete_event", G_CALLBACK(FilePickerClose), NULL);
  g_signal_connect(G_OBJECT(fp->file_selector_save), "delete_event", G_CALLBACK(FilePickerClose), NULL);
  g_signal_connect(G_OBJECT(fp->file_selector_data), "delete_event", G_CALLBACK(FilePickerClose), NULL);
  
  g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fp->file_selector_geom)->ok_button),
                    "clicked",
                    G_CALLBACK (store_filename),
                    (gpointer) fp->file_selector_geom);
  
  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (fp->file_selector_geom)->cancel_button),
                            "clicked",
                            G_CALLBACK (FilePickerClose),
                            (gpointer) fp->file_selector_geom);
  
  g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fp->file_selector_data)->ok_button),
                    "clicked",
                    G_CALLBACK (store_filename),
                    (gpointer) fp->file_selector_data);
  
  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (fp->file_selector_data)->cancel_button),
                            "clicked",
                            G_CALLBACK (FilePickerClose),
                            (gpointer) fp->file_selector_data);
  
  g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fp->file_selector_save)->ok_button),
                    "clicked",
                    G_CALLBACK (store_filename),
                    (gpointer) fp->file_selector_save);
  
  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (fp->file_selector_save)->cancel_button),
                            "clicked",
                            G_CALLBACK (FilePickerClose),
                            (gpointer) fp->file_selector_save);
  
}

void PickFile(char* file)
{
  if (fp == NULL)
    FilePickerCreate();
  fp->filename = file;
  fp->entry = NULL;
  fp->cb = NULL;
  fp->active = true;
  gtk_widget_show(fp->file_selector_geom);
  if (filedialog && filedialog->window) 
    gtk_widget_set_sensitive(filedialog->window, false);
  if (fsdialog && fsdialog->window)
    gtk_widget_set_sensitive(fsdialog->window, false);
  if (savedialog && savedialog->window)
    gtk_widget_set_sensitive(savedialog->window, false);
}

void PickFileWidget(GtkWidget* entry)
{
  if (fp == NULL)
    FilePickerCreate();
  fp->filename = NULL;
  fp->entry = entry;
  fp->cb = NULL;
  fp->active = true;
  
  map<GtkWidget*, GtkWidget*>::iterator iter = fp->which_selector.find(entry);
  if (iter == fp->which_selector.end())
    gtk_widget_show(fp->file_selector_geom);
  else
    gtk_widget_show(iter->second);
  if (filedialog && filedialog->window) 
    gtk_widget_set_sensitive(filedialog->window, false);
  if (fsdialog && fsdialog->window)
    gtk_widget_set_sensitive(fsdialog->window, false);
  if (savedialog && savedialog->window)
    gtk_widget_set_sensitive(savedialog->window, false);
}

void PickFileCb(FilePickerCallback* cb)
{
  
  if (fp == NULL)
    FilePickerCreate();
  fp->filename = NULL;
  fp->entry = NULL;
  fp->cb = cb;
  fp->active = true;
  map<GtkWidget*, GtkWidget*>::iterator iter = fp->which_selector.find(cb->entry);
  if (iter == fp->which_selector.end())
    gtk_widget_show(fp->file_selector_geom);
  else
    gtk_widget_show(iter->second);
  
  if (filedialog && filedialog->window) 
    gtk_widget_set_sensitive(filedialog->window, false);
  if (fsdialog && fsdialog->window)
    gtk_widget_set_sensitive(fsdialog->window, false);
  if (savedialog && savedialog->window)
    gtk_widget_set_sensitive(savedialog->window, false);
}

// ------------------- //
//frame dialog callbacks, create, and accessor functions
void frameDialogCreate()
{
  if (framedialog != NULL){
    return;
  }
  framedialog = new FrameDialog;
  framedialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(framedialog->window), "Frame Step");
  //  gtk_container_set_border_width (GTK_CONTAINER (map3d_info.window), 5);
  gtk_window_set_resizable(GTK_WINDOW(framedialog->window), FALSE);
  
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_container_add(GTK_CONTAINER(framedialog->window), vbox);
  gtk_widget_show(vbox);
  
  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, TRUE, 0);
  //  gtk_container_add(GTK_CONTAINER(framedialog->window), notebook);
  gtk_widget_show(notebook);
  
  //////////////////////////////////////////////////////////////
  // Create the horizontal box on the bottom of the dialog
  //     with the "Reset" and "Close" buttons.
  GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show(hbox);
  
  GtkWidget *close = gtk_button_new_with_label("Close");
  gtk_box_pack_end(GTK_BOX(hbox), close, FALSE, FALSE, 5);
  g_signal_connect_swapped(G_OBJECT(close), "clicked", G_CALLBACK(framedestroycallback), framedialog->window);
  gtk_widget_show(close);
  
  GtkWidget *reset_btn = gtk_button_new_with_label("Reset");
  gtk_box_pack_end(GTK_BOX(hbox), reset_btn, true, true, 5);
  //g_signal_connect_swapped(G_OBJECT(reset_btn), "clicked", G_CALLBACK(gtk_widget_hide), framedialog->window);
  //gtk_widget_show(reset_btn);
  //// Adding a tool tip:
  GtkTooltips *button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(button_tips, GTK_WIDGET(reset_btn),
                       "Resets the tab shown to the state it was in when "
                       "the tab was first opened.  WARNING: THIS HAS NOT BEEN IMPLEMENTED YET!", "");
  ////
  //////////////////////////////////////////////////////////////
  
  GtkWidget *vstep = gtk_hbox_new(FALSE, 20);
  gtk_container_set_border_width(GTK_CONTAINER(vstep), 5);
  
  GtkWidget *stepLabel = gtk_label_new("          Frame Step          ");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vstep, stepLabel);
  
  gtk_widget_show(vstep);
    
  framedialog->fs_adj = gtk_adjustment_new(fstep,1,100,1,0.5,1);
  framedialog->framestep = gtk_spin_button_new((GtkAdjustment*)framedialog->fs_adj,1,0);
  gtk_box_pack_start(GTK_BOX(vstep), framedialog->framestep, TRUE, TRUE, 50);
  gtk_widget_show(framedialog->framestep);
  
  g_signal_connect_swapped(G_OBJECT(framedialog->framestep), "value-changed", G_CALLBACK(framecallback), NULL);
  
  g_signal_connect(G_OBJECT(framedialog->window), "delete_event", G_CALLBACK(framedestroycallback), NULL);
}


void framecallback()
{
  fstep = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(framedialog->framestep));
  map3d_info.user_fstep = fstep;
  //ComputeLockFrameData();
  for (unsigned i = 0; i < numGeomWindows(); i++)
    GetGeomWindow(i)->frame_num.setActive(0);
}

void framedestroycallback()
{
  //printf("Calling frame destroy stuff\n");
  gtk_object_destroy (framedialog->fs_adj);
  gtk_widget_destroy (framedialog->framestep);
  gtk_widget_destroy (framedialog->window);
  framedialog->gwindows.clear();
  framedialog = NULL;
}

// ------------------- //
//fog dialog callbacks, create, and accessor functions
void fogDialogCreate(GeomWindow* priv)
{
  if (fogdialog != NULL){
    fogdialog->gwindows.push_back(priv);
    return;
  }
  fogdialog = new FogDialog;
  fogdialog->gwindows.push_back(priv);
  
  fogdialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  gtk_window_set_title(GTK_WINDOW(fogdialog->window), "Depth Cueing");
  //  gtk_container_set_border_width (GTK_CONTAINER (map3d_info.window), 5);
  gtk_window_set_resizable(GTK_WINDOW(fogdialog->window), FALSE);
  
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_container_add(GTK_CONTAINER(fogdialog->window), vbox);
  gtk_widget_show(vbox);
  
  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, TRUE, 0);
  //  gtk_container_add(GTK_CONTAINER(fogdialog->window), notebook);
  gtk_widget_show(notebook);
  
  //////////////////////////////////////////////////////////////
  // Create the horizontal box on the bottom of the dialog
  //     with the "Reset" and "Close" buttons.
  GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show(hbox);
  
  GtkWidget *close = gtk_button_new_with_label("Close");
  gtk_box_pack_end(GTK_BOX(hbox), close, FALSE, FALSE, 5);
  g_signal_connect(G_OBJECT(close), "clicked", G_CALLBACK(fogdestroycallback), fogdialog->window);
  gtk_widget_show(close);
  
  GtkWidget *reset_btn = gtk_button_new_with_label("Reset");
  gtk_box_pack_end(GTK_BOX(hbox), reset_btn, true, true, 5);
  //g_signal_connect_swapped(G_OBJECT(reset_btn), "clicked", G_CALLBACK(gtk_widget_hide), fogdialog->window);
  //gtk_widget_show(reset_btn);
  //// Adding a tool tip:
  GtkTooltips *button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(button_tips, GTK_WIDGET(reset_btn),
                       "Resets the tab shown to the state it was in when "
                       "the tab was first opened.  WARNING: THIS HAS NOT BEEN IMPLEMENTED YET!", "");
  ////
  //////////////////////////////////////////////////////////////
  
  GtkWidget *vcont = gtk_vbox_new(FALSE, 20);
  gtk_container_set_border_width(GTK_CONTAINER(vcont), 5);
  
  GtkWidget *spacingLabel = gtk_label_new("          Depth Cueing          ");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vcont, spacingLabel);
  
  gtk_widget_show(vcont);
  
  GtkWidget *fogLabel1 = gtk_label_new("Front Plane:");
  GtkWidget *fogLabel2 = gtk_label_new("Rear Plane:");
  
  fogdialog->fs_adj1 = gtk_adjustment_new(priv->fog1,0,4,0.1,0.5,1);
  fogdialog->fogspacing1 = gtk_hscale_new((GtkAdjustment*)fogdialog->fs_adj1);
  fogdialog->fs_adj2 = gtk_adjustment_new(priv->fog2,0,4,0.1,0.5,1);
  fogdialog->fogspacing2 = gtk_hscale_new((GtkAdjustment*)fogdialog->fs_adj2);
  gtk_box_set_spacing (GTK_BOX(vcont),0);
  gtk_box_pack_start_defaults(GTK_BOX(vcont), fogLabel1);
  gtk_box_pack_start_defaults(GTK_BOX(vcont), fogdialog->fogspacing1);
  gtk_box_pack_start_defaults(GTK_BOX(vcont), fogLabel2);
  gtk_box_pack_start_defaults(GTK_BOX(vcont), fogdialog->fogspacing2);
  gtk_widget_show(fogLabel1);
  gtk_widget_show(fogdialog->fogspacing1);
  gtk_widget_show(fogLabel2);
  gtk_widget_show(fogdialog->fogspacing2);
  gtk_widget_show(fogdialog->window);
  
  g_signal_connect_swapped(G_OBJECT(fogdialog->fogspacing1), "value-changed", G_CALLBACK(fogcallback1), NULL);
  g_signal_connect_swapped(G_OBJECT(fogdialog->fogspacing2), "value-changed", G_CALLBACK(fogcallback2), NULL);
  
  g_signal_connect(G_OBJECT(fogdialog->window), "delete_event", G_CALLBACK(fogdestroycallback), NULL);
  
}


void fogcallback1()
{
  
  for(unsigned i = 0; i < fogdialog->gwindows.size(); i++){
    fogdialog->gwindows[i]->fog1 = (float) gtk_range_get_value(GTK_RANGE(fogdialog->fogspacing1));
  }
  
  
  Broadcast(MAP3D_REPAINT_ALL, 0);
  
}

void fogcallback2()
{
  
  for(unsigned i = 0; i < fogdialog->gwindows.size(); i++){
    fogdialog->gwindows[i]->fog2 = (float)gtk_range_get_value(GTK_RANGE(fogdialog->fogspacing2));
  }
  
  
  Broadcast(MAP3D_REPAINT_ALL, 0);
  
}

void fogdestroycallback()
{
  
  //printf("Calling fog destroy stuff\n");
  gtk_object_destroy (fogdialog->fs_adj1);
  gtk_widget_destroy (fogdialog->fogspacing1);
  gtk_object_destroy (fogdialog->fs_adj2);
  gtk_widget_destroy (fogdialog->fogspacing2);
  //printf("fogdialog->gwindows.size() = %d\n",fogdialog->gwindows.size());
  fogdialog->gwindows.clear();
  //printf("fogdialog->gwindows.size() = %d\n",fogdialog->gwindows.size());
  gtk_widget_destroy (fogdialog->window);
  fogdialog = NULL;
  //printf("Finished Calling fog destroy stuff\n");
  
}





