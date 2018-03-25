/* PickWindow.cxx */

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <float.h>

#include "PickWindow.h"
#include "Contour_Info.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "WindowManager.h"
#include "dialogs.h"
#include "ContourDialog.h"
#include "eventdata.h"
#include "glprintf.h"
#include "map3d-struct.h"
#include "pickinfo.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "scalesubs.h"
#include "savescreen.h"
#include "MainWindow.h"
#include "FileDialog.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>


#define PICK_INSIDE  0
#define PICK_OUTSIDE 1
#define PICK_LEFT    2
#define PICK_RIGHT   3

extern Map3d_Info map3d_info;
extern const char *units_strings[5];
extern int fstep, cur, fstart, fend;
extern int key_pressed;
extern int pick;
extern int delay;
extern MainWindow *masterWindow;

PickInfo *pickstack[100] = { 0 };
int pickstacktop = -1;

#define CHAR_SIZE 70
#define CHAR_BIG CHAR_SIZE * 4
#define CHAR_MED CHAR_SIZE * 3
//#define PROJ_SIZE 1000.f

static const int min_width = 100;
static const int min_height = 100;
static const int default_width = 328;
static const int default_height = 144;

enum pickmenu { axes_color, graph_color, full_screen, graph_width_menu, toggle_subseries_mode};


PickWindow::PickWindow(QWidget* parent) : Map3dGLWidget(parent)
{
  SetStyle(0);
  mesh = 0;
  pick = 0;
}

PickWindow::PickWindow(QWidget* parent, bool rms) : Map3dGLWidget(parent, (rms?RMSWINDOW:TIMEWINDOW),"Time Signal", 260, 120), rms(rms)
{
  if (wintype == TIMEWINDOW) {
    SetStyle(1);
  }
  else {
    SetStyle(0);
  }
  mesh = 0;
  fileWidget = 0;
}

//static
PickWindow* PickWindow::PickWindowCreate(int _width, int _height, int _x, int _y)
{
  if (map3d_info.numPickwins >= MAX_PICKS) {
    printf("Warning: cannot create more than %d Time Series Windows\n", MAX_PICKS);
    return 0;
  }
  PickWindow* win = new PickWindow(masterWindow ? masterWindow->childrenFrame : NULL, false);
  win->positionWindow(_width, _height, _x, _y, default_width, default_height);

  map3d_info.pickwins[map3d_info.numPickwins] = win;
  map3d_info.numPickwins++;

  // don't show until mesh and pick have been set up
  //win->show();
  return win;
}

PickWindow::~PickWindow()
{
}

void PickWindow::initializeGL()
{
  axiscolor[0] = .75;
  axiscolor[1] = .75;
  axiscolor[2] = .1f;
  axiscolor[3] = 1;
  
  graphcolor[0] = .1f;
  graphcolor[1] = .75f;
  graphcolor[2] = .1f;
  graphcolor[3] = 1.0f;
  graph_width = 2;
  
  Map3dGLWidget::initializeGL();
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glShadeModel(GL_FLAT);
  
  glDepthMask(GL_FALSE);

  if (mesh && mesh->gpriv) {
    GeomWindow* geom = (GeomWindow *) mesh->gpriv;
    bgcolor[0] = geom->bgcolor[0];
    bgcolor[1] = geom->bgcolor[1];
    bgcolor[2] = geom->bgcolor[2];
    bgcolor[3] = geom->bgcolor[3];
    
    fgcolor[0] = geom->fgcolor[0];
    fgcolor[1] = geom->fgcolor[1];
    fgcolor[2] = geom->fgcolor[2];
    fgcolor[3] = geom->fgcolor[3];
  }
  else {
    bgcolor[0] = bgcolor[1] = bgcolor[2] = bgcolor[3] = 0;
    fgcolor[0] = fgcolor[1] = fgcolor[2] = fgcolor[3] = 1;
  }
}

