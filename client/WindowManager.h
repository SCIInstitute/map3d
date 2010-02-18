/* WindowManager.h */

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

class Map3dGLWidget;
class MainWindow;
class GeomWindow;
class PickWindow;
class LegendWindow;

class QEvent;

struct key_data;
class Mesh_Info;
struct Global_Input;

GeomWindow *GetGeomWindow(int index);
unsigned numGeomWindows();
Map3dGLWidget *GetWindow(int index);
unsigned numWindows();
Mesh_Info* getFirstMesh();
void AssociateGeomWindow(GeomWindow * priv);
int AssociateWindow(Map3dGLWidget * priv);

#ifndef MIN
#  define MIN(x,y) ((x<y)?x:y)
#endif
#ifndef MAX
#  define MAX(x,y) ((x>y)?x:y)
#endif

void Broadcast(int message, int data = -1);
// broadcast has group id for selective broadcasts (-1 for all groups), 
// and a pointer to the window it was called in (except on repaint or update)
void Broadcast(int message, Map3dGLWidget * widget, QEvent * event, int data=-1);

void SetFirstWindow();
void DestroyWindow(GeomWindow *);
void DestroyWindow(PickWindow *);
void DestroyWindow(LegendWindow *);

void ComputeLockFrameData();
void FrameMinMax();
void GlobalMinMax();

void updateGroup(Mesh_Info * mesh, int gid);
void recalcGroup(int gid);
void assignMasters(Global_Input * g);

// FIX void IconifyAll(GtkWidget *widget);
// FIX void DeIconifyAll(GtkWidget *widget);

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
