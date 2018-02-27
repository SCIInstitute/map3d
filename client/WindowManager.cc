/* WindowManager.cxx */

#ifdef _WIN32
#  include <windows.h>
#  pragma warning(disable:4505)
#  undef TRACE
#endif

#include "WindowManager.h"
#include "Contour_Info.h"
#include "GenericWindow.h"
#include "GeomWindow.h"
#include "Map3d_Geom.h"
#include "MainWindow.h"
#include "GeomWindowMenu.h"
#include "PickWindow.h"
#include "pickinfo.h"
#include "LegendWindow.h"
#include "map3d-struct.h"       /* for Surf_Data */
#include <limits.h>
#include <float.h>              /*for FLT_MAX */
#include "MeshList.h"
#include "Surf_Data.h"
#include "eventdata.h"
#include "ContourDialog.h"

#include <string>
#include <vector>

#include <QMouseEvent>

using std::string;
using std::vector;

// globals governing multi-surf frame advancement
int fstart, fend, cur, fstep;
bool allSurfsAtBeginning;
bool allSurfsAtEnd;


int current_winid = -1;
extern Map3d_Info map3d_info;
extern vector<Surface_Group> surf_group;
extern MainWindow *masterWindow;

vector < Map3dGLWidget * >AllWindows;
vector < GeomWindow * >GeomWindows;

GeomWindow *GetGeomWindow(int index)
{
  if (index < 0 || index >= GeomWindows.size())
    return NULL;
	return GeomWindows[index];
}

unsigned numGeomWindows()
{
	return GeomWindows.size();
}

Map3dGLWidget *GetWindow(int index)
{
  if (index < 0 || index >= AllWindows.size())
    return NULL;
	return AllWindows[index];
}

unsigned numWindows()
{
	return AllWindows.size();
}

Mesh_Info* getFirstMesh()
{
  MeshIterator mi(0,0);

  // get first mesh of group if group locking is on
  if (map3d_info.selected_group != -1)
    while (mi.getMesh()->groupid != map3d_info.selected_group)
      ++mi;
  return mi.getMesh();
}

void SetFirstWindow()
{
  int length = GeomWindows.size();
  int loop;
  
  for (loop = 0; loop < length; loop++) {
    GeomWindow *p = GeomWindows[loop];
    if (p) {
      return;
    }
  }
  
  exit(1);
}

void DestroyWindow(LegendWindow * p)
{
  AllWindows[p->winid] = 0;
  // FIX gtk_widget_hide(p->window);
  delete p;
}

void DestroyWindow(PickWindow * p)
{
  AllWindows[p->winid] = 0;
/* - since we currently have all of our windows up at start time,
     don't actually destroy the window, we might reuse it later. */
  if (masterWindow){
    // FIX gtk_widget_hide(p->drawarea);
  }
  else{
    // FIX gtk_widget_hide(p->window);
  }
  //p->setVisible(false);
  p->deleteLater();
}

void DestroyWindow(GeomWindow * p)
{
    int length = GeomWindows.size();
    int loop;
    AllWindows[p->winid] = 0;
    for (loop = 0; loop < length; loop++) {
      GeomWindow *priv = GeomWindows[loop];
      if (priv && priv->winid == p->winid) {
	GeomWindows[loop] = 0;
	break;
      }
    }
    
    length = p->meshes.size();
    for (loop = 0; loop < length; loop++) {
      DestroyWindow(p->meshes[loop]->legendwin);
    }
    
    delete p;
}

int HandleKeyboard(key_data * data)
{
  int handled = 0;
  switch (data->key) {
    case 27:
      handled = 1;
      exit(0);
  }
  
  return handled;
}

void AssociateGeomWindow(GeomWindow * priv)
{
  priv->geomWinId = GeomWindows.size();
	GeomWindows.push_back(priv);
}

