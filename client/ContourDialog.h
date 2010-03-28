#ifndef CONTOURDIALOG_H
#define CONTOURDIALOG_H

#include "ui_ContourDialog.h"
#include "dialogs.h"
#include <QList>

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;

// BJW - I chose to implement this as a grid layout with dynamic content
//   as opposed to the following alternatives:
//   a) new widget for each row - does not necessarily line up content nicely
//   b) table/treeView - does not display spinboxes/etc. well unless you create
//     a QItemDelegate subclass, which would then require you to double-click
//     on the entry to start editing.  I wanted more immediate access.
class ContourDialog : public QDialog, public Ui::ContourDialog
{
  Q_OBJECT;
public:
  ContourDialog();

public slots:
  void on_applyButton_clicked ();
  void on_cancelButton_clicked ();
  void contourCallback();

private:
  // original values
  QList<double> origSpacings;
  QList<double> origNumContours;
  QList<double> origMins;
  QList<double> origMaxes;
  QList<double> origOcclusions;
  QList<bool> origFixedRange;

  QList<Mesh_Info*> meshes;

  // widgets
  QList<QDoubleSpinBox*> spacingBoxes;
  QList<QSpinBox*> numContBoxes;
  QList<QDoubleSpinBox*> minBoxes;
  QList<QDoubleSpinBox*> maxBoxes;
  QList<QDoubleSpinBox*> occlusionBoxes;
  QList<QCheckBox*> fixedRangeBoxes;
};

#if 0

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

#endif
