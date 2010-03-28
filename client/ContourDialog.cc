#include "ContourDialog.h"
#include "Contour_Info.h"
#include "LegendWindow.h"
#include "MeshList.h"
#include "Surf_Data.h"
#include "Map3d_Geom.h"
#include "WindowManager.h"
#include "eventdata.h"
#include <math.h>

#include <QDebug>

extern Map3d_Info map3d_info;

const char* MeshProperty = "MeshProperty";

enum contTableCols{
  SurfNum, Spacing, NumConts, DefaultRange, LowRange, HighRange, OccGrad, NumCols
};

ContourDialog::ContourDialog()
{
  setupUi(this);

  int row = 1;
  for (MeshIterator mi; !mi.isDone(); ++mi, ++row)
  {
    Mesh_Info* mesh = mi.getMesh();
    int index = row-1;

    // FIX - subsurf, NULL data

    float surfmin=0, surfmax=0;
    float min=0, max=0;
    float origSpacing=0, origNumConts=0, origMin=0, origMax=0, origOcclusion=0;
    bool origFixed = false;
    if (mesh->data) 
    {
      surfmin = mesh->data->potmin;
      surfmax = mesh->data->potmax;
      mesh->data->get_minmax(min, max);
      float spaces;
      if (map3d_info.use_spacing && mesh->contourspacing)
        spaces = mesh->contourspacing;
      else {
        spaces = (max - min)/float(mesh->data->numconts+1);
        mesh->contourspacing = spaces;
      }
      origSpacing = spaces;
      origNumConts = (float) mesh->data->numconts;
      origMin = mesh->data->userpotmin;
      origMax = mesh->data->userpotmax;
      origOcclusion = mesh->cont->occlusion_gradient;
      origFixed = mesh->data->user_scaling;
    }

    origSpacings << origSpacing;
    origNumContours << origNumConts;
    origMins << origMin;
    origMaxes << origMax;
    origOcclusions << origOcclusion;
    origFixedRange << origFixed;

    meshes << mesh;

    float cs_min = (surfmax-surfmin)/1000;  // 1000 because local windows can often be a small subset of the data range
    float high = MAX(fabs(surfmax), fabs(surfmin));

    QLabel* label = new QLabel(QString::number(mesh->geom->surfnum+1), this);
    gridLayout->addWidget(label, row, SurfNum);

    QDoubleSpinBox* spacing = new QDoubleSpinBox(this);
    spacing->setRange(1e-6, surfmax-surfmin);
    spacing->setSingleStep(cs_min);
    spacing->setValue(origSpacing);
    spacing->setProperty(MeshProperty, index);
    spacingBoxes << spacing;  
    gridLayout->addWidget(spacing, row, Spacing);
    
    QSpinBox* conts = new QSpinBox(this);
    conts->setRange(1, 100);
    conts->setSingleStep(1);
    conts->setValue(origNumConts);
    conts->setProperty(MeshProperty, index);
    numContBoxes << conts;  
    gridLayout->addWidget(conts, row, NumConts);

    QCheckBox* fixed = new QCheckBox(this);
    fixed->setChecked(origFixed);
    fixed->setProperty(MeshProperty, index);
    fixedRangeBoxes << fixed;  
    gridLayout->addWidget(fixed, row, DefaultRange);

    QDoubleSpinBox* low = new QDoubleSpinBox(this);
    low->setRange(-3*high, 3*high);
    low->setSingleStep(1);
    low->setValue(origMin);
    low->setProperty(MeshProperty, index);
    minBoxes << low;  
    gridLayout->addWidget(low, row, LowRange);

    QDoubleSpinBox* highBox = new QDoubleSpinBox(this);
    highBox->setRange(-3*high, 3*high);
    highBox->setSingleStep(1);
    highBox->setValue(origMax);
    highBox->setProperty(MeshProperty, index);
    maxBoxes << highBox;  
    gridLayout->addWidget(highBox, row, HighRange);

    QDoubleSpinBox* occlusion = new QDoubleSpinBox(this);
    occlusion->setRange(0, 2*(surfmax-surfmin));
    occlusion->setSingleStep(1);
    occlusion->setValue(origOcclusion);
    occlusion->setProperty(MeshProperty, index);
    occlusionBoxes << occlusion;  
    gridLayout->addWidget(occlusion, row, OccGrad);

    connect(spacing, SIGNAL(valueChanged(double)), this, SLOT(contourCallback()));
    connect(conts, SIGNAL(valueChanged(int)), this, SLOT(contourCallback()));
    connect(low, SIGNAL(valueChanged(double)), this, SLOT(contourCallback()));
    connect(highBox, SIGNAL(valueChanged(double)), this, SLOT(contourCallback()));
    connect(occlusion, SIGNAL(valueChanged(double)), this, SLOT(contourCallback()));
    connect(fixed, SIGNAL(toggled(bool)), this, SLOT(contourCallback()));
  }
  connect(contSpacingRadioButton, SIGNAL(toggled(bool)), this, SLOT(contourCallback()));
  connect(numContsRadioButton, SIGNAL(toggled(bool)), this, SLOT(contourCallback()));
}