int AssociateWindow(Map3dGLWidget * priv)
{
	static int count = 0;
	
	if (count == 0) {
		AllWindows.push_back(NULL);
		count++;
	}
	// create window storage location for broadcasts
	AllWindows.push_back(priv);
	return count++;
}

void Broadcast(int message, int data)
{
	Broadcast(message, NULL, NULL, data);
}

void Broadcast(int message, Map3dGLWidget* widget, QEvent * event, int data)
{
  int loop, alength = AllWindows.size();
  int glength = GeomWindows.size();
  
  switch (message) {
    case MAP3D_REPAINT_ALL:
      //set masterWindow's bg color - it is the only physical color controlled
      //  by gtk rather than gl.
      if (masterWindow) {
	// FIX GdkColor color;
	// FIX color.red = (short unsigned) (masterWindow->bgcolor[0]*65535.f);
	// FIX color.green = (short unsigned) (masterWindow->bgcolor[1]*65535.f);
	// FIX color.blue = (short unsigned) (masterWindow->bgcolor[2]*65535.f);
	// FIX gtk_widget_modify_bg(masterWindow->window, GTK_STATE_NORMAL, &color);
	// FIX gtk_widget_queue_draw(masterWindow->window);
      }
      
      for (loop = 1; loop < alength; loop++) {
	if (AllWindows[loop] != 0) {
	  // FIX if (AllWindows[loop]->wintype == LEGENDWINDOW)
	  // FIX  LegendWindowReshape(AllWindows[loop]->drawarea, NULL, AllWindows[loop]);
	  AllWindows[loop]->update();
	}
      }
      break;
      
    case MAP3D_MOUSE_MOTION:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  float xn, yn;
	  GeomWindow *priv = GeomWindows[loop];
	  QMouseEvent *m_event = (QMouseEvent *) event;
	  xn = (float)m_event->x() / widget->width();
	  yn = (float)m_event->y() / widget->height();
	  priv->HandleMouseMotion(m_event, xn, yn);
	}
      }
      break;
      
    case MAP3D_MOUSE_BUTTON_PRESS:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  float xn, yn;
	  GeomWindow *priv = GeomWindows[loop];
	  QMouseEvent *m_event = (QMouseEvent *) event;
	  xn = (float)m_event->x() / widget->width();
	  yn = (float)m_event->y() / widget->height();
	  priv->HandleButtonPress(m_event, xn, yn);
	}
      }
      break;
    case MAP3D_MOUSE_BUTTON_RELEASE:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  float xn, yn;
	  GeomWindow *priv = GeomWindows[loop];
	  QMouseEvent *m_event = (QMouseEvent *) event;
	  xn = (float)m_event->x() / widget->width();
	  yn = (float)m_event->y() / widget->height();
	  priv->HandleButtonRelease(m_event, xn, yn);
	}
      }
      break;
    case MAP3D_KEY_PRESS:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0)
	  GeomWindows[loop]->HandleKeyPress((QKeyEvent*) event);
      }
      break;
    case MAP3D_KEY_RELEASE:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0)
	  GeomWindows[loop]->HandleKeyRelease((QKeyEvent*) event);
      }
      break;
    case MAP3D_MENU:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  GeomWindows[loop]->HandleMenu(data);
	}
      }
      
      break;
    case MAP3D_PICK_FRAMES:
      {
      // data has the delta_frames stored in it.
     int frames = data;
      if (cur + frames > fend || cur + frames < fstart)
	break;
      cur += frames;
      for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
	      Mesh_Info* mesh = mi.getMesh();
	      if (mesh->data && (map3d_info.selected_group == -1 || map3d_info.selected_group == mesh->groupid))
	        mesh->data->FrameAdvance(frames);
        // FIX updateContourDialogValues(mesh);
        
      }
    }
    break;
	    
    // called by GeomWindowKeyboard - forward in time (See also MAP3D_PICK_FRAMES)
    // this is also the only one that should allow looping, as it should only be allowed with keyboard
    case MAP3D_FRAMES:
    {
      if (!map3d_info.frame_loop) // because, in this case, we ignore it altogether
        ComputeLockFrameData();
      int key = data;
      if (key != Qt::Key_Left && key != Qt::Key_Right)
        break;
      int multiplier = (key == Qt::Key_Left) ? -1 : 1; // multiply by -1 if left
      if (!map3d_info.frame_loop && (cur + fstep*multiplier < fstart || cur + fstep*multiplier > fend))
        break;
      if (!map3d_info.frame_loop)
        cur += fstep*multiplier;
      
      for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
        Mesh_Info* mesh = mi.getMesh();
        if (mesh->data && (map3d_info.selected_group == -1 || map3d_info.selected_group == mesh->groupid))
          mesh->data->FrameAdvance(multiplier*fstep, map3d_info.frame_loop);
          // FIX updateContourDialogValues(mesh);
      }
    }
    Broadcast(MAP3D_UPDATE);
    break;
    case MAP3D_UPDATE:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
		GeomWindows[loop]->UpdateAndRedraw();
	}
      }
      if (masterWindow)
        masterWindow->updateLabel();
      break;
  }
}

