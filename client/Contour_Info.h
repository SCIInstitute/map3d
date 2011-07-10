/* Contour_Info.h */

#ifndef CONTOURINFO_H
#define CONTOURINFO_H


class Bandpoly;
class Map3d_Geom;
class Mesh_Info;
class Surf_Data;

#include <QColor>

#define ABSMAXCONTS  200000
#define CONTBLOCK    1000
#define BANDBLOCK 5000
#define MAX_CONTOURS 80
class Contour_Info  /*** Contour segments -- one for each surface ***/
{
public:
  Contour_Info(Mesh_Info * m);
  ~Contour_Info();
  int buildContours();
  long GrowConts(long contblocksize);
  void AddBandPoly(float **nodes, long numnodes, float potval, long isonum, float **normals);
  long AddContourSeg(long *numsegs, float node1[3], float node2[3], long levelnum, long trinum);
  long GenSurfContourBand(float min, float max, Map3d_Geom * onemap3dgeom, Surf_Data * onesurfdata);
  long GenFidContourBand(float min, float max, Map3d_Geom * onemap3dgeom, Surf_Data * onesurfdata);
  long GenFidMapContourBand(float min, float max, Map3d_Geom * onemap3dgeom, Surf_Data * onesurfdata);
  long surfnum;     /*** Surface number to which these isosurfs belong. ***/
  long datatype;    /*** Set=-2 for pots, otherwise fidtype. ***/
  long numlevels;      /*** Number of isolevels. ***/
  long numisosegs;     /*** Number of contour segments in the surface ***/
  long maxnumisosegs; /*** Current size of the contpt/contcol buffers ***/
  long maxnumlevels; /*** Current size of the isolevels buffer ***/
  double potthreshold;  /*** Threshold for potential differences ***/
  double distthreshold; /*** Threshold for distance differences ***/
  float *isolevels; /*** The contour level values. ***/
  long *numtrisegs;  /*** Number of segments in each triangle. ***/
  int numtris; /*** How many tris we currently have ****/
  int maxnumtris; /*** How many tris we have allocated ***/

  float **contpt1; /*** First point of the contour segments. ***/
  float **contpt2; /*** Second point of the contour segments. ***/
  long *trinum;  /*** Local (surface based) triangle # for each segment. ***/
  long *contcol;    /*** Isolevel number for each contour segment. ***/
  float occlusion_gradient;
  
  //fiducials
  QColor fidcolor;//fiducial contour color
  float fidContSize;
  int fidmap;    /*** Set=1 for fidmap. ***/
  int numfidconts;
  float userfidmin, userfidmax;
  float fidcontourspacing;
  bool user_fid_range;
  bool use_spacing_fid;

  //Bandshade 
  long numbandpolys; /*** Number of polys with allocated nodes in bandpolys ***/
  long maxnumbandpolys; /*** Current size of badnpolys ***/
  Bandpoly *bandpolys; /*** Array of band shade polygons ***/
  Mesh_Info *mesh; /*** pointer to the contour's mesh ***/
};

class TriContBandStruct
{
public:
  TriContBandStruct();
  ~TriContBandStruct();
  void swapCurrPrev();
  void propergateCurrToPrev();
  void checkContourLevel(long trinum, long i1, long i2, float pot[3], long nodes[3],
                         Map3d_Geom * onemap3dgeom, long *npts, double potdiffthreshold);
  void prepareBandTriangle(float pot[3], float **trinodes,
                           long nodes[3], float **trinorms, Map3d_Geom * onemap3dgeom, long ltrinum);
  inline void addOneBand(float **trinodes, long numnodes, float **trinorms,
                  Map3d_Geom * onemap3dgeom, Contour_Info * onecontourinfo);
  void checkPossibleBands(float pot[3], float **trinodes,
                          long nodes[3], float **trinorms, Map3d_Geom * onemap3dgeom, Contour_Info * onecontourinfo);
  void checkLastBand(float pot[3], float **trinodes,
                     long nodes[3], float **trinorms, Map3d_Geom * onemap3dgeom, Contour_Info * onecontourinfo);

  float currlevel;              /* Current contour level value */
  long currlevelnum;            /*  and its index */
  float prevlevel;              /* Previous contour level value */
  float prevlevelnum;           /*  and its index */
  long ltrinum;                 /* Triangle index */
  long ifsetprev;               /* false until moving to 2nd contour level */

  float **prevpts;              /* Intercepting points to previous contour line */
  float **prevnormals;          /* associated normals */
  long *previdx1;               /* Index to nodes of end points of a single */
  long *previdx2;               /*  intercepting point, 1 w/ low pot, 2 w/ high pot */
  long prevnum;                 /* How many intercepting points for previous contour */

  float **currpts;              /* similar with "prev", but refer to current contour */
  float **currnormals;
  long *curridx1;
  long *curridx2;
  long currnum;

};


#endif
