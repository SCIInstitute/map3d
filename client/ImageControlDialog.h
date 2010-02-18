#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include "ui_ImageControlDialog.h"
#include "dialogs.h"

class ImageControlDialog : public QDialog, public Ui::ImageControlDialog
{
  Q_OBJECT;

public:
  ImageControlDialog();
public slots:
  void on_imageToolButton_clicked();
  void on_closeButton_clicked();
  void on_saveAndCloseButton_clicked();
};


#endif
