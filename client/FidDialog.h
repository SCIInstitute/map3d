#ifndef FIDDIALOG_H
#define FIDDIALOG_H

#include "dialogs.h"
#include "ui_FidDialog.h"

class QCheckBox;
class QRadioButton;

class FidDialog : public QDialog, public Ui::FidDialog 
{
  Q_OBJECT;
public:
  FidDialog(QWidget* parent = NULL);

public slots:
  void on_applyButton_clicked();
  void on_cancelButton_clicked();
  void on_surfComboBox_currentIndexChanged(int index);
private:
  Mesh_Info* _currentMesh;

  QList<QLabel*> _fiducialLabels;
  QList<ColorWidget*> _fiducialColors;
  QList<QSpinBox*> _fiducialSizes;
  QList<QRadioButton*> _fiducialMapRadio;
  QList<QCheckBox*> _fiducialContourCheckBox;

  QList<int> _origContourSpacing;
  QList<int> _origSelectedMap;

  QList<QList<QColor> > _origContourColors;
  QList<QList<float> > _origContourSizes;
  QList<QList<bool> > _origSelectedContours;

};

#if 0
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