#include "ScaleDialog.h"
#include "map3d-struct.h"
#include <gtk/gtk.h>

ScaleDialog *scaledialog = NULL;
extern Map3d_Info map3d_info;
// ------------------- //
// scale picker dialog callbacks, create, and accessor functions

void scaleDialogCreate()
{
  if (scaledialog != NULL)
    return;
  scaledialog = new ScaleDialog;
  scaledialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(scaledialog->window), "Scaling");
  //  gtk_container_set_border_width (GTK_CONTAINER (map3d_info.window), 5);
  gtk_window_set_resizable(GTK_WINDOW(scaledialog->window), FALSE);
  
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_container_add(GTK_CONTAINER(scaledialog->window), vbox);
  gtk_widget_show(vbox);
  
  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, TRUE, 0);
  //  gtk_container_add(GTK_CONTAINER(scaledialog->window), notebook);
  gtk_widget_show(notebook);
  
  //////////////////////////////////////////////////////////////
  // Create the horizontal box on the bottom of the dialog
  //     with the "Reset" and "Close" buttons.
  GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show(hbox);
  
  GtkWidget *close = gtk_button_new_with_label("Close");
  gtk_box_pack_end(GTK_BOX(hbox), close, FALSE, FALSE, 5);
  g_signal_connect_swapped(G_OBJECT(close), "clicked", G_CALLBACK(gtk_widget_hide), scaledialog->window);
  gtk_widget_show(close);
  
  GtkWidget *reset_btn = gtk_button_new_with_label("Reset");
  gtk_box_pack_end(GTK_BOX(hbox), reset_btn, true, true, 5);
  //g_signal_connect_swapped(G_OBJECT(reset_btn), "clicked", G_CALLBACK(gtk_widget_hide), scaledialog->window);
  //gtk_widget_show(reset_btn);
  //// Adding a tool tip:
  GtkTooltips *button_tips = gtk_tooltips_new();
  gtk_tooltips_set_tip(button_tips, GTK_WIDGET(reset_btn),
                       "Resets the tab shown to the state it was in when "
                       "the tab was first opened.  WARNING: THIS HAS NOT BEEN IMPLEMENTED YET!", "");
  ////
  //////////////////////////////////////////////////////////////
  
  GtkWidget *vrange = gtk_vbox_new(FALSE, 2);
  gtk_container_set_border_width(GTK_CONTAINER(vrange), 5);
  
  GtkWidget *vfunction = gtk_vbox_new(FALSE, 2);
  gtk_container_set_border_width(GTK_CONTAINER(vfunction), 5);
  
  GtkWidget *vmapping = gtk_vbox_new(FALSE, 2);
  gtk_container_set_border_width(GTK_CONTAINER(vmapping), 5);
  
  
  GtkWidget *rangeLabel = gtk_label_new(" Range ");
  GtkWidget *functionLabel = gtk_label_new(" Function ");
  GtkWidget *mappingLabel = gtk_label_new(" Mapping ");
  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vrange, rangeLabel);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vfunction, functionLabel);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vmapping, mappingLabel);
  
  
  gtk_widget_show(vrange);
  gtk_widget_show(vfunction);
  gtk_widget_show(vmapping);
  
  
  // Range radio group
  
  scaledialog->range[LOCAL_SCALE] = gtk_radio_button_new_with_label((NULL),
                                                                    "Local");
  scaledialog->range[GLOBAL_SURFACE] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])),
                                                                       "Global over all frames in one surface");
  scaledialog->range[GLOBAL_FRAME] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])),
                                                                     "Global over all surfaces in one frame");
  scaledialog->range[GLOBAL_GLOBAL] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])),
                                                                      "Global over all surfaces and frames");
  scaledialog->range[GROUP_FRAME] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])),
                                                                    "Scaling over groups in a frame");
  scaledialog->range[GROUP_GLOBAL] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])),
                                                                     "Scaling over groups in all frames");
  scaledialog->range[SLAVE_FRAME] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])),
                                                                    "Slave scaling over a frame");
  scaledialog->range[SLAVE_GLOBAL] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->range[LOCAL_SCALE])),
                                                                     "Slave scaling over all frames");
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scaledialog->range[map3d_info.scale_scope]), TRUE);
  
  for (int i = 0; i < NUM_RANGES; i++) {
    g_signal_connect_swapped(G_OBJECT(scaledialog->range[i]), "clicked", G_CALLBACK(setScalingRange),
                             gpointer(i));
    gtk_box_pack_start(GTK_BOX(vrange), scaledialog->range[i], FALSE, TRUE, 0);
    gtk_widget_show(scaledialog->range[i]);
  }
  
  // Function radio group
  
  scaledialog->func[LINEAR] = gtk_radio_button_new_with_label((NULL),
                                                              "Linear");
  scaledialog->func[LOG] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->func[LINEAR])),
                                                           "Logarithmic");
  scaledialog->func[EXP] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->func[LINEAR])),
                                                           "Exponential");
  scaledialog->func[LAB7] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->func[LINEAR])),
                                                            "lab standard");
  scaledialog->func[LAB13] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->func[LINEAR])),
                                                             "lab-13 standard");
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scaledialog->func[map3d_info.scale_model]), TRUE);
  
  for (int i = 0; i < NUM_FUNC; i++) {
    g_signal_connect_swapped(G_OBJECT(scaledialog->func[i]), "clicked", G_CALLBACK(setScalingFunction),
                             gpointer(i));
    gtk_box_pack_start(GTK_BOX(vfunction), scaledialog->func[i], FALSE, TRUE, 0);
    gtk_widget_show(scaledialog->func[i]);
  }
  
  // Mapping radio group
  
  scaledialog->map[TRUE_MAP] = gtk_radio_button_new_with_label((NULL),
                                                               "True");
  scaledialog->map[MID_MAP] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->map[TRUE_MAP])),
                                                               "Symmetric about midpoint");
  scaledialog->map[SYMMETRIC] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->map[TRUE_MAP])),
                                                                "Symmetric about zero");
  scaledialog->map[SEPARATE] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(scaledialog->map[TRUE_MAP])),
                                                               "Separate about zero");
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scaledialog->map[map3d_info.scale_mapping]), TRUE);
  
  for (int i = 0; i < NUM_MAPS; i++) {
    g_signal_connect_swapped(G_OBJECT(scaledialog->map[i]), "clicked", G_CALLBACK(setScalingMapping),
                             gpointer(i));
    gtk_box_pack_start(GTK_BOX(vmapping), scaledialog->map[i], FALSE, TRUE, 0);
    gtk_widget_show(scaledialog->map[i]);
  }
  
  
  //gtk_widget_show(priv->scale->vfunction);
  //gtk_widget_show(priv->scale->vmapping);
  //gtk_widget_show(priv->scale->vgrouping);
  
  g_signal_connect(G_OBJECT(scaledialog->window), "delete_event", G_CALLBACK(gtk_widget_hide), NULL);
}
