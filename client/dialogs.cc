/* dialogs.cxx */

#include "dialogs.h"
#include "map3d-struct.h"
#include "GeomWindowMenu.h"
#include "Contour_Info.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "MainWindow.h"
#include "Map3d_Geom.h"
#include "ProcessCommandLineOptions.h"
#include "ParseCommandLineOptions.h"
#include "Surf_Data.h"
#include "WindowManager.h"
#include "eventdata.h"
#include "Transforms.h"
#include "savescreen.h"
#include "savestate.h"
#include "PickWindow.h"
#include "pickinfo.h"
#include "scalesubs.h"
#include "readfiles.h"
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

// dialog globals
GtkWidget *quitwindow = NULL;
ColorPicker *cp = NULL;
SizePicker *sp = NULL;
extern int fstep;


// will bring up a dialog and either quit or do nothing
void map3d_quit(QWidget* parentWindow)
{
  if (QMessageBox::question(parentWindow, "Map3d", "Really Quit?", QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Ok)
    exit(0);
}

bool prompt_overwrite(QWidget* parentWindow, QString filename)
{
  return QMessageBox::question(parentWindow, "Map3d", "Overwrite " + filename + "?", QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Ok;
}

QString PickFile(QWidget* parentWindow, bool save)
{
  if (save)
    return QFileDialog::getSaveFileName(parentWindow);
  else
    return QFileDialog::getOpenFileName(parentWindow);
}

void QStringToCharPtr(QString in, char* out, int size)
{
  strncpy(out, in.toLatin1().data(), size);
}


double roundedNum(double num){
  
  double x;
  double mant, ipart;
  
  x = (num/(pow(10.0,(double)(((long)log10(fabs(num)))-1))));
  mant = modf(x, &ipart);
  
  //x = round(x); not sgi compatible
  if(fabs(mant) >= 0.5){
    if(x > 0){
      x = ceil(x);
    }
    else{
      x = floor(x);
    }
  }
  else{
    if(x > 0){
      x = floor(x);
    }
    else{
      x = ceil(x);
    }
  }
  
  x = x*(pow(10.0,(double)((long)log10(fabs(num)) -1)));
  
  return x;
}

char* shorten_filename(char* x)
{
  // get rid of the leading directories in filename - find last occurrence of / or '\'
  char * slash = strrchr(x, '/');
  if (!slash) {
    slash = strrchr(x,'\\');
  }
  if (!slash)
    slash = x;
  else
    slash = slash+1; // to get rid of the /
  return slash;
}

string get_path(char* x)
{
  // get rid of the leading directories in filename - find last occurrence of / or '\'
  char * slash = strrchr(x, '/');
  string path(x);
  if (!slash) {
    slash = strrchr(x,'\\');
  }
  if (!slash)
    slash = x;
  else
    slash = slash +1; // to get rid of the /
  int i = path.find(slash);
  path.erase(i);
  return path;
}

void parse_filename(char* n, char* initials, char* date, char* info, char* num){
  int i = 0;
  char temp;
  char* ptr;
  
  //find initials
  temp = n[i];
  while(!isdigit(temp)){
    i++;
    temp = n[i];
  }
  strncpy(initials,n, i);
  initials[i] = '\0';
  
  //ptr = n(filename) - initials
  ptr = strchr(n, temp);
  //printf("%s\n",ptr);
  
  //find date
  strncpy(date,ptr,7);
  date[7] = '\0';
  
  //ptr = n(filename) - initials - date
  ptr = ptr + 8;
  //printf("%s\n",ptr);
  
  
  //find info
  i = strlen(ptr) -8;
  strncpy(info,ptr,i);
  info[i - 1] = '\0';
  
  //ptr = n(filename) - initials - date - info
  ptr = ptr +i;
  //printf("%s\n",ptr);
  
  //find run number
  i = strlen(ptr) - 5;
  strncpy(num,ptr,i);
  num[i] = '\0';
  
  //ptr = n(filename) - initials - date - info - run number
  ptr = ptr +i;
  //printf("%s\n",ptr);
  
  //   printf("filename = %s\n",n);
  //   printf("initials = %s\n",initials);
  //   printf("date = %s\n",date);
  //   printf("info = %s\n",info);
  //   printf("num = %s\n",num);
}


void replaceFilenameWithShort(const char* longname, const char* shortname, char* combinedname)
{
  char tmp[256];
  strcpy(tmp, longname);
  bool slash = false;
  int i;
  // look for a slash in longname, so we can append shortname to that current dir
  for (i = strlen(tmp)-1; i >= 0; i--)
    if (tmp[i] == '/' || tmp[i] == '\\') {
      slash = true;
      break;
    }
      tmp[i] = 0;
  
  // if shortname is an absolute path, then only use that
  // unix, if it starts with /
  // windows, if it is X:, where X is a drive letter.
  if (shortname[0] == '/' || shortname[1] == ':')
    slash = false;
  
  if (slash)
    sprintf(combinedname, "%s/%s", tmp, shortname);
  else
    strcpy(combinedname, shortname);
}


extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;

void ColorPicker::on_cancelButton_clicked()
{
  for (unsigned int i = 0; i < cp->cs_float.size(); i++) {
    cp->cs_float[i][0] = cp->cs_orig_vals[i][0];
    cp->cs_float[i][1] = cp->cs_orig_vals[i][1];
    cp->cs_float[i][2] = cp->cs_orig_vals[i][2];
    delete cp->cs_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per cs_float
  }
  cp->cs_float.clear();
  cp->cs_orig_vals.clear();
  close();
  Broadcast(MAP3D_REPAINT_ALL, 0);
}

void ColorPicker::on_closeButton_clicked()
{
  for (unsigned int i = 0; i < cp->cs_float.size(); i++) {
    delete cp->cs_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per cs_float
  }
  cp->cs_float.clear();
  cp->cs_orig_vals.clear();
  close();
}

ColorWidget::ColorWidget(QWidget* parent, QColor color) : 
  QWidget(parent), _selected(false)
{
	setColor(color);
}

void ColorWidget::paintEvent ( QPaintEvent* )
{
  QPainter painter(this);
  QBrush brush(colorAsQColor());
  QPen pen(Qt::black);
  pen.setWidth(3);

  painter.setBrush(brush);

  if (_selected)
    painter.setPen(pen);

  painter.drawRect(0, 0, width(), height());
}

void ColorWidget::mousePressEvent ( QMouseEvent * )
{
  emit clicked();
}

void ColorWidget::mouseDoubleClickEvent ( QMouseEvent *)
{
  emit doubleClicked();
}

ColorPicker::ColorPicker()
{
  selected_color_widget = NULL;
  
  // create the window and set window parameters
  setWindowTitle("Select Color");

  // add sub-boxes
  QVBoxLayout* layout = new QVBoxLayout(this);
  QFrame* colorsFrame = new QFrame(this);
  layout->addWidget(colorsFrame);

  QFrame* buttonFrame = new QFrame(this);
  layout->addWidget(buttonFrame);


  QGridLayout* gridLayout = new QGridLayout(colorsFrame);
  gridLayout->setContentsMargins(0,0,0,0);
  QHBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame);
  buttonLayout->setContentsMargins(0,0,0,0);
  
  
  QLabel* label = new QLabel("Original:", buttonFrame);
  buttonLayout->addWidget(label);
  orig_color_widget = new ColorWidget(this, QColor());
  orig_color_widget->setMinimumSize(20, 15);
  buttonLayout->addWidget(orig_color_widget);

  // add last row and selected color box into it.
  buttonLayout->addStretch(10);

  cancelButton = new QPushButton("Cancel", buttonFrame);
  buttonLayout->addWidget(cancelButton);
  closeButton = new QPushButton("Close", buttonFrame);
  buttonLayout->addWidget(closeButton);

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(on_cancelButton_clicked()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(on_closeButton_clicked()));
  
  int numcolors = 28;
  // add colors
  for (int i = 0; i < numcolors; i++) {
    
    // interpolate through all combinations of 0,.5,.1 for color
    float r = 1;
    float g = 1;
    float b = 1;
    
    if (i < 27) {
      r = i * .5f;
      g = 0;
      b = 0;
      
      while (r - 1.5 >= 0) {
        r -= 1.5;
        g += .5;
      }
      while (g - 1.5 >= 0) {
        g -= 1.5;
        b += .5;
      }
    }

    QColor color(r*255, g*255, b*255);
    ColorWidget* frame = new ColorWidget(this, color);
    connect(frame, SIGNAL(clicked()), this, SLOT(colorSelected()));
    frame->setMinimumSize(20, 20);
    gridLayout->addWidget(frame, i/(numcolors/2), i % (numcolors/2));    
  }
}

ColorPicker::~ColorPicker()
{
  // clear out the values for the next selection (set in GeomWindowMenu)
  cp->cs_float.clear();
  for (unsigned i = 0; i < cp->cs_orig_vals.size(); i++) {
    delete cp->cs_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per cs_float
  }
  cp->cs_orig_vals.clear();

  cp = NULL;
}

void ColorPicker::colorSelected()
{
  ColorWidget* widget = dynamic_cast<ColorWidget*>(sender());

  if (selected_color_widget)
    selected_color_widget->_selected = false;

  selected_color_widget = widget;
  widget->_selected = true;

  for (unsigned int i = 0; i < cp->cs_float.size(); i++) {
    cp->cs_float[i][0] = widget->_color[0];
    cp->cs_float[i][1] = widget->_color[1];
    cp->cs_float[i][2] = widget->_color[2];
  }
  Broadcast(MAP3D_REPAINT_ALL, 0);
  update();
}

void PickColor(float *storage, bool modal)
{
  float *orig = new float[3];
  orig[0] = storage[0];
  orig[1] = storage[1];
  orig[2] = storage[2];
  
  if (!cp)
    cp = new ColorPicker();

  if (modal)
    cp->setModal(true);
  cp->setAttribute(Qt::WA_DeleteOnClose);

  cp->selected_color_widget = 0;

  cp->cs_float.push_back(storage);
  cp->cs_orig_vals.push_back(orig);
  
  cp->selected_color[0] = storage[0];
  cp->selected_color[1] = storage[1];
  cp->selected_color[2] = storage[2];
  
  QColor color;
  color.setRedF(cp->cs_float[0][0]);
  color.setGreenF(cp->cs_float[0][1]);
  color.setBlueF(cp->cs_float[0][2]);

  cp->orig_color_widget->setColor(color);

  if (modal)
    cp->exec();
  else 
  {
    cp->show();
    cp->update();  
  }
}

void SizePicker::on_cancelButton_clicked()
{
  for (unsigned int i = 0; i < sp->ss_float.size(); i++) {
    *(sp->ss_float[i]) = *(sp->ss_orig_vals[i]);
    delete sp->ss_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per ss_float
  }
  sp->ss_float.clear();
  sp->ss_orig_vals.clear();
  close();
  Broadcast(MAP3D_REPAINT_ALL, 0);
}

void SizePicker::on_closeButton_clicked()
{
  for (unsigned int i = 0; i < sp->ss_float.size(); i++) {
    delete sp->ss_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per ss_float
  }
  sp->ss_float.clear();
  sp->ss_orig_vals.clear();
  close();
}

SizeWidget::SizeWidget(QWidget* parent, int size) : 
  QWidget(parent), _size(size), _selected(false)
{
}

void SizeWidget::paintEvent ( QPaintEvent* )
{
  QPainter painter(this);
  QPen pen(Qt::black);

  if (_selected)
  {
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawRect(0, 0, width(), height());
  }

  pen.setWidth(1);
  painter.setPen(pen);
  QBrush brush(Qt::black);
  painter.setBrush(brush);
  painter.drawRect(4, height()/2.0f - _size/2.0f, width()-8, _size);

}

void SizeWidget::mousePressEvent ( QMouseEvent * )
{
  emit clicked();
}

void SizeWidget::mouseDoubleClickEvent ( QMouseEvent * )
{
  emit doubleClicked();
}

SizePicker::SizePicker()
{
  selected_size_widget = NULL;
  
  // add sub-boxes
  QVBoxLayout* layout = new QVBoxLayout(this);
  QFrame* colorsFrame = new QFrame(this);
  layout->addWidget(colorsFrame);

  QFrame* buttonFrame = new QFrame(this);
  layout->addWidget(buttonFrame);


  QGridLayout* gridLayout = new QGridLayout(colorsFrame);
  gridLayout->setContentsMargins(0,0,0,0);
  QHBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame);
  buttonLayout->setContentsMargins(0,0,0,0);
  
  
  QLabel* label = new QLabel("Original:", buttonFrame);
  buttonLayout->addWidget(label);
  orig_size_widget = new SizeWidget(this, 1);
  orig_size_widget->setMinimumSize(50, 25);
  buttonLayout->addWidget(orig_size_widget);

  // add last row and selected color box into it.
  buttonLayout->addStretch(10);

  cancelButton = new QPushButton("Cancel", buttonFrame);
  buttonLayout->addWidget(cancelButton);
  closeButton = new QPushButton("Close", buttonFrame);
  buttonLayout->addWidget(closeButton);

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(on_cancelButton_clicked()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(on_closeButton_clicked()));
  
  int numsizes = 10;
  // add colors
  for (int i = 1; i <= numsizes; i++) {
    
    SizeWidget* frame = new SizeWidget(this, i);
    connect(frame, SIGNAL(clicked()), this, SLOT(sizeSelected()));
    frame->setMinimumSize(50, 25);
    gridLayout->addWidget(frame, i, 1);    
  }
}

