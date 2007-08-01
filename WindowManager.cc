/* WindowManager.cxx */

#ifdef _WIN32
#  include <windows.h>
#  pragma warning(disable:4505)
#  undef TRACE
#endif
#include <gtk/gtk.h>

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
#include <gdk/gdkkeysyms.h>

#include <string>
#include <vector>
using std::string;
using std::vector;

#ifndef MIN
#  define MIN(x,y) ((x<y)?x:y)
#endif
#ifndef MAX
#  define MAX(x,y) ((x>y)?x:y)
#endif

int fstart, fend, cur, fstep;
int current_winid = -1;
extern Map3d_Info map3d_info;
extern vector<Surface_Group> surf_group;
extern MainWindow *masterWindow;

vector < GLWindow * >AllWindows;
vector < GeomWindow * >GeomWindows;

GeomWindow *GetGeomWindow(int index)
{
	return GeomWindows[index];
}

unsigned numGeomWindows()
{
	return GeomWindows.size();
}

GLWindow *GetWindow(int index)
{
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

void SetCurrentWindow(int id)
{
  current_winid = id;
}
int GetCurrentWindow()
{
  return current_winid;
}

void DestroyWindow(LegendWindow * p)
{
  AllWindows[p->winid] = 0;
  gtk_widget_hide(p->window);
  delete p;
}

void DestroyWindow(PickWindow * p)
{
  AllWindows[p->winid] = 0;
/* - since we currently have all of our windows up at start time,
     don't actually destroy the window, we might reuse it later. */
  if (masterWindow){
    //gtk_widget_destroy(p->drawarea);
    gtk_widget_hide(p->drawarea);
  }
  else{
    //gtk_widget_destroy(p->window);
    gtk_widget_hide(p->window);
  }
  //delete p;
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
	GeomWindows.push_back(priv);
}

int AssociateWindow(GLWindow * priv)
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

GenericWindow *FindWidgetOwner(GtkWidget * widget)
{
	unsigned i;
	
	for (i = 1; i < AllWindows.size(); i++)
		if (AllWindows[i] && AllWindows[i]->drawarea == widget)
			return AllWindows[i];
	return NULL;
}

void Broadcast(int message, void *data)
{
	Broadcast(message, NULL, NULL, data);
}

void Broadcast(int message, GtkWidget *, GdkEvent * event, gpointer data)
{
  int loop, alength = AllWindows.size();
  int glength = GeomWindows.size();
  
  switch (message) {
    case MAP3D_REPAINT_ALL:
      //set masterWindow's bg color - it is the only physical color controlled
      //  by gtk rather than gl.
      if (masterWindow) {
	GdkColor color;
	color.red = (short unsigned) (masterWindow->bgcolor[0]*65535.f);
	color.green = (short unsigned) (masterWindow->bgcolor[1]*65535.f);
	color.blue = (short unsigned) (masterWindow->bgcolor[2]*65535.f);
	gtk_widget_modify_bg(masterWindow->window, GTK_STATE_NORMAL, &color);
	//gtk_widget_hide(masterWindow->window);
	//gtk_widget_show(masterWindow->window);
	gtk_widget_queue_draw(masterWindow->window);
      }
      
      for (loop = 1; loop < alength; loop++) {
	if (AllWindows[loop] != 0) {
	  if (AllWindows[loop]->wintype == LEGENDWINDOW)
	    LegendWindowReshape(AllWindows[loop]->drawarea, NULL, AllWindows[loop]);
	  gtk_widget_queue_draw(AllWindows[loop]->drawarea);
	}
      }
      break;
      
    case MAP3D_MOUSE_MOTION:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  float xn, yn;
	  GeomWindow *priv = (GeomWindow *) data;
	  GdkEventButton *bevent = (GdkEventButton *) event;
	  xn = (float)bevent->x / priv->width;
	  yn = (float)bevent->y / priv->height;
	  GeomWindowHandleMouseMotion(GeomWindows[loop]->drawarea, (GdkEventMotion *) event, (gpointer) GeomWindows[loop],
				      xn, yn);
	}
      }
      break;
      
    case MAP3D_MOUSE_BUTTON_PRESS:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  float xn, yn;
	  GeomWindow *priv = (GeomWindow *) data;
	  GdkEventButton *bevent = (GdkEventButton *) event;
	  xn = (float)bevent->x / priv->width;
	  yn = (float)bevent->y / priv->height;
	  //        printf("GeomWindows[loop] = 0x%X\n", GeomWindows[loop]);
	  GeomWindowBPress(GeomWindows[loop]->drawarea, (GdkEventButton *) event, (gpointer) GeomWindows[loop], xn, yn);
	  //        GeomWindowBPress(AllWindows[loop]->drawarea, (GdkEventButton *)event, data);
	  //        GeomWindowHandleMouseButton((mouse_button_data *) data);
	}
      }
      break;
    case MAP3D_MOUSE_BUTTON_RELEASE:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  float xn, yn;
	  GeomWindow *priv = (GeomWindow *) data;
	  GdkEventButton *bevent = (GdkEventButton *) event;
	  xn = (float)bevent->x / priv->width;
	  yn = (float)bevent->y / priv->height;
	  //        printf("widget = 0x%X; event = 0x%X; priv = 0x%X\n", GeomWindows[loop]->drawarea, event, GeomWindows[loop]);
	  GeomWindowBRelease(GeomWindows[loop]->drawarea, (GdkEventButton *) event, (gpointer) GeomWindows[loop], xn, yn);
	  //        GeomWindowBRelease(AllWindows[loop]->drawarea, (GdkEventButton *)event, data);
	  //        GeomWindowHandleMouseButton((mouse_button_data *) data);
	}
      }
      break;
    case MAP3D_KEY_PRESS:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0)
	  GeomWindowKPress(GeomWindows[loop]->drawarea, (GdkEventKey *) event, (gpointer) GeomWindows[loop]);
      }
      break;
    case MAP3D_KEY_RELEASE:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0)
	  GeomWindowKRelease(GeomWindows[loop]->drawarea, (GdkEventKey *) event, (gpointer) GeomWindows[loop]);
      }
      break;
    case MAP3D_MENU:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
	  menu_data *md = new menu_data(*(menu_data *) data);
	  md->priv = GeomWindows[loop];
	  GeomWindowHandleMenu(md);
	  delete md;
	}
      }
      
      break;
    case MAP3D_PICK_FRAMES:
    {
      // event->x has the delta_frames stored in it.
     GdkEventButton* button_event = (GdkEventButton*)event;
     int frames = (int)button_event->x;
      if (cur + frames > fend || cur + frames < fstart)
	break;
      cur += frames;
      for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
	Mesh_Info* mesh = mi.getMesh();
	if (mesh->data && (map3d_info.selected_group == -1 || map3d_info.selected_group == mesh->groupid))
	  mesh->data->FrameAdvance((int)button_event->x);
        updateContourDialogValues(mesh);
      }
    }
    break;
	    
    // called by GeomWindowKeyboard - forward in time (See also MAP3D_PICK_FRAMES)
    case MAP3D_FRAMES:
    {
      ComputeLockFrameData();
      GdkEventKey* key_event = (GdkEventKey*)event;
      if (key_event->keyval != GDK_Left && key_event->keyval != GDK_Right)
	break;
      int multiplier = (key_event->keyval == GDK_Left) ? -1 : 1; // multiply by -1 if left
      if (cur + fstep*multiplier < fstart || cur + fstep*multiplier > fend)
	break;
      cur += fstep*multiplier;
      
      for (MeshIterator mi(0,0); !mi.isDone(); ++mi) {
	Mesh_Info* mesh = mi.getMesh();
	if (mesh->data && (map3d_info.selected_group == -1 || map3d_info.selected_group == mesh->groupid))
	  mesh->data->FrameAdvance(multiplier*fstep);
        updateContourDialogValues(mesh);
      }
    }
    Broadcast(MAP3D_UPDATE,data);
    break;
    case MAP3D_UPDATE:
      for (loop = 0; loop < glength; loop++) {
	if (GeomWindows[loop] != 0) {
		GeomWindowUpdateAndRedraw(GeomWindows[loop]);
	}
      }
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
  
  for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
    Mesh_Info* mesh = mi2.getMesh();
    if (map3d_info.selected_group != -1 && mesh->groupid != map3d_info.selected_group)
      continue;
    cursurf = mesh->data;
    if (cursurf) {
      left = MIN(left, cursurf->framenum);  // - cursurf->framenum % cursurf->ts_step);//-cursurf->ts_start);
										//changed the -cursurf->ts_start from left to right
      right = MIN(right, cursurf->numframes - cursurf->framenum - 1);
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
    Broadcast(MAP3D_UPDATE, 0);
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

void IconifyAll(GtkWidget *widget){
  for(unsigned i = 1; i < AllWindows.size(); i++){
    if(widget != AllWindows[i]->window){
      gtk_window_iconify(GTK_WINDOW(AllWindows[i]->window));
    }
  }
}

void DeIconifyAll(GtkWidget *widget){
  for(unsigned i = 1; i < AllWindows.size(); i++){
    if(AllWindows[i] && widget != AllWindows[i]->window){
      gtk_window_deiconify(GTK_WINDOW(AllWindows[i]->window));
    }
  }
}


/*
 * Window state tracking
 */

void window_state_callback (GtkWidget *widget,
							GdkEventWindowState *event,
							gpointer)
{
  
  if(COMPARE_MODIFIERS(event->new_window_state, (GDK_WINDOW_STATE_ICONIFIED))){
    IconifyAll(widget);
  }
  if(!COMPARE_MODIFIERS(event->new_window_state, (GDK_WINDOW_STATE_ICONIFIED))){
    DeIconifyAll(widget);
  }
}


GtkWidget *AddCheckMenuEntry(GtkWidget * menu, char *text, int activate, GenericWindow * priv,
                             void (*func) (menu_data *))
{
  //GtkWidget* menu_items = gtk_menu_item_new_with_label(text);
  GtkWidget *menu_items = gtk_check_menu_item_new_with_label(text);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);
  g_signal_connect_swapped(G_OBJECT(menu_items), "activate",
                           G_CALLBACK(func), (gpointer) new menu_data(activate, priv));
  gtk_widget_show(menu_items);
  return menu_items;
}

GtkWidget *AddMenuEntry(GtkWidget * menu, char *text, int activate, GenericWindow * priv, void (*func) (menu_data *))
{
  string menu_text = text;
  GtkWidget *menu_item = gtk_menu_item_new_with_label(text);
  
  if (menu_text == "") {
    menu_item = gtk_separator_menu_item_new();
  }
  else {
    menu_item = gtk_menu_item_new_with_label(text);
  }
  
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(func), (gpointer) new menu_data(activate, priv));
  gtk_widget_show(menu_item);
  return menu_item;
}

GtkWidget *AddSubMenu(GtkWidget * menu, char *text)
{
  GtkWidget *submenu = gtk_menu_new();
  
  GtkWidget *submenuname = gtk_menu_item_new_with_label(text);
  
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(submenuname), submenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), submenuname);
  gtk_widget_show(submenuname);
  
  return submenu;
}


MeshIterator::MeshIterator(int geom /*=0*/, int mesh /*=0*/)  : geomwinnum(geom), meshnum(mesh) 
{
  done = numGeomWindows() == 0 || GetGeomWindow(0)->meshes.size() == 0;
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


