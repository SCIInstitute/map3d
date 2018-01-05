#include "FileDialog.h"
#include "MainWindow.h"
#include "Map3d_Geom.h"
#include "MeshList.h"
#include "PickWindow.h"
#include "ProcessCommandLineOptions.h"
#include "Surf_Data.h"
#include "Transforms.h"
#include "WindowManager.h"
#include "eventdata.h"
#include "map3d-struct.h"
#include "pickinfo.h"
#include "ContourDialog.h"
#include "Contour_Info.h"

#include "matlabarray.h"

#include <math.h>

#include <QFileDialog>
#include <QDebug>
#include <QMenu>

FileDialog* filedialog;
//extern SaveDialog *savedialog;
//extern ContourDialog *contourdialog;
//extern FidDialog *fiddialog;
//extern FidMapDialog *fidmapdialog;
extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
//extern FilePicker *fp;
// ------------------- //
// files dialog helpers, callbacks, create, and accessor functions

const char* surfPropName = "SurfaceNum";

void FileDialogWidget::on_geomLineEdit_editingFinished ()
{
  int num = GetNumGeoms(geomLineEdit->text().toLatin1().data());

  geomIndexComboBox->clear();
  QStringList indexEntries;
  for (int i = 1; i <= num; i++)
    indexEntries << QString::number(i);

  if (num > 1)
    indexEntries << "all";

  geomIndexComboBox->addItems(indexEntries);
  geomIndexComboBox->setCurrentIndex(0);
  reload_geom = true;    
}

void FileDialogWidget::on_geomIndexComboBox_activated ( const QString & text )
{
  Q_UNUSED(text);
  reload_geom = true;    
}

void FileDialogWidget::on_dataLineEdit_editingFinished ()
{
  QString filename = dataLineEdit->text();
  if (filename == "")
    return;

  GetDataFileInfo(filename.toStdString(), numseries, numFramesPerSeries, timeSeriesLabels, dataArray);

  if (filename.endsWith(".tsdfc") || filename.endsWith(".mat"))
  {    
    QStringList items;
    for(int i = 0; i < numseries; i++){
      items << QString::fromStdString(timeSeriesLabels[i]);
    }

    dataIndexComboBox->clear();
    dataIndexComboBox->addItems(items);
  }
  
  int ds = 0;  // if changing the first filename, always grab the first index
  int numframes = numFramesPerSeries[ds];
  startFrameSpinBox->setRange(1, numframes);
  endFrameSpinBox->setRange(1, numframes);

  startFrameSpinBox->setValue(1);
  endFrameSpinBox->setValue(numframes);
  
  updateRMS(); 
  reload_data = true;
}
  
  
void FileDialogWidget::on_dataIndexComboBox_activated ( const QString & text )
{
  Q_UNUSED(text);
  char filename[256];
  strncpy(filename, dataLineEdit->text().toLatin1().data(), 256);

  if(strcmp(filename,"")!=0)
  {  
    int ds = dataIndexComboBox->currentIndex();    
    int numframes = numFramesPerSeries[ds];

    startFrameSpinBox->setRange(1, numframes);
    endFrameSpinBox->setRange(1, numframes);

    startFrameSpinBox->setValue(1);
    endFrameSpinBox->setValue(numframes);
    
    updateRMS(); 
    reload_data = true;
  }
}
  
void FileDialogWidget::on_startFrameSpinBox_valueChanged ( int i )
{
  Q_UNUSED(i);
  rmsWidget->update();
  reload_data = true;
}
  
void FileDialogWidget::on_endFrameSpinBox_valueChanged ( int i )
{
  Q_UNUSED(i);
  rmsWidget->update();
  reload_data = true;
}

// these happen when you click the '...' buttons
void FileDialogWidget::on_geomBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select geometry file");

  geomLineEdit->setText(newFile);
  on_geomLineEdit_editingFinished();
}
  
void FileDialogWidget::on_dataBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select data file");
  dataLineEdit->setText(newFile);
  on_dataLineEdit_editingFinished();
}

void FileDialogWidget::on_channelsBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select channels file");
  channelsLineEdit->setText(newFile); 
}

void FileDialogWidget::on_leadlinksBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select leadlinks file");
  leadlinksLineEdit->setText(newFile);
}

void FileDialogWidget::on_fiducialBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select Fiducial file");
  fiducialLineEdit->setText(newFile);
}

void FileDialogWidget::on_landmarksBrowseButton_clicked ()
{
  QString newFile = QFileDialog::getOpenFileName(parentWidget(), "Select Landmarks file");
  landmarksLineEdit->setText(newFile);
}

