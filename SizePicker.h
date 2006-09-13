/* SizePicker.h */

#ifndef SIZEPICKER_H
#define SIZEPICKER_H

#include <vector>
using std::vector;

typedef struct _GtkWidget GtkWidget;

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
