/* SizePicker.cxx */

#ifdef _WIN32
#include <afxwin.h>
#pragma warning(disable:4505)
#undef TRACE
#endif

#include <gtk/gtk.h>
#include "SizePicker.h"
#include "WindowManager.h"
#include "eventdata.h"

#include <math.h>
#include <vector>

using std::vector;

#define W 5
#define H 2
static int spwinid = 0;
static int width = 128, height = 64;

SizePicker *sp = NULL;

gint SizePickerSetSize(GtkWidget*, GdkEvent*, gpointer data) {
  sp->selected_size = reinterpret_cast<int>(data);
  gtk_widget_set_size_request(sp->selected_size_widget, 8 * sp->selected_size, 25);
  return 0;
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
    *sp->ss_float[i] = sp->selected_size * (sp->factor/10);
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

  // create the window and set window parameters
  sp->sizepicker = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(sp->sizepicker), "Select size");
  gtk_window_set_resizable(GTK_WINDOW(sp->sizepicker), false);
  g_signal_connect(G_OBJECT(sp->sizepicker), "delete_event", G_CALLBACK(gtk_widget_hide),
    NULL);

  // add sub-boxes
  GtkWidget* vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(sp->sizepicker), vbox);
  
  GtkWidget* hbox_size_cols = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox_size_cols, FALSE, FALSE, 2);

  // add last row and selected size box into it.
  GtkWidget* hbox = gtk_hbox_new(TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

  sp->orig_size_widget = gtk_event_box_new();
  sp->selected_size_widget = gtk_event_box_new();

  gtk_widget_set_size_request(sp->selected_size_widget, 100, 25);
  gtk_widget_set_size_request(sp->orig_size_widget, 100, 25);

  gtk_box_pack_start(GTK_BOX(hbox), sp->orig_size_widget, TRUE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), sp->selected_size_widget, TRUE, FALSE, 2);

  GdkColor color;
  color.red = color.blue = color.green = 0.0;
  
  gtk_widget_modify_bg(sp->orig_size_widget, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(sp->selected_size_widget, GTK_STATE_NORMAL, &color);

  // add sizes
  for (int i = 0; i < 10; i++) {
    GtkWidget* frame = gtk_event_box_new();
    gtk_widget_set_size_request(frame, 8*(i+1), 25);
    gtk_widget_modify_bg(frame, GTK_STATE_NORMAL, &color);
    // add sizes
    g_signal_connect(G_OBJECT(frame), "button_press_event", G_CALLBACK(SizePickerSetSize), (void*)(i+1));
  
    gtk_box_pack_start(GTK_BOX(hbox_size_cols), frame, FALSE, FALSE, 2);
  }

  
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
void PickSize(float *storage, float f)
{
  if (!sp)
    SizePickerCreate();
  else
    gtk_widget_show(sp->sizepicker);

  sp->ss_float.push_back(storage);
  sp->ss_orig_vals.push_back(*storage);

  sp->factor = f;

  gtk_widget_set_size_request(sp->orig_size_widget, 8 * *storage * f/10, 25);
  //gtk_widget_set_default_size(sp->orig_size_widget, *storage * f/10, 25);

  gtk_widget_set_size_request(sp->selected_size_widget, 8 * *storage * f/10, 25);
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