void PickWindow::Destroy()
{
  int i, j = -1;
  
  for (i = 0; i <= mesh->pickstacktop; i++) {
    if (mesh->pickstack[i]->pickwin == this) {
      delete mesh->pickstack[i];
      mesh->pickstacktop--;
      for (j = i; j <= mesh->pickstacktop; j++)
        mesh->pickstack[j] = mesh->pickstack[j + 1];
      break;
    }
  }
  for (i = 0; i < map3d_info.numPickwins; i++) {
    if (map3d_info.pickwins[i] == this) {
      map3d_info.numPickwins--;
      for (j = i; j < map3d_info.numPickwins; j++)
        map3d_info.pickwins[j] = map3d_info.pickwins[j + 1];
      break;
    }
  }
  if (j != -1) {
    map3d_info.pickwins[j] = this;
    DestroyWindow(this);
  }
  mesh->gpriv->update();
  pick = NULL;
  mesh = NULL;
}

void PickWindow::paintGL()
{
	if (mesh == NULL)
		return;
  if (!rms && pick == NULL)
    return;
  // this compensates for the "Retina" display ratio.  See http://doc.qt.io/qt-5/highdpi.html
  //  (for some reason the picking doesn't need this)
  int pixelFactor = QApplication::desktop()->devicePixelRatio();
  glViewport(0, 0, width()*pixelFactor, height()*pixelFactor);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  //projection in pixels
  gluOrtho2D(0, width(), 0, height());
  glMatrixMode(GL_MODELVIEW);

  glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  DrawNode();
}

void PickWindow::enterEvent(QEvent*)
{
  if (rms) return;
  mesh->curpicknode = pick->node;
  mesh->gpriv->update();
}

void PickWindow::leaveEvent(QEvent*)
{
  if (rms) return;
  mesh->curpicknode = -1;
  mesh->gpriv->update();
}

void PickWindow::mouseReleaseEvent(QMouseEvent* event)
{
  if (rms)
  {
    RMSButtonRelease(event);
    return;
  }
  
  if(!mesh)
    return;
  
  // redraw all relevant windows.  The frame should already be selected
  if (click && event->button() == Qt::LeftButton && mesh->data) {
    if (!map3d_info.lockframes) {
      // if advancing in time only affects this surface
      if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
          map3d_info.scale_scope != SLAVE_FRAME) {
        mesh->gpriv->UpdateAndRedraw();
      }
      // it affects at least one other surface...
      else {
        Broadcast(MAP3D_UPDATE);
      }
    }
    else 
      Broadcast(MAP3D_UPDATE);
    
    // if animating, save since we redrew the new frame
    if (map3d_info.saving_animations) {
      SaveScreen();
    }
  }
}

// in here we hack the original values of event->x and event->y
void PickWindow::mousePressEvent(QMouseEvent* event)
{
  if (rms)
  {
    RMSButtonPress(event);
    return;
  }

  state = 1;

  setMoveCoordinates(event);
  
  if(!mesh)
    return;
  
  float distance;
  button = event->button();
  int x = (int)event->x();
  int y = (int)(height() - event->y());
  
  int deltaFrames = 0;

  click = false;
  if (event->button() == Qt::RightButton) { // right click - menu popup
    int menu_data = OpenMenu(mapToGlobal(event->pos()));
    if (menu_data >= 0)
      MenuEvent(menu_data);
  }

  /* LEFT MOUSE DOWN = select frame in graph */
  else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier && mesh->data) {
    map3d_info.scale_frame_set = 0;
    if (y < height() * topoffset && y > height() * bottomoffset &&
        x > width() * leftoffset && x < width() * rightoffset) {
      click = true;

      int left, right;
      getFrameRange(mesh->data->CurrentSubseries(), left, right);
      int numFrames = right - left;

      distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width(); // percentage of the graph the click is over
      int newFrame = left + distance * (numFrames-1);
      deltaFrames = -(mesh->data->framenum - newFrame);

      map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP)
        ? mesh->groupid : -1;
      ComputeLockFrameData();

      // clamp delta frames to the lock frame data
      if (deltaFrames < 0 && map3d_info.lockframes) 
        deltaFrames = MAX(deltaFrames, fstart-cur);
      else if (map3d_info.lockframes)
        deltaFrames = MIN(deltaFrames, fend-cur);


      if (!map3d_info.lockframes) {
        mesh->data->FrameAdvance(deltaFrames);
        // FIX updateContourDialogValues(mesh);
      }
      else 
        Broadcast(MAP3D_PICK_FRAMES, this, event, deltaFrames);
    }
    
  }
  update();
}

