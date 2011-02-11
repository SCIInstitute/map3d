/* readfile.h */

#ifndef MAP3D_READ_FILES
#define MAP3D_READ_FILES
class Surf_Data;
class Mesh_Info;
class Map3d_Geom;

#include "map3d-struct.h"
#include <vector>

using std::vector;

#define PTS_ONLY   0 /*** Geometry type: .pts file only  ***/
#define PTS_SEG    1 /*** Geometry type: .pts + .seg files ***/
#define PTS_FAC    2 /*** Geometry type: .pts + .fac files ***/
#define PTS_TETRA  3 /*** Geometry type: .pts + .tetra files ***/
#define GEOM       4 /*** Geometry type: .geom file  - DEPRECATED ***/
#define DATA_FILE  5 /*** Geometry/Data file type: new .data file - DEPRECATED ***/
#define LMARK_FILE 6 /*** Landmark file ***/
#define LMARKS_FILE 7 /*** Landmark file ***/
#define POT_ONLY   8 /*** Data file type: old .pot file ***/
#define GRAD_ONLY  9 /*** Data file type: old .grad file ***/
#define POT_GRAD   10 /*** Data file type: .pot + .grad files ***/
#define TSDF_FILE  11 /*** New, single time-series data file - DEPRECATED ***/
#define CHANNELS   12 /*** channels file type ***/
#define TSDFC_FILE 13           // TSDF container file - DEPRECATED
#define LEADLINKS  14 /*** leadlinks file type ***/
#define MATLAB_FILE 15 /*** matlab file for geom or data ****/
#define LEADLABELSIZE 10 /*** Max size of the label we put on leads ***/

#include "MatlabIO.h"
class Container;

#include <map>
#include <string>

// In order to get info from a file, we don't want to open the file a million times
// (particularly container files), so store the post-opened file here
struct FileCache
{
  std::map<std::string, void*> cache;
  matlabarray* readMatlabFile(std::string str);
};


bool getFileWithPath(char* filename, char* path1, char* path2, char* out);
long ReadPts(Map3d_Geom* onemap3dgeom);
long ReadSegs(FILE * luin_p, Map3d_Geom* onemap3dgeom);
long ReadTris(Map3d_Geom* onemap3dgeom);
long ReadTetras(FILE * luin_p, Map3d_Geom* onemap3dgeom);
long ReadMatlabGeomFile(Map3d_Geom* onemap3dgeom, long insurfnum);
void WriteMatlabGeomFile(char *filename, vector<Map3d_Geom *> onemap3dgeom);
long ReadLandmarkGeomFile(Map3d_Geom* onemap3dgeom);

Surf_Data *ReadPotFiles(Surf_Input * sinputlist, Surf_Data * surfdata,
                        Map3d_Geom * map3dgeom, long numsurfsread, long insurfnum);
Surf_Data *ReadMatlabDataFile(Surf_Input * sinputlist, Surf_Data * surfdata,
			      long numsurfsread, long insurfnum);


long ReadMap3dFidfile(Surf_Input * sinputlist, Surf_Data * surfdata, long numsurfsread, long insurfnum);
long CopyFidTypes(Series_Fids * outfidseries, Series_Fids * infidseries);
long CopyFidNames(Series_Fids * outfidseries, Series_Fids * infidseries);
void ReadOnePotFile(FILE * luin_p, float potscale, Surf_Data * onesurfdata, long lframenum, long *onechannels);

Series_Fids *ReadTsdfcFile(char* filename, char* potfilename, long timeseriesnum,
			   Lead_Fids *global, Series_Fids *series, int *numfs,
			   float *fidmin, float *fidmax);
void ReadChannelsFile(FILE * luin_p, Map3d_Geom * onemap3dgeom);
long ReadChannelLinksFile(FILE * luin_p, Map3d_Geom * onemap3dgeom);
long ChanneltoNode(long chlink, long *channels, long numpts);
long NodetoLeadlink(long nodenum, Map3d_Geom * onemap3dgeom, char *leadlabel);
long ReadLeadFile(FILE * luin_p, Map3d_Geom * onemap3dgeom);
#endif
