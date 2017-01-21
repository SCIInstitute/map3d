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

void PickColor(float *, bool modal = false);
void PickSize(float *storage, float factor, const char* str, bool modal = false);
void incrSize(float *storage, float maxChange, float midpoint, float inc);

// --------------------------- //
// SizePicker widget and accessor functions
class SizePicker;

class SizeWidget : public QWidget {
  Q_OBJECT
public:
  SizeWidget(QWidget* parent, int size);

  void setSize(float size) { _size = size; }

  virtual void mousePressEvent ( QMouseEvent * event );
  virtual void mouseDoubleClickEvent ( QMouseEvent * event );
  virtual void paintEvent ( QPaintEvent * event );

  float _size;
  bool _selected;

signals:
  void clicked();
  void doubleClicked();
};

class SizePicker : public QDialog {
  Q_OBJECT
public:

  SizePicker();
  ~SizePicker();

  // public interface to everything to minimize reworking of legacy code

  QDialog *sizepicker;
  SizeWidget *orig_size_widget;
  SizeWidget *selected_size_widget;

  int selected_size;
  float factor; // scale the size*10 by this

  vector < float *> ss_float;
  vector < float *> ss_orig_vals;

public slots:
  void on_cancelButton_clicked();
  void on_closeButton_clicked();
  void sizeSelected();

private:
  QPushButton* cancelButton;
  QPushButton* closeButton;
};


// --------------------------- //
// ColorPicker widget and accessor functions
class ColorPicker;

class ColorWidget : public QWidget {
  Q_OBJECT
public:
  ColorWidget(QWidget* parent, QColor color);

  void setColor(QColor color) { _color[0] = color.redF(); _color[1] = color.greenF(); _color[2] = color.blueF();}
  QColor colorAsQColor() { return QColor(_color[0]*255, _color[1]*255, _color[2]*255); }

  virtual void mousePressEvent ( QMouseEvent * event );
  virtual void mouseDoubleClickEvent ( QMouseEvent * event );
  virtual void paintEvent ( QPaintEvent * event );

  // store as float and not QColor so that we can use a ColorPicker to change it!
  //  (See FidDialog)
  float _color[3];
  bool _selected;

signals:
  void clicked();
  void doubleClicked();
};

class ColorPicker : public QDialog {
  Q_OBJECT
public:

  ColorPicker();
  ~ColorPicker();

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

public slots:
  void on_cancelButton_clicked();
  void on_closeButton_clicked();
  void colorSelected();

private:
  QPushButton* cancelButton;
  QPushButton* closeButton;
};


#endif