SizePicker::~SizePicker()
{
  sp->ss_float.clear();
  for (unsigned i = 0; i < sp->ss_orig_vals.size(); i++) {
    delete sp->ss_orig_vals[i];  // they were allocated in PickColor, and there should be one of them per cs_float
  }
  sp->ss_orig_vals.clear();
  sp = NULL;
}

void SizePicker::sizeSelected()
{
  SizeWidget* widget = dynamic_cast<SizeWidget*>(sender());

  if (selected_size_widget)
    selected_size_widget->_selected = false;

  selected_size_widget = widget;
  widget->_selected = true;
  int size = widget->_size;
  for (unsigned int i = 0; i < sp->ss_float.size(); i++) {
    *(sp->ss_float[i]) = (size*10)/factor;
  }
  Broadcast(MAP3D_REPAINT_ALL, 0);
  update();
}

void PickSize(float *storage, float factor, const char* str, bool modal)
{
  float *orig = new float;
  *orig = *storage;
  
  if (!sp)
    sp = new SizePicker();

  sp->selected_size_widget = 0;
  sp->setAttribute(Qt::WA_DeleteOnClose);
  if (modal)
    sp->setModal(true);


  sp->setWindowTitle(QString("Select ") + str);
  sp->ss_float.push_back(storage);
  sp->ss_orig_vals.push_back(orig);
  
  sp->selected_size = *sp->ss_float[0];
  
  sp->orig_size_widget->_size = sp->selected_size;
  sp->factor = factor;
  sp->update();
  sp->show();
}

// storage is a pointer to the value to change,
//   maxChange is the highest possible range of values
//   inc is what fraction (decimal) to change each time
//   midpoint is the middle possible value (*usually* maxChange / 2)
//   range: ( midpoint-(maxChange/2), midpoint+(maxChange/2) ]
void incrSize(float *storage, float maxChange, float midpoint, float inc)
{
  float s = *storage;
  float increment = maxChange * inc;
  s += increment;
  if (s < midpoint - (maxChange / 2) + fabs(inc) * maxChange) // normally to clamp it
    s = midpoint - (maxChange / 2) + fabs(inc) * maxChange; // at 1 instead of 0
  else if (s > midpoint + (maxChange / 2))
    s = midpoint + (maxChange / 2);
  *storage = s;
}

