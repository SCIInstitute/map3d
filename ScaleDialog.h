#ifndef SCALEDIALOG_H
#define SCALEDIALOG_H

#include "dialogs.h"
// --------------------------- //
// ScaleDialog widget and accessor/helper functions //
struct ScaleDialog {
  GtkWidget* window;
  GtkWidget* range[NUM_RANGES];
  GtkWidget* func[NUM_FUNC];
  GtkWidget* map[NUM_MAPS];

};
void scaleDialogCreate();

#endif
