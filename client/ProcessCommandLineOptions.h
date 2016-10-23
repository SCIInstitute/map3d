/* ProcessCommandLineOptions.h */

#ifndef PROCESSCOMMANDLINEOPTIONS_H
#define PROCESSCOMMANDLINEOPTIONS_H

struct Global_Input;
struct Surf_Input;
class Map3d_Geom;
class Mesh_Info;
class GenericWindow;

#include "MeshList.h"
#include <vector>

namespace MatlabIO { class matlabarray; }

typedef std::vector < Mesh_Info * >Mesh_List;

#define RELOAD_NONE 0
#define RELOAD_GEOM 1
#define RELOAD_DATA 2
#define RELOAD_BOTH 3
#define LOAD_RMS_DATA 4

void PrintCommandLineOptions(Global_Input & g);
int ProcessCommandLineOptions(Global_Input & g);

// we pass in a mesh, as the "normal" case is to pass in one mesh
// and have the geom updated.  However, in the case where we want to
// read in all the surfaces, we create them here too, and return them
// in the mesh list.
Mesh_List FindAndReadGeom(Surf_Input *, Mesh_List currentMeshes, int reload);
void FindAndReadChannels(Surf_Input *, Map3d_Geom *);
void FindAndReadLeadlinks(Surf_Input *, Map3d_Geom *);
void FindAndReadData(Surf_Input *, Mesh_Info *, int reload);
int DetectFileType(char *);
const char *GetExtension(const char *s);
char *GetBase(char *s);
unsigned GetNumGeoms(char *s);

// try to minimize calls to matlabarray stuff.  Caller is responsible for freeing matlabarray
void GetDataFileInfo(std::string filename, int& numTimeSeries, std::vector<int>& numFramesPerSeries, std::vector<std::string>& timeSeriesLabels, MatlabIO::matlabarray*& outArray);

void ComputeTriNormals(Map3d_Geom * m);
void ComputeTetNormals(Map3d_Geom * m);
void ComputeGeomStatistics(Map3d_Geom * m);
void CopySurfToMesh(Surf_Input * s, Surf_Input* defaultSurf, Mesh_Info * m);
void ExtractSeriesNumbers(Global_Input & g);


#endif
