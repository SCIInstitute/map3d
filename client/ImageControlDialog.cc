#include "ImageControlDialog.h"
#include "MeshList.h"
#include "Map3d_Geom.h"
#include "WindowManager.h"
#include "map3d-struct.h"
#include "savescreen.h"
#include "savestate.h"
#include "dialogs.h"

extern Map3d_Info map3d_info;

ImageControlDialog::ImageControlDialog()
{
  setupUi(this);
  imageLineEdit->setText(map3d_info.imagefile);
}

void ImageControlDialog::on_closeButton_clicked()
{
  strncpy(map3d_info.imagefile, imageLineEdit->text().toLatin1().data(), 1024);
  map3d_info.saving_animations = saveAnimationsCheckBox->isChecked();

  done(0);
}

void ImageControlDialog::on_saveAndCloseButton_clicked()
{
  on_closeButton_clicked();
  SaveScreen();
}

void ImageControlDialog::on_imageToolButton_clicked()
{
  QString filename = PickFile(this, true);
  imageLineEdit->setText(filename);
}