void FileDialogWidget::on_geomSaveButton_clicked ()
{
  QMenu menu;
  QAction* transformAction = menu.addAction("Apply transformations");
  QAction* noTransformAction = menu.addAction("Ignore transformations");
  QAction* selectedAction = menu.exec(QCursor::pos());

  if (selectedAction == NULL)
    return;

  QString newFile = QFileDialog::getSaveFileName(parentWidget(), "Select geometry file to save");
  QByteArray fileUTF = newFile.toUtf8(); // so char* will be in scope for function call to saveMeshes

  Mesh_List meshes = mesh->gpriv->findMeshesFromSameInput(mesh);
  vector<bool> transforms(meshes.size(), selectedAction == transformAction);
  bool success = SaveMeshes(meshes, transforms, fileUTF.data());

  if (success)
  {
    geomLineEdit->setText(newFile);
    on_geomLineEdit_editingFinished();
  }
}
  
void FileDialogWidget::on_expandButton_clicked ()
{
  bool visible = otherOptionsFrame->isVisible();
  if (visible)
  {
    expandButton->setText("+");
    otherOptionsFrame->setVisible(false);
  }
  else
  {
    expandButton->setText("-");
    otherOptionsFrame->setVisible(true);
  }

}

void FileDialog::addRow(Mesh_Info* mesh)
{
  bool empty_mesh = false;
  int surfnumber, window;
  if (!mesh) {
    // we clicked the addRow button
    surfnumber = _widgets.size()+1; // make one-based
    window = numGeomWindows(); // put in in a new window by default
    mesh = new Mesh_Info; // initialize an empty mesh
    empty_mesh = true;
  }
  else {
    surfnumber = mesh->geom->surfnum;
    window = mesh->gpriv->geomWinId;
  }

  FileDialogWidget* widget = new FileDialogWidget(surfaceScrollArea, mesh);
  widget->show();
  surfaceScrollArea->show();
  widget->setProperty(surfPropName, surfnumber);
  if (!empty_mesh)
      widget->geomSaveButton->setEnabled(true);
  _widgets.append(widget);

  // insert because we want the spacer at the end
  surfaceScrollAreaLayout->insertWidget(surfaceScrollAreaLayout->count()-1, widget);

  widget->surfLabel->setText("Surface #" + QString::number(surfnumber));
  widget->winComboBox->setCurrentIndex(window); // the combo box is 0-based

  if (!empty_mesh)
  {
    widget->geomLineEdit->setText(mesh->mysurf->geomfilename);
    widget->on_geomLineEdit_editingFinished();
    if (mesh->mysurf->geomsurfnum <= 0)
      widget->geomIndexComboBox->setCurrentIndex(widget->geomIndexComboBox->count()-1);
    else
      widget->geomIndexComboBox->setCurrentIndex(mesh->mysurf->geomsurfnum-1); // geomsurfnum is 1-based

    widget->dataLineEdit->setText(mesh->mysurf->potfilename);
    widget->on_dataLineEdit_editingFinished();

    widget->dataIndexComboBox->setCurrentIndex(mesh->mysurf->timeseries);
    //data series

    //frame start, end, step
    if (mesh->data)
    {
      widget->startFrameSpinBox->setValue(mesh->data->ts_start+1);
      widget->endFrameSpinBox->setValue(mesh->data->ts_end+1);
      widget->frameStepSpinBox->setValue(mesh->data->ts_sample_step);
    }

    widget->leadlinksLineEdit->setText(mesh->mysurf->llfilename);
    widget->landmarksLineEdit->setText(mesh->mysurf->lmfilename);
    widget->channelsLineEdit->setText(mesh->mysurf->chfilename);
    widget->fiducialLineEdit->setText(mesh->mysurf->fidfilename);
  }
  else
  {
    foreach(QLineEdit *lineEdit, widget->findChildren<QLineEdit*>())
    {
      lineEdit->setText("");
    }
    foreach(QSpinBox *spinBox, widget->findChildren<QSpinBox*>())
    {
      spinBox->setValue(1);
    }
  }

  widget->updateRMS();
}

