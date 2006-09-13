/* dialogs.h */

#ifndef DIALOGS_H
#define DIALOGS_H

#include "map3d-struct.h"
#include "scalesubs.h"
#include "MeshList.h"
#include "GeomWindow.h"
#include <vector>
#include <string>
#include <map>

using std::string;
using std::vector;
using std::map;

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkAdjustment GtkAdjustment;
typedef struct _GtkObject GtkObject;
typedef struct _GTimer GTimer;
typedef struct _GtkListStore GtkListStore;

#ifdef _WIN32
#pragma warning(disable:4505)  /* quiet visual c++ */
#endif


class GeomWindow;
class PickWindow;
class Mesh_Info;

// exit dialog function
void map3d_quit(gpointer parentWindow);
// misc functions
char* shorten_filename(char* x);
string get_path(char* x);
double roundedNum(double num);

// --------------------------- //
// SizePicker widget and accessor functions

struct SizePicker {
  GtkWidget *sizepicker;
  GtkWidget *size_slider;
  float factor;
  float selected_size;
  vector < float* > ss_float;
  vector < float > ss_orig_vals;

  // this is so we can clear the storage on a subsequent call to PickColor
  // right now, kind of hacky, it is set at the end of GeomWindowMenu
  bool post_change;
};

void PickSize(float *storage, float factor, char* str);
void incrSize(float *storage, float maxChange, float midpoint, float inc);

// --------------------------- //
// ColorPicker widget and accessor functions

struct ColorPicker {
  GtkWidget *colorpicker;
  GtkWidget *orig_color_widget;
  GtkWidget *selected_color_widget;
  float selected_color[3];
  vector < float *> cs_float;
  vector < float *> cs_orig_vals;

  // this is so we can clear the storage on a subsequent call to PickColor
  // right now, kind of hacky, it is set at the end of GeomWindowMenu
  bool post_change;
};

void PickColor(float *);

// --------------------------- //
// FilePicker widget and accessor functions

class FilePickerCallback {
public:
  FilePickerCallback(GtkWidget* entry) : entry(entry) {}
  virtual void callback() = 0;
  GtkWidget* entry;
};

// these are so we can create multiple file pickers to remember the last-used
// directory for each of these. 
#define FILES_DIR_GEOM 0
#define FILES_DIR_DATA 1
#define FILES_DIR_SAVE 2


struct FilePicker {
  //It's easier to make different file selectors than to have to reassign the directory every time
  GtkWidget *file_selector_geom;
  GtkWidget *file_selector_data;
  GtkWidget *file_selector_save;
  char* filename;
  bool active;  // if this is active, then the savedialog, filedialog or filesecondarydialog should not run

  // a gtk entry to set the text.  Only one of this or to_save shoule be set
  GtkWidget* entry;

  // a callback - used to make a callback once a file is selected
  FilePickerCallback *cb;

  // map of text entries to file selectors
  map<GtkWidget*, GtkWidget*> which_selector;
  inline void assignEntry(GtkWidget* entry, GtkWidget* selector) 
  { which_selector[entry] = selector; }

};

void FilePickerCreate();
// to pick a file from the string file, and store result there
void PickFile(char* file);
// to pick a file from the string in entry, and store result there
void PickFileWidget(GtkWidget* entry);
void PickFileCb(FilePickerCallback *cb);
int rowcallback(gpointer data);

// --------------------------- //
// FrameDialog widget and accessor/helper functions //
struct FrameDialog {
  GtkWidget* window;
  GeomWindow* priv;
  GtkWidget* framestep;
  GtkObject* fs_adj;
  vector<GeomWindow*> gwindows;
};
void frameDialogCreate();
void framecallback();
void framedestroycallback();

// --------------------------- //
// FogDialog widget and accessor/helper functions //
struct FogDialog {
  GtkWidget* window;
  GtkWidget* fogspacing1;
  GtkObject* fs_adj1;
  GtkWidget* fogspacing2;
  GtkObject* fs_adj2;
  vector<GeomWindow*> gwindows;
};
void fogDialogCreate(GeomWindow* priv);
void fogcallback1();
void fogcallback2();
void fogdestroycallback();

// allows for the editing of the filename in the files dialog.  Since that is 
// the short filename (without dirs), we must combine with the long name
void replaceFilenameWithShort(const char* longname, const char* shortname, char* combinedname);

int getGtkComboIndex(GtkWidget* combo);
int getGtkComboIndex(GtkWidget* combo,char * filename);

//include these so we only have to include dialogs.h
#include "ContourDialog.h"
#include "FidDialog.h"
#include "FileDialog.h"
#include "SaveDialog.h"
#include "ScaleDialog.h"


#endif