void PickWindow::RMSButtonRelease(QMouseEvent * event)
{

}

void PickWindow::RMSButtonPress(QMouseEvent * event)
{
  state = 1;
  
  if(!mesh)
    return;
  
  //ComputeLockFrameData();
  float distance;
  int button = event->button();
  int x = (int)event->x();
  int y = (int)(height() - event->y());
  
  click = false;
  
  if ((button == Qt::LeftButton) && mesh->data) {
    map3d_info.scale_frame_set = 0;
    if (y < height() * topoffset && y > height() * bottomoffset &&
        x > width() * leftoffset && x < width() * rightoffset) {
      click = true;
      distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width();
      distance *= (mesh->data->numframes-1);
      
      // set window_line to 0 if the click is closer to the left line, 1 if closer to right
      int clicked_frame = (int) (distance+1);

      Q_ASSERT(fileWidget);
      int start = fileWidget->startFrameSpinBox->value();
      int end = fileWidget->endFrameSpinBox->value();
      window_line = (abs(clicked_frame-start) > abs(clicked_frame-end)) ? 1 : 0;
      
      if(window_line == 0){
        if(distance > end) {
          fileWidget->startFrameSpinBox->setValue(end);
        }else{
          fileWidget->startFrameSpinBox->setValue(distance+1);
        }
      }
      else if(window_line == 1){
        if(distance < start){
          fileWidget->endFrameSpinBox->setValue(start);
        }else{
          fileWidget->endFrameSpinBox->setValue(distance+1);
        }
      }
    }
  }

  update();
}

void PickWindow::mouseMoveEvent(QMouseEvent* event)
{
  if (rms)
  {
    RMSMotion(event);
    return;
  }
  
  if(!mesh)
    return;
  
  float distance;
  int deltaFrames = 0;
  int x = (int)event->x();
  int y = (int)(height() - event->y());
     
  if (event->buttons() == Qt::LeftButton && event->modifiers() & Qt::AltModifier) {
    moveEvent(event);
  }
  else if (event->buttons() == Qt::MidButton && event->modifiers() & Qt::AltModifier) 
  {
    sizeEvent(event);
  }
  
  else if (event->buttons() == Qt::LeftButton && mesh->data) {
    //x -= width / 10;
    int left, right;
    getFrameRange(mesh->data->CurrentSubseries(), left, right);
    int numFrames = right - left;

    distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width(); // percentage of the graph the click is over
    int newFrame = left + distance * (numFrames - 1);
    deltaFrames = -(mesh->data->framenum - newFrame);

    deltaFrames = -(mesh->data->framenum - newFrame);
    map3d_info.scale_frame_set = 0;
    
    map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP)
      ? mesh->groupid : -1;
    ComputeLockFrameData();

    if (y < height() * topoffset && y > height() * bottomoffset &&
        x > width() * leftoffset && x < width() * rightoffset) {
      click = true; // to signify that we are dragging inside the pick window

      // clamp delta frames to the lock frame data
      if (deltaFrames < 0 && map3d_info.lockframes) 
        deltaFrames = MAX(deltaFrames, fstart-cur);
      else if (map3d_info.lockframes)
        deltaFrames = MIN(deltaFrames, fend-cur);
    }
    
    // we used to be in the frame but we're not anymore, so redraw the relevant windows
    else if (click) {
      click = false;
      //      for (i = 0; i < length; i++) {
      if (x <= width() * .1) {
        deltaFrames = (map3d_info.lockframes ? fstart - cur : mesh->data->ts_start - mesh->data->framenum);
      }
      else if (x >= width() * .95) {
        deltaFrames = (map3d_info.lockframes ? fend - cur : mesh->data->ts_end - mesh->data->framenum);
      }
      
      if (!map3d_info.lockframes) {
        mesh->data->FrameAdvance(deltaFrames);
        // if advancing in time only affects this surface
        if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
            map3d_info.scale_scope != SLAVE_FRAME) {
          mesh->gpriv->UpdateAndRedraw();
        }
        // it affects at least one other surface...
        else {
          Broadcast(MAP3D_UPDATE, this, event);
        }
      }
      else {
        Broadcast(MAP3D_PICK_FRAMES, this, event, deltaFrames);
        Broadcast(MAP3D_UPDATE, this, event);
      }
      
      // if animating, save since we redrew the new frame
      if (map3d_info.saving_animations) {
        SaveScreen();
      }
      return;
      }
    else {
      return;
    }
    
    // update the frames but don't redraw
    if (!map3d_info.lockframes)
      mesh->data->FrameAdvance(deltaFrames);
    else
      Broadcast(MAP3D_PICK_FRAMES, this, event, deltaFrames);
    update();
  }
}