void FrameMinMax()
{
  float min = FLT_MAX;
  float max = -FLT_MAX;
  for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
    Surf_Data* cursurf = mi2.getMesh()->data;
    if (cursurf) {
      max = MAX(cursurf->minmaxframes[cursurf->framenum].potmax, max);
      min = MIN(cursurf->minmaxframes[cursurf->framenum].potmin, min);
    }
  }
  map3d_info.scale_frame_max = max;
  map3d_info.scale_frame_min = min;
  
}

void GlobalMinMax()
{
  float min = FLT_MAX;
  float max = -FLT_MAX;
  for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
    Surf_Data* cursurf = mi2.getMesh()->data;
    if (cursurf) {
      max = MAX(cursurf->potmax, max);
      min = MIN(cursurf->potmin, min);
    }
  }
  map3d_info.global_potmax = max;
  map3d_info.global_potmin = min;
}

void ComputeLockFrameData()
{
  Surf_Data *cursurf = 0;
  int left, right;
  left = INT_MAX;
  right = INT_MAX;
  fstart = fend = cur = 0;
  allSurfsAtEnd = true;
  allSurfsAtBeginning = true;
  
  for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
    Mesh_Info* mesh = mi2.getMesh();
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;
    cursurf = mesh->data;
    if (cursurf) {
      left = MIN(left, cursurf->framenum);  // - cursurf->framenum % cursurf->ts_step);//-cursurf->ts_start);
										//changed the -cursurf->ts_start from left to right
      right = MIN(right, cursurf->numframes - cursurf->framenum - 1);

	  if (cursurf->framenum + fstep < cursurf->numframes)
		  allSurfsAtEnd = false;
	  if (cursurf->framenum - fstep >= 0)
		  allSurfsAtBeginning = false;
	}
  }
	
  cur = left;
  fend = left + right;
  //printf("ComputeLockFrameData: start = %d, end = %d, cur = %d\n",fstart,fend,cur);
}

void updateGroup(Mesh_Info * mesh, int gid)
{
  if (mesh->groupid != gid && mesh->gpriv->dominantsurf != -1) {
    int oldid = mesh->groupid;
    mesh->groupid = gid;

    if (mesh->data) {
      if (surf_group[gid].potmin > mesh->data->potmin)
        surf_group[gid].potmin = mesh->data->potmin;
      if (surf_group[gid].potmax < mesh->data->potmax)
        surf_group[gid].potmax = mesh->data->potmax;
      if (surf_group[gid].framemin > mesh->data->minmaxframes[mesh->data->framenum].potmin)
        surf_group[gid].framemin = mesh->data->minmaxframes[mesh->data->framenum].potmin;
      if (surf_group[gid].framemax < mesh->data->minmaxframes[mesh->data->framenum].potmax)
        surf_group[gid].framemax = mesh->data->minmaxframes[mesh->data->framenum].potmax;
    }
    recalcGroup(oldid);
    Broadcast(MAP3D_UPDATE);
  }
}

