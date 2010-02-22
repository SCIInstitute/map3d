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

#include <QColor>
#include <QDialog>
#include <QFrame>

using std::string;
using std::vector;
using std::map;

class QPushButton;

#ifdef _WIN32
#pragma warning(disable:4505)  /* quiet visual c++ */
#endif


class GeomWindow;
class PickWindow;
class Mesh_Info;

// exit dialog function
void map3d_quit(QWidget* parentWindow);
bool prompt_overwrite(QWidget* parentWindow, QString filename);

// misc functions

char* shorten_filename(char* x);
// allows for the editing of the filename in the files dialog.  Since that is 
// the short filename (without dirs), we must combine with the long name
void replaceFilenameWithShort(const char* longname, const char* shortname, char* combinedname);

string get_path(char* x);
double roundedNum(double num);

// FIX - mask
QString PickFile(QWidget* parent, bool save);

void QStringToCharPtr(QString in, char* out, int size);

void PickColor(float *);
void PickSize(float *storage, float factor, char* str);
void incrSize(float *storage, float maxChange, float midpoint, float inc);

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


// --------------------------- //
// ColorPicker widget and accessor functions
class ColorPicker;

class ColorWidget : public QFrame {
  Q_OBJECT
public:
  ColorWidget(ColorPicker* parent, QColor color, bool clickable);

  void setColor(QColor color) { _color = color; }

  virtual void mousePressEvent ( QMouseEvent * event );
  virtual void paintEvent ( QPaintEvent * event );

  ColorPicker* _colorPicker;
  QColor _color;
  bool _clickable;
};

class ColorPicker : public QDialog {
  Q_OBJECT
public:

  ColorPicker();

  // public interface to everything to minimize reworking of legacy code

  QDialog *colorpicker;
  ColorWidget *orig_color_widget;
  ColorWidget *selected_color_widget;

  float selected_color[3];
  vector < float *> cs_float;
  vector < float *> cs_orig_vals;

  // this is so we can clear the storage on a subsequent call to PickColor
  // right now, kind of hacky, it is set at the end of GeomWindowMenu
  bool post_change;

  void closeEvent(QCloseEvent*) { hide(); }

  void colorSelected(ColorWidget* widget);

public slots:
  void on_cancelButton_clicked();
  void on_closeButton_clicked();

private:
  QPushButton* cancelButton;
  QPushButton* closeButton;
};

//include these so we only have to include dialogs.h
#include "ContourDialog.h"
#include "FidDialog.h"
#include "FileDialog.h"
#include "ImageControlDialog.h"
#include "ScaleDialog.h"


#endif