void ContourDialog::on_cancelButton_clicked()
{
  // restore old values

  for (int i = 0; i < meshes.size(); i++)
  {
    Mesh_Info* mesh = meshes[i];

    if (mesh->cont && mesh->data)
    {
      mesh->contourspacing = origSpacings[i];
      mesh->data->numconts = origNumContours[i];
      mesh->data->userpotmin = origMins[i];
      mesh->data->userpotmax = origMaxes[i];
      mesh->cont->occlusion_gradient = origOcclusions[i];
      mesh->data->user_scaling = origFixedRange[i];
    }
  }

  Broadcast(MAP3D_UPDATE);
  close();
}

void ContourDialog::on_applyButton_clicked()
{
  Broadcast(MAP3D_UPDATE);
  close();
}


// called from cancel or preview
void ContourDialog::contourCallback()
{
  int numconts;
  map3d_info.use_spacing = contSpacingRadioButton->isChecked();

  Q_ASSERT(sender());

  QVariant rowProp = sender()->property(MeshProperty);
  if (!rowProp.isValid())
    return; // was a global radio button or something

  int row = rowProp.toInt();

  Mesh_Info* mesh = meshes[row];

  //FIX Mesh_List meshes = rowdata->mesh->gpriv->findMeshesFromSameInput(rowdata->mesh);

  if (mesh->data) {
    if (fixedRangeBoxes[row]->isChecked())
      mesh->data->user_scaling = true;
    else
      mesh->data->user_scaling = false;

    mesh->data->userpotmin = (float) minBoxes[row]->value();
    mesh->data->userpotmax = (float) maxBoxes[row]->value();


    if (map3d_info.use_spacing) {
      mesh->contourspacing = (float) spacingBoxes[row]->value();
      numconts = mesh->cont->buildContours();
    }
    else {
      numconts = numContBoxes[row]->value();
      if (mesh->data)
        mesh->data->numconts = numconts;
    }
    mesh->cont->occlusion_gradient = (float) occlusionBoxes[row]->value();

  }
  Broadcast(MAP3D_UPDATE);
}

#if 0
void updateContourDialogValues(Mesh_Info* mesh)
{
  if (!contourdialog)
    return;
  float min,max;
  int numconts;
  double spaces;
  double numspaces;
  
  contourdialog->field_lock = true;

  FilesDialogRowData* rowdata;
  if(!filedialog){
    return;
  }

  // set the value back if you haven't previewed it yet (to avoid confusion)
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(contourdialog->lock_contspacing),map3d_info.use_spacing);


  for (unsigned i = 0; i < filedialog->rowData.size(); i++) {
    rowdata = filedialog->rowData[i];
    if (rowdata->mesh == mesh){
      if (mesh->data) {
        mesh->data->get_minmax(min,max);
        float spacing = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
        if (spacing == 0.f && mesh->data) {
          // initialize if we set up the mesh's first data after the mesh has been opened.
          gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->contourspacing), .01, mesh->data->potmax-mesh->data->potmin);
          gtk_spin_button_set_range(GTK_SPIN_BUTTON(rowdata->cont_occlusion_gradient), 0, mesh->data->potmax-mesh->data->potmin);
          rowdata->orig_numspaces  = (max - min)/float(mesh->data->numconts+1);
          rowdata->orig_numconts = (float) mesh->data->numconts;

          // reset the adjustments
          float surfmin = mesh->data->potmin, surfmax = mesh->data->potmax;
          float cs_min = (surfmax-surfmin)/1000;  // 1000 because local windows can often be a small subset of the data range
          float high = MAX(fabs(surfmax), fabs(surfmin));
          rowdata->cs_adj = gtk_adjustment_new(rowdata->orig_numspaces,1e-6,surfmax-surfmin,cs_min,cs_min*10,cs_min*10);
          rowdata->lr_adj = gtk_adjustment_new(mesh->data?mesh->data->userpotmin:0,-3*high, 3*high,1,5,5);
          rowdata->hr_adj = gtk_adjustment_new(mesh->data?mesh->data->userpotmax:0,-3*high, 3*high,1,5,5);
          gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(rowdata->contourspacing), (GtkAdjustment*)rowdata->lr_adj);
          gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(rowdata->cont_low_range), (GtkAdjustment*)rowdata->lr_adj);
          gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(rowdata->cont_high_range), (GtkAdjustment*)rowdata->hr_adj);

          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),rowdata->orig_numspaces);
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),rowdata->orig_numconts);
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_occlusion_gradient),mesh->data->potmax-mesh->data->potmin);
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range),0);
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range),0);
          continue;
        }
        if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_contspacing)->check_button.toggle_button)){
          // update num conts (round up)
          float nc = (max-min)/
            (gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing)))-1;
          numconts = (int) nc;
          if (nc - (int) nc > 0)
            numconts = (int) nc+1;
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),numconts);
          
        }
        else if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_numconts)->check_button.toggle_button)) {
          // update spacing 
          spaces = (max - min)/float(mesh->data->numconts+1);
          numspaces = spaces;
          gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),numspaces);
        }
      }
    }
  }   
  contourdialog->field_lock = false;
}

