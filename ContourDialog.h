#ifndef CONTOURDIALOG_H
#define CONTOURDIALOG_H

#include "dialogs.h"
// --------------------------- //
// ContourDialog widget and accessor/helper functions //
struct ContourDialog {
  GtkWidget* window;

  GtkWidget* table;
  GtkWidget* lock_numconts;
  GtkWidget* lock_contspacing;
  GtkWidget* adjust_range;
  bool orig_spacing_check;
  bool field_lock; // when you change one field and the code sets another
                   // it wants to call the callback for that field as well
                   // hopefully this will prevent that.
};

struct FilesDialogRowData;

void contourDialogCreate(bool show = true);
void ContourCancel();
void ContourPreview(bool okay);
void ContourOkay();
void contourcallback(FilesDialogRowData* rowdata);
void contourDialogChangeLock();
void contourdestroycallback();

// changes to contour dialog rows, when the world changes
// (on frame advance or scaling change)
void updateContourDialogValues(Mesh_Info* mesh);

// changes to contour dialog row from within the same row
// (update the spacing when the num contours changes, etc.)
void modifyContourDialogRow_NumContChange(FilesDialogRowData* rowdata);
void modifyContourDialogRow_RangeChange(FilesDialogRowData* rowdata);
void modifyContourDialogRow_ContSpaceChange(FilesDialogRowData* rowdata);
void modifyContourDialogRow_DefaultRange(FilesDialogRowData* rowdata);


#endif
