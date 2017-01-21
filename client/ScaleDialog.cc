#include "ScaleDialog.h"
#include "map3d-struct.h"
#include "scalesubs.h"

#include <QList>

ScaleDialog *scaledialog = NULL;
extern Map3d_Info map3d_info;

const char* valueProperty = "Value";

// ------------------- //
// scale picker dialog callbacks, create, and accessor functions

ScaleDialog::ScaleDialog(QWidget* parent) : QDialog(parent)
{
  setupUi(this);

  // the items in these widgets are named numerically because they correspond in order with the integer settings
  //   see scalesubs.h
  QList<QRadioButton*> rangeButtons;
  rangeButtons << range0Button << range1Button << range2Button << 
    range3Button << range4Button << range5Button << range6Button << range7Button;


  for (int i = 0; i < rangeButtons.size(); i++)
  {
    QRadioButton* radioButton = rangeButtons[i];
    radioButton->setProperty(valueProperty, i);
    connect(radioButton, SIGNAL(toggled(bool)), this, SLOT(rangeToggled(bool)));
  }
  
  if (map3d_info.scale_scope >= 0 && map3d_info.scale_scope < rangeButtons.size())
    rangeButtons[map3d_info.scale_scope]->toggle();

  QList<QRadioButton*> functionButtons;
  functionButtons << function0Button << function1Button << function2Button << 
    function3Button << function4Button;
  for (int i = 0; i < functionButtons.size(); i++)
  {
    QRadioButton* radioButton = functionButtons[i];
    radioButton->setProperty(valueProperty, i);
    connect(radioButton, SIGNAL(toggled(bool)), this, SLOT(functionToggled(bool)));
  }

  if (map3d_info.scale_model >= 0 && map3d_info.scale_model < functionButtons.size())
    functionButtons[map3d_info.scale_model]->toggle();

  QList<QRadioButton*> mappingButtons;
  mappingButtons << mapping0Button << mapping1Button << mapping2Button << 
    mapping3Button;
  for (int i = 0; i < mappingButtons.size(); i++)
  {
    QRadioButton* radioButton = mappingButtons[i];
    radioButton->setProperty(valueProperty, i);
    connect(radioButton, SIGNAL(toggled(bool)), this, SLOT(mappingToggled(bool)));
  }
  if (map3d_info.scale_mapping >= 0 && map3d_info.scale_mapping < functionButtons.size())
    mappingButtons[map3d_info.scale_mapping]->toggle();
}

void ScaleDialog::rangeToggled(bool checked)
{
  Q_ASSERT(sender());
  if (checked && sender()) // don't count the unchecked signal
  {
    int val = sender()->property(valueProperty).toInt();
    setScalingRange(val);
  }
}

void ScaleDialog::functionToggled(bool checked)
{
  Q_ASSERT(sender());
  if (checked && sender()) // don't count the unchecked signal
  {
    int val = sender()->property(valueProperty).toInt();
    setScalingFunction(val);
  }
}

void ScaleDialog::mappingToggled(bool checked)
{
  Q_ASSERT(sender());
  if (checked && sender()) // don't count the unchecked signal
  {
    int val = sender()->property(valueProperty).toInt();
    setScalingMapping(val);
  }
}