void cont_get_minmax(FilesDialogRowData* rowdata, float& min, float& max)
{
  // the value of mesh->data->user_scaling doesn't change until you
  // click preview/okay, so to update the values of the contour dialog
  // correctly if the default button is checked, then we temporaily set
  // user_scaling
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    if (rowdata->mesh->data) {
      bool tempval = rowdata->mesh->data->user_scaling;
      rowdata->mesh->data->user_scaling = false;
      rowdata->mesh->data->get_minmax(min, max);
      rowdata->mesh->data->user_scaling = tempval;
    }
    else {
      min = 0.0f;
      max = 0.0f;
    }
  }
  else {
    max = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_high_range));
    min = (float) gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->cont_low_range));
    if (min == 0.0f && max == 0.0f && rowdata->mesh->data) {
      // set it to the current min/max
      // rather than just setting it to 0
      contourdialog->field_lock = true;
      bool tempval = rowdata->mesh->data->user_scaling;
      rowdata->mesh->data->user_scaling = false;
      rowdata->mesh->data->get_minmax(min, max);
      rowdata->mesh->data->user_scaling = tempval;
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_occlusion_gradient), max-min);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range), max);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range), min);
      //contourdialog->field_lock = false;
    }
  }
}

void modifyContourDialogRow_RangeChange(FilesDialogRowData* rowdata)
{
  float min,max;
  float numconts;
  double spaces;
  double numspaces;
  
  if (contourdialog->field_lock)
    return;
  contourdialog->field_lock = true;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    gtk_widget_set_sensitive(rowdata->cont_high_range, false);
    gtk_widget_set_sensitive(rowdata->cont_low_range, false);
  }
  else {
    gtk_widget_set_sensitive(rowdata->cont_high_range, true);
    gtk_widget_set_sensitive(rowdata->cont_low_range, true);
  }

  cont_get_minmax(rowdata, min, max);

  if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_contspacing)->check_button.toggle_button)){
    // update num conts if range changed (round up)
    float nc = (max-min)/
      (gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing)))-1;
    numconts = nc;
    if (nc - (int) nc > 0)
      numconts = nc+1;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),numconts);
  }
  else if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(contourdialog->lock_numconts)->check_button.toggle_button)) {    
    // update spacing if the range changed
    spaces = (max - min)/float(gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->numcontours))+1);
    numspaces = spaces;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),numspaces);
  }
  contourdialog->field_lock = false;

}

void modifyContourDialogRow_ContSpaceChange(FilesDialogRowData* rowdata)
{
  float min,max;
  int numconts;
  
  if (contourdialog->field_lock)
    return;
  contourdialog->field_lock = true;

  cont_get_minmax(rowdata, min, max);

  float spacing = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    // default range: update the num conts if the contour spacing changed (round up)
    float nc = (max-min)/spacing - 1;
    numconts = nc;
    if (nc - (int) nc > 0)
      numconts = (int) nc+1;
  }
  else {
    // user range: change the range
    numconts = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rowdata->numcontours));
    float range = (numconts+1)*spacing;
    float range_ratio = range / (max-min);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range), max*range_ratio);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range), min*range_ratio);
  }
  if (numconts > MAX_CONTOURS) {
    numconts = MAX_CONTOURS;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),(max-min)/(numconts+1));
  }
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->numcontours),numconts);
  contourdialog->field_lock = false;
}


void modifyContourDialogRow_NumContChange(FilesDialogRowData* rowdata)
{
  float min,max;
  double spaces;
  
  if (contourdialog->field_lock)
    return;
  contourdialog->field_lock = true;
  cont_get_minmax(rowdata, min, max);

  int numconts = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->numcontours));
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rowdata->cont_default_range))) {
    // update spacing if the num conts changed

    // round down at the end to make sure we get the number of conts we ask for
    spaces = (max - min)/float(numconts+1) - .000005;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->contourspacing),spaces);
  }
  else {
    // user range: change the range
    float spacing = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rowdata->contourspacing));
    float range = (numconts+1)*spacing;
    float range_ratio = range / (max-min);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_high_range), max*range_ratio);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rowdata->cont_low_range), min*range_ratio);
  }

  contourdialog->field_lock = false;
}
#endif