void FileDialogWidget::updateRMS()
{
  // as much as it would be nice to use the same code here and in updateFiles, we can't because,
  //   the RMS needs to load the entire range, and the updateFiles needs to limit based on the frame selection
  char geom[256];
  char data[256];
  char ch[256];

  strncpy(geom, geomLineEdit->text().toLatin1().data(), 256);
  
  // gs should be 0 if the * was selected in a multisurf case, which is what we want 
  int gs = geomIndexComboBox->currentIndex() + 1;
  if (gs == geomIndexComboBox->count())
    gs = 0;
  
  if (dataLineEdit->text().toLatin1() == "")
    return;
  strncpy(data, dataLineEdit->text().toLatin1().data(), 256);
  int ds = 0;
  
  if (strcmp(GetExtension(data), ".tsdfc") == 0 || strcmp(GetExtension(data), ".mat") == 0) {
    ds = dataIndexComboBox->currentIndex();
    if (ds == -1)
      ds = 0;
  }
  
  strncpy(ch, channelsLineEdit->text().toLatin1().data(), 256);
    
  int numframes = numFramesPerSeries[ds];
  
  // we need to load separate surfaces here so the user can browse different data while keeping
  // the originals displayed.
  if (strcmp(geom, "") != 0 && strcmp(data, "") != 0) {
    // load geom and data so we can update the rms curve
    Surf_Input* input = new Surf_Input;
    Init_Surf_Input(input);
    input->parent = map3d_info.gi;
    input->geomfilename = new char[256];
    input->potfilename = new char[256];
    input->chfilename = new char[256];

    input->geomsurfnum = gs;
    input->timeseries = ds;
    input->displaysurfnum = 1; // who cares
    strcpy(input->geomfilename,geom);
    strcpy(input->potfilename,data);
    strcpy(input->chfilename,ch);
    input->ts_end = numframes-1;
    input->ts_start = 0;
    
    input->preloadedDataArray = dataArray;

    Mesh_Info* mesh = new Mesh_Info;
    Mesh_List currentMeshes;
    currentMeshes.push_back(mesh);
    
    // turn off report level, since we'll just be duplicating what we've seen already
    int reportlevel = map3d_info.reportlevel;
    map3d_info.reportlevel = 0;
    Mesh_List returnedMeshes = FindAndReadGeom(input,currentMeshes,LOAD_RMS_DATA);
    map3d_info.reportlevel = reportlevel;
    
    if (returnedMeshes.size() > 0) {
      if (rmsWidget->mesh)
        delete rmsWidget->mesh;
      rmsWidget->mesh = mesh;
    }
    else {
      delete mesh;
    }
    
    rmsWidget->fileWidget = this;
    rmsWidget->rms = true;
    rmsWidget->show();
    rmsWidget->update();
  }
}