void PickWindow::RMSMotion(QMouseEvent* event)
{
  if(!mesh)
    return;
  
  float distance;
  int x = (int)event->x();
  int y = (int)(height() - event->y());


  int stat = PICK_INSIDE;
  
  
  if (event->buttons() == Qt::LeftButton && mesh->data) {
    //x -= width / 10;
    distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width();
    
    distance = distance * (mesh->data->numframes-1);
    
    
    if (y < height() * topoffset && y > height() * bottomoffset &&
        x > width() * leftoffset && x < width() * rightoffset) {
      click = true; // to signify that we are dragging inside the pick window
    }
    
    // we used to be in the frame but we're not anymore, so set frames to max extent
    else if (click) {
      stat = PICK_OUTSIDE;
      click = false;
      //      for (i = 0; i < length; i++) {
      if (x <= width() * .1) {
        
        stat = PICK_LEFT;
        distance = 0;
      }
      else if (x >= width() * .95) {
        stat = PICK_RIGHT;
        distance = 1;
      }
      
    }
    
    int start = fileWidget->startFrameSpinBox->value();
    int end = fileWidget->endFrameSpinBox->value();
    // set the frame vals - window_line was set in ButtonPress based on which line we were closer to
    if(window_line == 0)
    {
      if(distance > end){
        fileWidget->startFrameSpinBox->setValue(end);
      }else{
        fileWidget->startFrameSpinBox->setValue(distance+1);
      }
    }
    if(window_line == 1)
    {
      if(distance < start){
        fileWidget->endFrameSpinBox->setValue(start);
      }else{
        fileWidget->endFrameSpinBox->setValue(distance+1);
      }
    }
  }

//delete Data;
}

void PickWindow::keyReleaseEvent(QKeyEvent* event)
{
  if (rms) return;
  GeomWindow *gpriv = mesh->gpriv;
  if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
    mesh->gpriv->keyReleaseEvent(event);
  }
}

void PickWindow::keyPressEvent(QKeyEvent* event)
{
  if (rms) return;

  int key = event->key();
  char keyChar = event->text().toLatin1()[0];
  if (keyChar == 'p') {
    MenuEvent(full_screen);
  }
  else if (keyChar == 'q') {
    pick->show = 0;
    setVisible(!pick->show);
  }
  else if (key == Qt::Key_Escape) {
    pick->show = 0;
    Destroy();
  }
  else if (key == Qt::Key_Left || key == Qt::Key_Right || keyChar == 'f' || key == Qt::Key_Plus || key == Qt::Key_Minus)
  {
    mesh->gpriv->keyPressEvent(event);
  }
}


