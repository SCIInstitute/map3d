#ifndef SCALEDIALOG_H
#define SCALEDIALOG_H

#include "ui_ScaleDialog.h"

#include <QDialog>

// --------------------------- //
// ScaleDialog widget and accessor/helper functions //
class ScaleDialog : public QDialog, public Ui::ScaleDialog {
  Q_OBJECT;
public:
  ScaleDialog(QWidget* parent);

private slots:
  void rangeToggled(bool checked);
  void functionToggled(bool checked);
  void mappingToggled(bool checked);
};

#endif