bool FileDialogWidget::updateFiles()
{
  bool successfulNewSurf = true;

  // we don't need this anymore, since we pushed apply.
  if (rmsWidget->mesh)
	  delete rmsWidget->mesh;
  rmsWidget->mesh = NULL;

    
  bool meshNotLoaded = (mesh->mysurf == NULL);
  // as far as I can tell, surf, ds, dstart and dend are 1-based to the user,
  // but gs is not
  int surf = property(surfPropName).toInt();
  int win = winComboBox->currentIndex();

  // grabbing the char* from a QString will not stay in scope after the QByteArray (toLatin1) is destroyed
  //   so we need to copy them out
  char geom[256];
  char data[256];
  char ch[256];
  char ll[256];
  char lm[256];
  char fi[256];

  strncpy(geom, geomLineEdit->text().toLatin1().data(), 256);
  
  // gs should be 0 if the * was selected in a multisurf case, which is what we want 
  int gs = geomIndexComboBox->currentIndex() + 1; // it's one based
  if (gs == geomIndexComboBox->count())
    gs = 0;
    
  strncpy(data, dataLineEdit->text().toLatin1().data(), 256);
  int ds = 0;
  
  if (strcmp(GetExtension(data), ".tsdfc") == 0 || strcmp(GetExtension(data), ".mat") == 0) {
    ds = dataIndexComboBox->currentIndex();
    if (ds == -1)
      ds = 0;
  }
  
  int dstart = startFrameSpinBox->value()-1;
  int dend = endFrameSpinBox->value()-1;
  int dstep = frameStepSpinBox->value();
  
  strncpy(ch, channelsLineEdit->text().toLatin1().data(), 256);
  strncpy(ll, leadlinksLineEdit->text().toLatin1().data(), 256);
  strncpy(lm, landmarksLineEdit->text().toLatin1().data(), 256);
  strncpy(fi, fiducialLineEdit->text().toLatin1().data(), 256);
  
  if (mesh == 0 && strcmp(geom, "") == 0) {
    // we clicked 'new surface' and 'apply' without putting in a filename
    successfulNewSurf = false;
    return false;
  }

  if (strcmp(geom, "") == 0) {
    // not sure if this needs to be its own special case...
    return false;
  }

  if (!mesh->mysurf && strcmp(geom, "") != 0) {
    //new surface
    GeomWindow* geomwin;
    Surf_Input* input = new Surf_Input;
    Init_Surf_Input(input);
    input->parent = map3d_info.gi;
    input->geomfilename = new char[256];
    input->potfilename = new char[256];
    input->chfilename = new char[256];
    input->lmfilename = new char[256];
    input->llfilename = new char[256];
    input->fidfilename = new char[256];

    input->geomsurfnum = gs;
    input->timeseries = ds;
    input->displaysurfnum = surf;
    strcpy(input->geomfilename,geom);
    strcpy(input->potfilename,data);
    strcpy(input->chfilename,ch);
    strcpy(input->llfilename,ll);
    strcpy(input->lmfilename,lm);
    strcpy(input->fidfilename, fi);
    
    input->ts_end = dend;
    input->ts_start = dstart;
    input->ts_sample_step = dstep;

    input->preloadedDataArray = dataArray;

    map3d_info.scale_frame_set = 0;
    
    Mesh_List currentMeshes;
    currentMeshes.push_back(mesh);
    Mesh_List returnedMeshes = FindAndReadGeom(input,currentMeshes,RELOAD_NONE);

	// at this point, we don't need it anymore, and get rid of the memory as soon as we can
    delete input->preloadedDataArray;
    input->preloadedDataArray = NULL;
    dataArray = NULL;

    if (returnedMeshes.size() > 0) {
      if (win >= numGeomWindows()) {
        // new window as well
        geomwin = GeomWindow::GeomWindowCreate(0,0,0,0);
        
        if (masterWindow && masterWindow->startHidden)
          masterWindow->show();
        
      }
      else
        geomwin = GetGeomWindow(win);
      for (unsigned i = 0; i < returnedMeshes.size(); i++)
        geomwin->addMesh(returnedMeshes[i]);

      map3d_info.lockgeneral = LOCK_FULL;
      geomwin->dominantsurf = -1;
      
      // update the non-files dialogs' information
      // FIX addRowToOtherDialogs();
    }
    else {
      successfulNewSurf = false;
      delete mesh->mysurf;
      mesh->mysurf = 0;
    }
    GlobalMinMax(); // needs to be called here, as the version in FindAndReadData only works on meshes added to windows
    return true;  // we don't need to (and shouldn't) go through the reload section
  }
  
  GeomWindow* newWindow = GetGeomWindow(win);
  if (mesh->gpriv != newWindow) {
    // move mesh from one window to another
    if (newWindow == NULL) {
      // new geom window
      GeomWindow::GeomWindowCreate(0,0,0,0);
      newWindow = GetGeomWindow(win);
    }
    GeomWindow* g = mesh->gpriv;
    for (unsigned i = 0; i < g->meshes.size(); i++) {
      // move all meshes with the same surf input to other window (multi-surf geom)
      Mesh_Info* tmp = g->meshes[i];
      if (tmp->mysurf == mesh->mysurf) {
        g->removeMesh(tmp);
        newWindow->addMesh(tmp);
        newWindow->show();
        tmp->gpriv = newWindow;
        i--;
      }
    }
    if (g->meshes.size() == 0) {
      g->hide();
    }
  }
  
  // decide if we need to reload the geom - it doesn't compensate for the geomsurfnum anywhere else
  if (gs != mesh->mysurf->geomsurfnum)
    reload_geom = true;
  
  
  if (reload_geom) {
    // reload geom info
    mesh->mysurf->geomfilename = new char[256];
    if (!mesh->mysurf->potfilename) mesh->mysurf->potfilename = new char[256];
    if (!mesh->mysurf->chfilename) mesh->mysurf->chfilename = new char[256];
    if (!mesh->mysurf->lmfilename) mesh->mysurf->lmfilename = new char[256];
    if (!mesh->mysurf->llfilename) mesh->mysurf->llfilename = new char[256];
    if (!mesh->mysurf->fidfilename) mesh->mysurf->fidfilename = new char[256];
    
    strcpy(mesh->mysurf->geomfilename, geom);
    strcpy(mesh->mysurf->chfilename, ch);
    strcpy(mesh->mysurf->llfilename, ll);
    strcpy(mesh->mysurf->lmfilename, lm);
    strcpy(mesh->mysurf->fidfilename, fi);

	mesh->mysurf->preloadedDataArray = dataArray;

    mesh->mysurf->geomsurfnum = gs;
    printf("Reloading Geom: Surf %d: Win %d, %s@%d, %s@%d, %d-%d\n",surf, win, geom, gs, data, ds, dstart, dend);
    Mesh_List currentMeshes = mesh->gpriv->findMeshesFromSameInput(mesh);
    Mesh_List returnedMeshes = FindAndReadGeom(mesh->mysurf, currentMeshes, RELOAD_GEOM);

	// at this point, we don't need it anymore, and get rid of the memory as soon as we can
	delete mesh->mysurf->preloadedDataArray;
	mesh->mysurf->preloadedDataArray = NULL;
	dataArray = NULL;


    if (returnedMeshes.size() > 0) {
      // TODO - change the name of the surface in the save and contour dialogs

      if (meshNotLoaded) {
        mesh->gpriv->addMesh(mesh);
      }
      if (returnedMeshes.size() > currentMeshes.size()) {
        // this is in case we load all surfaces of a multi-surf geom over a surf with less surfaces
        Mesh_List tmp; // pop them off the main list, to keep them in order
        while (mesh->gpriv->meshes.back()->mysurf != mesh->mysurf) {
          tmp.push_back(mesh->gpriv->meshes.back());
          mesh->gpriv->meshes.pop_back();
        }
        
        for (unsigned i = currentMeshes.size(); i < returnedMeshes.size(); i++)
          mesh->gpriv->addMesh(returnedMeshes[i]);
        
        while (tmp.size() > 0) {
          mesh->gpriv->meshes.push_back(tmp.back());
          tmp.pop_back();
        }            
      }
    }
    else {
      mesh->gpriv->removeMesh(mesh);
    }
    //mesh->gpriv = map3d_info.geomwins[win];
    mesh->gpriv->recalcMinMax();
  }
  if (reload_data && strcmp(data, "") != 0) {
    if (!mesh->mysurf->potfilename) mesh->mysurf->potfilename = new char[256];
    strcpy(mesh->mysurf->potfilename, data);
    mesh->mysurf->timeseries = ds;
    mesh->mysurf->ts_end = dend;
    mesh->mysurf->ts_start = dstart;
    map3d_info.scale_frame_set = 0;
    
    printf("Reloading Data: Surf %d: Win %d, %s@%d, %s@%d, %d-%d\n",surf, win, geom, gs, data, ds+1, dstart+1, dend+1);
    
    Mesh_List currentMeshes = mesh->gpriv->findMeshesFromSameInput(mesh);
    FindAndReadGeom(mesh->mysurf, currentMeshes, RELOAD_DATA);
    // FIX updateContourDialogValues(mesh);
    Broadcast(MAP3D_MENU, frame_reset);
  }
  GlobalMinMax(); // needs to be called here, as the version in FindAndReadData only works on meshes added to windows
  return true;
}

