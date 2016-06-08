/* Map3d_Geom.h */

#ifndef MAP3D_GEOM_H
#define MAP3D_GEOM_H

#define GEOM_ERROR -2
#define GEOM_WARNING -1
#define GEOM_OK 0

// in channel files, if a node has a channel of -1, use this value 
#define UNUSED_DATA -9.99e30f

#include <vector>

struct Land_Mark;
class Surf_Data;

class Map3d_Geom   /*** Geometry for a single surface. ***/
{
public:
  Map3d_Geom();
  ~Map3d_Geom();
  void destroy();
  void init();
  //static Map3d_Geom *AddAMap3dGeom(Map3d_Geom * map3dgeom, long numpts);
  long SetupMap3dSurfElements(long numelts, long eltsize);
  void SetTimestep(int data_timestep);
  void UpdateTimestep(Surf_Data* data); // based on what frame the data is on
  int CheckPointValidity();
  int CheckElementPoints();
  int CheckElementDoubles();
  int CheckElementValidity();

  long surfnum;        /*** Surface number for this surface ***/
  long subsurf;        /*** Number of surface within 1 geometry file ***/
  long numpts;         /*** Number of points in the surface geometry. ***/
  long maxnumelts; /*** Maximum number of elements in the surface. ***/
  long numelements;    /*** Number of elements in the surface. ***/
  long elementsize;    /*** number points in an element (2=seg, 3=tri, 4=tet) **/
  long numleadlinks;   /*** Number of lead links in this surface ***/
  float cubescale; /*** Scaling variable used to draw sphers and cubes ***/

  std::vector<float **> points;    /*** Point array for whole geometry, possibly time-dependent ***/
  int geom_index;
  long **elements;     /*** Segment array for whole geometry ***/
  long *channels;      /*** Channels for this surface ***/
  long *leadlinks;     /*** Leadlinks for this surface ***/
  char **leadlinklabels;  /*** Leadlink strings for this surface ***/

  long* seggroups;
  long* tetragroups;

  char label[100]; /*** A string with the label for this surface ***/
  char filepath[512]; /*** Path name for the geom files ***/
  char basefilename[512]; /*** Base filename for this geometry ***/
  Land_Mark *landmarks; /*** Landmarks for this surface. ***/

  float xmin, xmax, ymin, ymax, zmin, zmax;
  float xcenter, ycenter, zcenter;
  float l2norm;
  float color[3];
  float **ptnormals; /*** the normals at the points ***/
  float **fcnormals; /*** the normals at the faces ***/
  long **adjacent_triangles; /*** the normals at the faces ***/

  bool modified;


};

#endif