void recalcGroup(int gid)
{
  Init_Surface_Group(&surf_group[gid]);
  for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
    Mesh_Info* mesh = mi2.getMesh();
    if (mesh->data) {
      if (mesh->data->minmaxframes != 0 && mesh->groupid == gid) {
	if (surf_group[gid].potmin > mesh->data->potmin)
	  surf_group[gid].potmin = mesh->data->potmin;
	if (surf_group[gid].potmax < mesh->data->potmax)
	  surf_group[gid].potmax = mesh->data->potmax;
	if (surf_group[gid].framemin > mesh->data->minmaxframes[mesh->data->framenum].potmin)
	  surf_group[gid].framemin = mesh->data->minmaxframes[mesh->data->framenum].potmin;
	if (surf_group[gid].framemax < mesh->data->minmaxframes[mesh->data->framenum].potmax)
	  surf_group[gid].framemax = mesh->data->minmaxframes[mesh->data->framenum].potmax;
      }
    }
  }
}

void assignMasters(Global_Input *)
{
  for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
    Mesh_Info* orig_mesh = mi.getMesh();
    for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
      Mesh_Info* comp_mesh = mi2.getMesh();
      if (orig_mesh->mysurf->scale_lock == comp_mesh->geom->surfnum && orig_mesh->data && comp_mesh->data) {
	orig_mesh->data->mastersurf = comp_mesh->data;
        break;
      }
      else if (orig_mesh->data)
	orig_mesh->data->mastersurf = 0;
    }
  }
}

#if 0 
// FIX
void IconifyAll(GtkWidget *widget){
  for(unsigned i = 1; i < AllWindows.size(); i++){
    if(widget != AllWindows[i]->window){
      // FIX gtk_window_iconify(GTK_WINDOW(AllWindows[i]->window));
    }
  }
}

void DeIconifyAll(GtkWidget *widget){
  for(unsigned i = 1; i < AllWindows.size(); i++){
    if(AllWindows[i] && widget != AllWindows[i]->window){
      // FIX gtk_window_deiconify(GTK_WINDOW(AllWindows[i]->window));
    }
  }
}
/*
 * Window state tracking
 */

void window_state_callback (GtkWidget *widget,
							GdkEventWindowState *event,
							void*)
{
  /* FIX
  if(COMPARE_MODIFIERS(event->new_window_state, (GDK_WINDOW_STATE_ICONIFIED))){
    IconifyAll(widget);
  }
  if(!COMPARE_MODIFIERS(event->new_window_state, (GDK_WINDOW_STATE_ICONIFIED))){
    DeIconifyAll(widget);
  }
  */
}
#endif

MeshIterator::MeshIterator(int geom /*=0*/, int mesh /*=0*/)  : geomwinnum(geom), meshnum(mesh) 
{
  done = true;
  for (unsigned i = 0; i < numGeomWindows(); i++) 
  {
    if (GeomWindows[i]->meshes.size() > 0)
    {
      done = false;
      break;
    }
    else if (i == 0 && geomwinnum == 0)
	  {
      geomwinnum++; // don't crash if there isn't a mesh in the first window
    }
  }
}
MeshIterator& MeshIterator::operator++()
{
  if (++meshnum >= GeomWindows[geomwinnum]->meshes.size()) {
    geomwinnum++;
    meshnum = 0;
    while (geomwinnum < numGeomWindows() && (!GeomWindows[geomwinnum] || GeomWindows[geomwinnum]->meshes.size() == 0)) {
      geomwinnum++;
    }
    if (geomwinnum >= numGeomWindows())
      done = true;
  }
  return *this;
}

Mesh_Info* MeshIterator::getMesh() 
{
  if (done)
    return NULL;
  else 
    return GeomWindows[geomwinnum]->meshes[meshnum];
}


