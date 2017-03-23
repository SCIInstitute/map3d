/* ColorPicker.cxx */

#ifdef _WIN32
#  include <afxwin.h>
#  pragma warning(disable:4505)
#  undef TRACE
#endif

#include <gtk/gtk.h>
#include "ColorPicker.h"
#include "WindowManager.h"
#include "eventdata.h"

#include <math.h>

ColorPicker *cp = NULL;

static int cpwinid = 0;
static int width = 165, height = 57;
static float cellwidth = 2.f / 9;
static float cellheight = 2.f / 3;

gint ColorPickerSetColor(GtkWidget*, GdkEvent*, gpointer data) {
  int colorindex = reinterpret_cast<int>(data);

  float r = 1;
  float g = 1;
  float b = 1;

  if (colorindex < 27) {
    r = colorindex * .5;
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
  color.red = r * 65535.;
  color.green = g * 65535.;
  color.blue = b * 65535.;
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
  if (cp == NULL)
    cp = new ColorPicker;

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

  cp->orig_color_widget = gtk_event_box_new();
  cp->selected_color_widget = gtk_event_box_new();

  gtk_widget_set_size_request(cp->selected_color_widget, 100, 25);
  gtk_widget_set_size_request(cp->orig_color_widget, 100, 25);

  gtk_box_pack_start(GTK_BOX(hbox), cp->orig_color_widget, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), cp->selected_color_widget, FALSE, FALSE, 2);

  int numcolors = 28;
  // add colors
  for (int i = 0; i < numcolors; i++) {
    GtkWidget* frame = gtk_event_box_new();
    gtk_widget_set_size_request(frame, 22, 22);

    // interpolate through all combinations of 0,.5,.1 for color
    float r = 1;
    float g = 1;
    float b = 1;

    if (i < 27) {
      r = i * .5;
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

    color.red = r * 65535.;
    color.green = g * 65535.;
    color.blue = b * 65535.;

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

  cp->cs_float.push_back(storage);
  cp->cs_orig_vals.push_back(orig);

  cp->selected_color[0] = cp->cs_float[0][0];
  cp->selected_color[1] = cp->cs_float[0][1];
  cp->selected_color[2] = cp->cs_float[0][2];

  GdkColor color;
  color.red = cp->cs_float[0][0]*65535.;
  color.green = cp->cs_float[0][1]*65535.;
  color.blue = cp->cs_float[0][2]*65535.;

  gtk_widget_modify_bg(cp->selected_color_widget, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(cp->orig_color_widget, GTK_STATE_NORMAL, &color);

}