/* ColorPicker.h */

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <vector>
using std::vector;

typedef struct _GtkWidget GtkWidget;

struct ColorPicker {
  GtkWidget *colorpicker;
  GtkWidget *orig_color_widget;
  GtkWidget *selected_color_widget;
  float selected_color[3];
  vector < float *> cs_float;
  vector < float *> cs_orig_vals;
};

void PickColor(float *);
void PickColor(unsigned char *);

void ShowColorPicker();

struct SizePicker {
  GtkWidget *sizepicker;
  GtkWidget *orig_size_widget;
  GtkWidget *selected_size_widget;
  float factor;
  float selected_size;
  vector < float* > ss_float;
  vector < float > ss_orig_vals;
};

void PickSize(float *storage, float factor);
void incrSize(float *storage, float maxChange, float midpoint, float inc);
void ShowSizePicker();



#endif
