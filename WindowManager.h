/* WindowManager.h */

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

class GenericWindow;
class GLWindow;
class MainWindow;
class GeomWindow;
class PickWindow;
class LegendWindow;

struct key_data;
class Mesh_Info;
struct Global_Input;
struct menu_data;
typedef struct _GtkWidget GtkWidget;
typedef union _GdkEvent GdkEvent;
typedef struct _GdkEventWindowState GdkEventWindowState;
typedef void* gpointer;

GeomWindow *GetGeomWindow(int index);
unsigned numGeomWindows();
GLWindow *GetWindow(int index);
unsigned numWindows();
Mesh_Info* getFirstMesh();
void AssociateGeomWindow(GeomWindow * priv);
int AssociateWindow(GLWindow * priv);

#define COMPARE_MODIFIERS(x,y) ((x & y) == y)

void Broadcast(int message, void *data);
// broadcast has group id for selective broadcasts (-1 for all groups), 
// and a pointer to the window it was called in (except on repaint or update)
void Broadcast(int message, GtkWidget * widget, GdkEvent * event, gpointer data);
int HandleKeyboard(key_data * data);

void SetFirstWindow();
void SetCurrentWindow(int id);
int GetCurrentWindow();
void DestroyWindow(GeomWindow *);
void DestroyWindow(PickWindow *);
void DestroyWindow(LegendWindow *);

void ComputeLockFrameData();
void FrameMinMax();
void GlobalMinMax();

void updateGroup(Mesh_Info * mesh, int gid);
void recalcGroup(int gid);
void assignMasters(Global_Input * g);

void IconifyAll(GtkWidget *widget);
void DeIconifyAll(GtkWidget *widget);
void window_state_callback (GtkWidget *widget,
			    GdkEventWindowState *event,
			    gpointer data);

GtkWidget *AddCheckMenuEntry(GtkWidget * menu, char *text, int activate, GenericWindow * priv,
                             void (*func) (menu_data *));
GtkWidget *AddMenuEntry(GtkWidget * menu, char *text, int activate, GenericWindow * priv, void (*func) (menu_data *));
GtkWidget *AddSubMenu(GtkWidget * menu, char *text);
GenericWindow *FindWidgetOwner(GtkWidget * widget);

class MeshIterator {
public:
  MeshIterator(int geom=0, int mesh=0);
  MeshIterator& operator++();  // only do prefix
  Mesh_Info* getMesh();
  inline bool isDone() { return done;}
private:
  bool done;
  unsigned geomwinnum, meshnum;
};

#endif
