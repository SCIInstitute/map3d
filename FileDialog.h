#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "dialogs.h"
// --------------------------- //
// FileDialog widget and accessor functions 

// row data - since it is MOSTLY used for the files dialog, it lives here,
// but it also stores data for anyone who needs dialog stuff on a per-surface basis
struct FilesDialogRowData {
  FilesDialogRowData(Mesh_Info* mesh, int window, GtkWidget* dst, GtkWidget* dft, GtkWidget* dnt) :
    mesh(mesh), window(window), dataseriestitle(dst), datafiletitle(dft), datanametitle(dnt) {}
  Mesh_Info* mesh;
  int window;

  // headers in the files dialog we need 
  GtkWidget* dataseriestitle;
  GtkWidget* datafiletitle;
  GtkWidget* datanametitle;

  // entries in the files window table
  // set these manually (not in ctor)
  GtkWidget* surfnum;
  GtkWidget* winnum;
  GtkWidget* geomname;
  GtkWidget* geomshortname;
  
  GtkWidget* geomseries;
  GtkWidget* dataname;
  GtkWidget* datashortname;
  GtkWidget* datacontainer;
  GtkWidget* datacontainershort;

  GtkWidget* dataseries;
  GtkWidget* datafile;
  GtkWidget* datastart;
  GtkWidget* dataend;
  GtkAdjustment* ds_adj;
  GtkAdjustment* de_adj;
  
  PickWindow* rms_curve;
  
  GtkWidget* others;
  // these four are not shown, but are copied to from the Other Options Dialog
  GtkWidget* fidfilename;
  GtkWidget* channelsfilename;
  GtkWidget* leadlinksfilename;
  GtkWidget* landmarksfilename;
  
  bool reload_data;
  bool reload_geom;

  bool removed; // if a bad geom was loaded, then we want to mark it as "removed", and add it back when it is good.

  // the rest of these are used in other dialogs that also use tables with one row per surface.
  // When we add a row to the files dialog, also add a row to the table in the other dialogs

  // For Save Dialog
  GtkWidget* save_surfnum;
  GtkWidget* save_input_filename;
  GtkWidget* save_filename;
  GtkWidget* save_this_surface;
  GtkWidget* save_with_transforms;

  // For Contour Dialog
  GtkWidget* cont_surfnum;
  GtkWidget* cont_surfname;
  GtkWidget* contourspacing;
  GtkObject* cs_adj;
  GtkWidget* numcontours;
  GtkObject* cn_adj;
  GtkWidget* cont_low_range;
  GtkObject* lr_adj;
  GtkWidget* cont_high_range;
  GtkObject* hr_adj;
  GtkWidget* cont_default_range;
  GtkWidget* cont_occlusion_gradient;
  GtkObject* cog_adj;
  
  float orig_numspaces;
  float orig_exp;
  float orig_numconts;
  float orig_lowrange;
  float orig_highrange;
  bool orig_defaultrange;
  
//#if 0//need gtk2.4 for this
  // For Fiducial Dialog
  GtkWidget* fid_surfnum;
  GtkWidget* fid_surfname;
  GtkWidget* fid_name;
  GtkWidget* fid_contSize;
  GtkWidget* fid_contColor;
  GtkWidget* fid_contColor_button;
  GtkListStore* fid_list_store;
  
  // For Fiducial Map Dialog
  GtkWidget* fid_map_surfnum;
  GtkWidget* fid_map_surfname;
  GtkWidget* fid_map_name;
  GtkWidget* fid_map_contourspacing;
  GtkObject* fid_map_cs_adj;
  GtkWidget* fid_map_numcontours;
  GtkObject* fid_map_cn_adj;
  GtkWidget* fid_map_cont_default_range;
  GtkWidget* fid_map_cont_low_range;
  GtkObject* fid_map_lr_adj;
  GtkWidget* fid_map_cont_high_range;
  GtkObject* fid_map_hr_adj;
  GtkWidget* fid_map_occlusion_gradient;
  GtkObject* fmog_adj;
  vector<float> fid_map_orig_numspaces;
  vector<float> fid_map_orig_numconts;
  vector<bool> fid_map_orig_defaultrange;
  vector<float> fid_map_orig_lowrange;
  vector<float> fid_map_orig_highrange;
  vector<float> fid_map_orig_og;
//#endif
};

struct FilesDialog {
  GtkWidget *window;
  PickWindow* pickwins[MAX_SURFS];
  int numPickwins;
  int GraphTitle;
  int NumRows;
  GtkWidget* table;
  GtkWidget* newsurfacebutton;
  vector<FilesDialogRowData*> rowData;
  FilesDialogRowData* newSurfaceRowData;
  bool destroyed; 
  bool sensitive;
  vector<FilePickerCallback*> cbs; // keep around to destroy
};

struct FilesSecondaryDialog {
  GtkWidget* channels;
  GtkWidget* leadlinks;
  GtkWidget* landmarks;
  GtkWidget* fids;
  GtkWidget* nv;

  GtkWidget* window;
  FilesDialogRowData* data;
};

void filesDialogCreate(bool show = true);
void filesSecondaryDialogCreate(FilesDialogRowData* data);

#endif
