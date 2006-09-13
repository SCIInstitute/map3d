#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include "dialogs.h"
// --------------------------- //
// SaveDialog widget and accessor functions 
// we create a new geom table every time, so we need to keep track
// of whether it exists so we don't create new ones when 
// some exist.  The rest of the stuff is only created once.
struct SaveDialog {
  GtkWidget* window;

  // Geom table and save prefs
  GtkWidget* table;
  GtkWidget* save_separate;
  GtkWidget* save_together;


  // image stuff
  GtkWidget* image_filename;
  GtkWidget* image_default_res;
  GtkWidget* image_res_height;
  GtkWidget* image_res_label;
  GtkWidget* image_res_width;

  //animation stuff
  GtkWidget* animation_transformation;
  GtkWidget* animation_frame_advance;
  GtkWidget* animation_other_events;
  GtkWidget* animation_idle;
  GtkWidget* animation_idle_ms;
  GtkWidget* animation_idle_ms_label;
  GtkWidget* animation_start_button;
  GtkWidget* animation_stop_button;

  GTimer* timer;
  int animation_idle_loop;
  double animation_last_save_time;

  // save settings stuff.
  GtkWidget* settings_map3drc;
  GtkWidget* settings_batch;
  GtkWidget* settings_script;
  GtkWidget* settings_filename;
  GtkWidget* settings_change;

  GtkWidget* settings_global;
  GtkWidget* settings_transform;
  GtkWidget* settings_meshoptions;

  GtkWidget* close_button;
  bool destroyed; 
};

void saveDialogCreate(bool show = true);
void AnimationIdleLoop(SaveDialog* sd);
void destroySaveDialog(SaveDialog* sd);
void saveDialogGeomsTogether(SaveDialog* sd);
void saveDialogGeomsSeparate(SaveDialog* sd);
void saveDialogSettingsRadio(SaveDialog* sd);
void saveDialogImageResolution(SaveDialog* sd);
void saveDialogStartAnimations(SaveDialog* sd);
void saveDialogStopAnimations(SaveDialog* sd);
void saveDialogAnimationsIdle(SaveDialog* sd);

#endif
