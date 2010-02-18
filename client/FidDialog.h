#ifndef FIDDIALOG_H
#define FIDDIALOG_H

#include "dialogs.h"

#if 0
struct FilesDialogRowData;


// --------------------------- //
// FidDialog widget and accessor/helper functions //
struct FidDialog {
  GtkWidget* window;
  Mesh_Info* mesh;
  
  GtkWidget* table;
  
  bool field_lock; // when you change one field and the code sets another
                   // it wants to call the callback for that field as well
                   // hopefully this will prevent that.
};



// --------------------------- //
// FidMapDialog widget and accessor/helper functions //
struct FidMapDialog {
  GtkWidget* window;
  Mesh_Info* mesh;
  GtkWidget* table;  
  bool orig_spacing_check;
  bool field_lock; // when you change one field and the code sets another
                   // it wants to call the callback for that field as well
                   // hopefully this will prevent that.
  
    
  GtkWidget* lock_numconts;
  GtkWidget* lock_contspacing;  
};

// --------------------------- //
// ColorPicker widget and accessor functions

struct FidColorPicker {
  GtkWidget *fidcolorpicker;
  GtkWidget *orig_color_widget;
  GtkWidget *selected_color_widget;
  GdkColor selected_color;
  GdkColor cs_orig_vals;
  int index;
  GtkTreeIter iter;
  FilesDialogRowData* rowdata;
};

void FidPickColor(GdkColor *, int index, GtkTreeIter iter, FilesDialogRowData* rowdata);
void fidDialogCreate(bool show);
void fidMapDialogCreate(bool show);
void fidcontourcallback();
void fidcontourcallback_exp();


void fidNameChange(FilesDialogRowData* rowdata);
void fidContSizeChange(FilesDialogRowData* rowdata);
void fidSelectColor(FilesDialogRowData* rowdata);
void fidColorChange(GdkColor fidcolor, int index, GtkTreeIter iter, FilesDialogRowData* rowdata);

void fidmapNameChange(GtkWidget *widget, FilesDialogRowData* rowdata);
void fidmapDialogChangeLock();
void modifyfidmapDialogRow_NumContChange(FilesDialogRowData* rowdata);
void modifyfidmapDialogRow_RangeChange(FilesDialogRowData* rowdata);
void modifyfidmapDialogRow_ContSpaceChange(FilesDialogRowData* rowdata);
void fidmapcontourcallback(FilesDialogRowData* rowdata);
void fidmapcontourcallback_exp(FilesDialogRowData* rowdata);
void fidmapnumcontcallback(FilesDialogRowData* rowdata);
void fidmaprangecontcallback(FilesDialogRowData* rowdata);
void fidmapdestroycallback();
void FidMapCancel();
void FidMapPreview(bool okay);
void FidMapOkay();


#endif
#endif