#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "dialogs.h"
#include "ui_FileDialog.h"
#include "ui_FileDialogWidget.h"

#include <QList>


class FileDialogWidget;
class Mesh_Info;

class FileDialog : public QDialog, public Ui::FileDialog
{
  Q_OBJECT;
public:
  FileDialog();
  void addRow(Mesh_Info* mesh);

public slots:
  void on_applyButton_clicked ();
  void on_cancelButton_clicked ();
  void on_newSurfaceButton_clicked ();
private:
  QList<FileDialogWidget*> _widgets;
};

class FileDialogWidget : public QWidget, public Ui::FileDialogWidget
{
  Q_OBJECT;
public:
  FileDialogWidget(QWidget* parent, Mesh_Info* mesh);
  ~FileDialogWidget();

public slots:
  // auto-connect slots
  void on_geomLineEdit_editingFinished ();
  void on_geomIndexComboBox_activated ( const QString & text );
  void on_dataLineEdit_editingFinished ();
  void on_dataIndexComboBox_activated ( const QString & text );
  void on_startFrameSpinBox_valueChanged ( int i );
  void on_endFrameSpinBox_valueChanged ( int i );

  void on_geomBrowseButton_clicked ();
  void on_dataBrowseButton_clicked ();
  void on_channelsBrowseButton_clicked ();
  void on_leadlinksBrowseButton_clicked ();
  void on_fiducialBrowseButton_clicked ();
  void on_landmarksBrowseButton_clicked ();

  void on_geomSaveButton_clicked();

  void on_expandButton_clicked ();

  // return true if a mesh was loaded successfully
  bool updateFiles();

  void updateRMS();
public:
  Mesh_Info* mesh;
  QList<Mesh_Info*> subsurf_meshes; // this is all meshes if we select the '*' option in geom index

  bool reload_geom;
  bool reload_data;


  int numseries;
  std::vector<int> numFramesPerSeries;
  std::vector<std::string> timeSeriesLabels;
  MatlabIO::matlabarray* dataArray;
};

void filesDialogCreate(bool show = true);

#endif