void PickWindow::DrawNode()
{
  int loop;
  float a, b;
  float d;
  float min = FLT_MAX, max = -FLT_MAX;
  float right = rightoffset;
  float left = leftoffset;
  float top = topoffset;
  float bottom = bottomoffset;
  
  float pos[3] = { 5.f, (float)height(), 0 };
  //float norm[3]={0,0,-1}, up[3]={0,1,0};
  //float aspect = .62f * height / width;
  float coloroffset = .5f;
  
  // this is for the case in the files dialog with an empty row
  if (!mesh)
    return;
  Surf_Data* data = mesh->data;  
  
  /* Find the extrema of the time signal */
  if (data && rms) {
    for (loop = 0; loop < data->numframes; loop++) {
    	if (data->rmspotvals[loop] < min)
	      min = data->rmspotvals[loop];
	    if (data->rmspotvals[loop] > max)
	      max = data->rmspotvals[loop];
    }
  }
  else if (data){
    for (loop = 0; loop <data->numframes; loop++) {
    	if (data->potvals[loop][pick->node] < min)
	      min = data->potvals[loop][pick->node];
	    if (data->potvals[loop][pick->node] > max)
	      max = data->potvals[loop][pick->node];
    }
  }
  else {
    min = 0;
    max = 0;
  }
  //ComputeLinearMappingCoefficients(min, max, -.6 * PROJ_SIZE, .5 * PROJ_SIZE, a, b);
  
  a = ((top - bottom) * height()) / (max - min);
  b = (bottom * height() * max - min * top * height()) / (max - min);
  
  
  if (bgcolor[0] + .3 > 1 || bgcolor[1] + .3 > 1 || bgcolor[2] + .3 > 1)
    coloroffset = -.5;
  
  glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);
  
  QString toRender;
  if (showinfotext && !rms) {
    
    pos[1] = height() - getFontHeight(mesh->gpriv->med_font);
    
    
    if (data) {
      // print real frame num if start is not beginning
      int real_frame = data->getRealFrameNum();
      int zero_frame = data->zerotimeframe * data->ts_sample_step + data->ts_start;
      if (data->ts_start != 0 || data->ts_sample_step != 1)
        toRender = QString("Frame: %1 (%2)   Time: %3%4").arg(data->framenum + 1).arg(real_frame + 1)
          .arg((real_frame-zero_frame) * map3d_info.frames_per_time_unit).arg(map3d_info.time_unit);
      else
        toRender = QString("Frame: %1   Time: %2%3").arg(data->framenum + 1)
          .arg((real_frame-zero_frame) * map3d_info.frames_per_time_unit).arg(map3d_info.time_unit);
    }
    else {
      toRender = "Frame: ---";
    }
    renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);
    toRender = "";
    
    if (data) {
      toRender = "Value: " + QString::number(data->potvals[data->framenum][pick->node], 'g', 3);
    }
    else {
      toRender = "Value: ---";
    }
    pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;

    renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);
    toRender = "";
    
    pos[0] = 5;
    pos[1] = 3;
    
    if (mesh->geom->channels)
      //fix channel printing
      toRender = "Node# " + QString::number(pick->node + 1) + 
        " (Ch " + QString::number(mesh->geom->channels[pick->node] + 1) + ")";
    else
      toRender = "Node# " + QString::number(pick->node + 1);

    renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);
    pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender);
    
    toRender = "Surface# " + QString::number(mesh->geom->surfnum);
    if (mesh->geom->subsurf > 0)
      toRender += "-" + QString::number(mesh->geom->subsurf);

    pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;
    renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);    
    
    /* axis labels */
    pos[0] = width()/2 - getFontWidth(mesh->gpriv->med_font, "Time")/2;
    pos[1] = height()/7.5f;

    renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, "Time");

    pos[0] = 2;
    pos[1] = b;
    if (data && data->units != 0) {
      renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, units_strings[data->units - 1]);
    }
    else {
      renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, "data");
      
    }
    
  }  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  /* draw vertical axis line, and zero horizontal line */
  glLineWidth(1);
  glColor3f(axiscolor[0], axiscolor[1], axiscolor[2]);
  glBegin(GL_LINES);
  if (data) {
    d = width() / (float)(data->numframes-1) * (right - left); //graph's domain
    
    glVertex3f(left * width() + d*(float)data->zerotimeframe, top * height(), 0);
    glVertex3f(left * width() + d*(float)data->zerotimeframe, bottom * height(), 0);
    glVertex3f((left - .02f) * width(), b, 0);
    glVertex3f((right + .02f) * width(), b, 0);
    glEnd();
  }
  else{
    glVertex3f(left * width(), top * height(), 0);
    glVertex3f(left * width(), bottom * height(), 0);
    glVertex3f((left - .02f) * width(), b, 0);
    glVertex3f((right + .02f) * width(), b, 0);
    glEnd();
  }
  /* draw time signal */
  if (data) {

    // this is a lambda because of all the dumb little variables we need
    auto DrawPlot = [=](int leftFrame, int rightFrame)
    {
      float d = width() / (float)(rightFrame - leftFrame - 1) * (right - left); //graph's domain for each segment
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      glLineWidth(graph_width);
      glColor3f(graphcolor[0], graphcolor[1], graphcolor[2]);
      glBegin(GL_LINE_STRIP);
      // loop is which frame to draw, counter is which position to draw it at
      for (int counter = 0, loop = leftFrame; loop <= rightFrame; loop++, counter++) {
        // draw to the next frame if it exists
        if (loop >= data->numframes)
          break;
        if (rms) {
          glVertex3f(left * width() + d * counter, data->rmspotvals[loop] * a + b, 0);
        }
        else {
          glVertex3f(left * width() + d * counter, data->potvals[loop][pick->node] * a + b, 0);
        }
      }
      glEnd();
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_BLEND);
    };

    if (wintype == TIMEWINDOW && map3d_info.subseries_mode && mesh->data->subseriesToStack.size() > 0)
    {
      // draw each stacked subseries on top of each other
      // TODO - some transparency or something
      // crashes: for (int subseriesNum : mesh->data->subseriesToStack)
      for (int i = 0; i < mesh->data->subseriesToStack.size(); i++)
      {
        int subseriesNum = mesh->data->subseriesToStack[i];
        int leftFrame, rightFrame;
        getFrameRange(subseriesNum, leftFrame, rightFrame);
        d = width() / (float)(rightFrame - leftFrame - 1) * (right - left); //graph's domain for each segment

        DrawPlot(leftFrame, rightFrame);
      }
    }
    else
    {
      int leftFrame, rightFrame;
      // will draw everything if not in subseries node, else the current subseries
      getFrameRange(mesh->data->CurrentSubseries(), leftFrame, rightFrame);
      d = width() / (float)(rightFrame - leftFrame - 1) * (right - left); //graph's domain for each segment
      DrawPlot(leftFrame, rightFrame);

      //draw fiducial markers
      if (wintype == TIMEWINDOW) {
        int index = 0;
        //    printf("mesh->fidConts.size() %d\n",mesh->fidConts.size());
        //    printf("1data->fids[fidsets].numfidleads %d\n", data->fids[fidsets].numfidleads);

        if (pick->node < data->fids.numfidleads) {
          //      printf("2data->fids.numfidleads %d\n", data->fids[fidsets].numfidleads);
          for (int i = 0; i < data->fids.leadfids[pick->node].numfids; i++) {
            short fidType = data->fids.leadfids[pick->node].fidtypes[i];
            for (unsigned j = 0; j < mesh->fidConts.size(); j++) {
              if (mesh->fidConts[j]->datatype == fidType)
                index = j;
            }
            glLineWidth(1);
            glColor3f(mesh->fidConts[index]->fidcolor.redF(),
              mesh->fidConts[index]->fidcolor.greenF(),
              mesh->fidConts[index]->fidcolor.blueF());
            glBegin(GL_LINES);
            glVertex3f(left * width() + d * (data->fids.leadfids[pick->node].fidvals[i] - leftFrame), b + (.1f * height()), 0);
            glVertex3f(left * width() + d * (data->fids.leadfids[pick->node].fidvals[i] - leftFrame), b - (.1f * height()), 0);
            glEnd();
            //            printf("index %d i %d\n", index, i);
            index++;
          }
        }
        else {
          index += data->fids.numfidtypes;
        }

        if (data->subseriesStartFrames.size() > 0)
        {
          glLineWidth(1);
          glColor3f(axiscolor[0], axiscolor[1], axiscolor[2]);
          glBegin(GL_LINES);

          for (int i = 0; i < data->subseriesStartFrames.size(); i++)
          {
            int subseriesStart = data->subseriesStartFrames[i];
            glVertex3f(left * width() + d * ((float)subseriesStart - leftFrame), top * height(), 0);
            glVertex3f(left * width() + d * ((float)subseriesStart - leftFrame), bottom * height(), 0);
          }
          glEnd();
        }

        // draw vertical frame line
        glLineWidth(1);
        glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
        glBegin(GL_LINES);
        glVertex3f(left * width() + d * ((float)data->framenum - leftFrame), (top + .02f) * height(), 0);
        glVertex3f(left * width() + d * ((float)data->framenum - leftFrame), (bottom - .02f) * height(), 0);
        glEnd();

      }
      else if (wintype == RMSWINDOW) {
        // vertical frame line
        glLineWidth(1);
        glColor3f(0, 1, 0);
        glBegin(GL_LINES);

        int start = fileWidget->startFrameSpinBox->value();
        int end = fileWidget->endFrameSpinBox->value();
        glVertex3f(left * width() + d * (start - 1), (top + .02f) * height(), 0);
        glVertex3f(left * width() + d * (start - 1), (bottom - .02f) * height(), 0);
        glColor3f(1, 0, 0);
        glVertex3f(left * width() + d * (end - 1), (top + .02f) * height(), 0);
        glVertex3f(left * width() + d * (end - 1), (bottom - .02f) * height(), 0);
        glEnd();
      }
    }
    
  }
}