void filesDialogCreate(bool show /*=true*/)
{
  if (!filedialog) {
    filedialog = new FileDialog;
    //filedialog->destroyed = true;

    QSet<Surf_Input*> surfs_used;
    for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
      Mesh_Info* mesh = mi.getMesh();
      if (!surfs_used.contains(mesh->mysurf))
      {
        // there can be more than one mesh per Surf_Input, but the addRow really works on 
        //   Surf_Input
        filedialog->addRow(mesh);
        surfs_used.insert(mesh->mysurf);
      }
    }
  }


  if (show){
    filedialog->show();
    return;
  }
}

FileDialog::FileDialog() :
    QDialog(0, Qt::Dialog | Qt::WindowTitleHint)
{
  setupUi(this);
}

void FileDialog::on_applyButton_clicked()
{
  foreach (FileDialogWidget* widget, _widgets)
  {
    widget->updateFiles();
  }

  Broadcast(MAP3D_UPDATE,0);
  close();
  deleteLater();
  filedialog = 0;
}

void FileDialog::on_cancelButton_clicked()
{
  close();
  deleteLater();
  filedialog = 0;
}

void FileDialog::on_newSurfaceButton_clicked()
{
  addRow(0);
}


FileDialogWidget::FileDialogWidget(QWidget* parent, Mesh_Info* mesh) : QWidget(parent), mesh(mesh)
{
  setupUi(this);
  geomSaveButton->setIcon(style()->standardIcon(QStyle::SP_DriveFDIcon));
  geomSaveButton->setToolTip("Save Geometry");
  geomSaveButton->setEnabled(false);
  
  rmsWidget->rms = true;
  rmsWidget->mesh = NULL;

  // plan for the future
  dataSaveButton->hide();
  landmarksSaveButton->hide();
  channelsSaveButton->hide();

  otherOptionsFrame->setVisible(false);
  reload_geom = 0;
  reload_data = 0;
  mesh = 0;
  dataArray = NULL;
}

FileDialogWidget::~FileDialogWidget()
{
  if (dataArray)
  {
    delete dataArray;
    dataArray = NULL;
  }
  if (rmsWidget->mesh)
	  delete rmsWidget->mesh;
  rmsWidget->mesh = NULL;
}