// the range [left, right) - right is exclusive - as the condition of a for loop
void PickWindow::getFrameRange(int subseriesNum, int& left, int& right)
{
  if (mesh->data == NULL)
  {
    left = right = 0;
    return;
  }
  if (map3d_info.subseries_mode && subseriesNum < mesh->data->subseriesStartFrames.size())
  {
    left = mesh->data->subseriesStartFrames[subseriesNum];

    if (subseriesNum < mesh->data->subseriesStartFrames.size() - 1)
      right = mesh->data->subseriesStartFrames[subseriesNum + 1];
    else
      right = mesh->data->numframes;
  }
  else
  {
    left = 0;
    right = mesh->data->numframes;
  }
}

void PickWindow::SetStyle(int x)
{
  switch (x) {
    case 0:                      //full size
      showinfotext = 0;
      
      leftoffset = bottomoffset = .025f;
      topoffset = rightoffset = .975f;
      break;
      
    case 1:                      //details
      showinfotext = 1;
      
      leftoffset = 0.1f;
      rightoffset = 0.95f;
      topoffset = .83f;
      bottomoffset = .27f;
      break;
  }
  
}

int PickWindow::OpenMenu(QPoint point)
{
  QMenu menu(this);
  menu.addAction("Axes Color")->setData(axes_color);
  menu.addAction("Graph Color")->setData(graph_color);
  menu.addAction("Graph Width")->setData(graph_width_menu);
  QAction* fullscreenAction = menu.addAction("Toggle Display Mode"); fullscreenAction->setData(full_screen);
  fullscreenAction->setCheckable(true); fullscreenAction->setChecked(showinfotext == 0);
  QAction* subseriesModeAction = menu.addAction("Toggle Subseries Mode"); subseriesModeAction->setData(toggle_subseries_mode);
  subseriesModeAction->setCheckable(true); subseriesModeAction->setChecked(map3d_info.subseries_mode);

  QAction* action = menu.exec(point);
  if (action)
    return action->data().toInt();
  else
    return -1;

}

void PickWindow::MenuEvent(int menu_data)
{
  switch (menu_data) {
    case axes_color:
      PickColor(axiscolor);
      break;
    case graph_color:
      PickColor(graphcolor);
      break;
    case graph_width_menu:
      PickSize(&graph_width, 10, "Graph Width");
      break;
    case toggle_subseries_mode:
      // Force redraw of all pick windows
      map3d_info.subseries_mode = !map3d_info.subseries_mode;
      Broadcast(MAP3D_UPDATE);
      break;
    case full_screen:
      SetStyle(showinfotext == 1 ? 0 : 1); // pass the opposite of what it currently is
      break;
  }
  update();
}

void PickWindow::closeEvent(QCloseEvent *event)
{
  Destroy();
}
