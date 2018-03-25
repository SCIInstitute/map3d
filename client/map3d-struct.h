/* map3d-struct.h */

#ifndef __MAP3D_STRUCTS__
#define __MAP3D_STRUCTS__
/*****************************************************************/
/*                        Some structures                        */
/*****************************************************************/
#include "Ball.h"

/* this, for some reason needs windows.h or it won't compile */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "fids.h"


#include <vector>
#include <string>

using std::vector;

#define MAX_SURFS 40
#define MAX_PICKS 20

class GeomWindow;
class LegendWindow;
class PickWindow;
class Texture;

namespace MatlabIO { class matlabarray; }

#define LOCK_FULL 2
#define LOCK_GROUP 1
#define LOCK_OFF 0

struct Scalar_Input
{
  long scalarnum; /*** Number of this scalar information ***/
  long channelnum; /*** Channel number of this scalar ***/
  int xmin, xmax, ymin, ymax;
  /*** Location of scalar in the display  ***/
};

struct Global_Input;
struct Surf_Input
{
  Global_Input *parent;
  char *geomfilename; /*** Geometry filename ***/
  vector < char *>potfilenames;/*** Potential data filename(s) - can hold
                               several files, but will be split such that
                               only one is used per surface, in potfile ***/
  vector<int> timeseriesnums;
  char* potfilename;
  char *fidfilename; /*** Potential data filename ***/
  char *chfilename; /*** Channels filename ***/
  char *clfilename; /*** Channellinks filename ***/
  char *llfilename; /*** Leadlinks filename ***/
  char *lmfilename; /*** Landmarks filename ***/
  char *tmpfilename; /*** temporary filename ***/
  long geomfiletype; /*** Geometry file type (.geom, .pts, etc.) ***/
  long datafiletype; /*** Data filetype (.data, .pot) ***/
  long geomsurfnum; /*** Surface number specified in geom file ***/
  long colour_grad; /*** Colour for drawing gradients ***/
  long numscalars; /*** Number of scalars set for this surface. ***/
  vector < Scalar_Input > scalarInputs;
  long scale_lock; /*** Value of surface to which to slave scaling   ***/
  short winxmin, winxmax, winymin, winymax;
  short lwxmin, lwxmax, lwymin, lwymax;
  bool newwindow; // whether or not this surface starts a new window
  bool lworientation;           // orientation of legend window
  /*** Location of main window in the display ***/
  long numsurfsread;  /*** Number of display surfaces read for this
  single input surface. ***/
  long displaysurfnum; /*** First display surface number for this input ***/
  float potscale; /*** Scaling factor applied to all potentials ***/
  float potusermax, potusermin; /*** User extrema for data scaling ***/
  long timeseries; /*** desired time series ***/
  long fidseriesnum; /*** The series number from the fid file; from 0. ***/
  long ts_start;/*** Frame numbers: start, end, delta ***/
  long ts_end; /*** -- start and end begin at 0!! ***/
  long ts_sample_step; /*** subsample - only open every n frames ***/
  float timestep; /*** Absolute time between frames ***/
  float timestart; /*** Absolute starting time ***/
  int groupid;
  long colour_bg[3]; /*** Colour for background ***/
  long colour_fg[3]; /*** Colour for foreground ***/
  long large_font; /* font sizes */
  long med_font;
  long small_font;

    
  // options that can be global or for each surface
  float contourstep; /*** User Step size between contours ***/
  long colour_mesh[3]; /*** Colour for drawing mesh ***/
  long colour_ll[3]; /*** Colour for drawing leadlinks ***/
  long colormap;
  long shadingmodel;
  long drawmesh;
  long invert;
  long numconts; /*** number of contours ***/
  Quat rotationQuat;
  float translation[3];
  float vfov;
  long showlegend;
  long legendticks;
  long lighting;
  long fogging;
  long drawcont;
  long negcontdashed;
  long drawfids;
  long axes;
  long axes_color[3];

  long showinfotext;
  long showlocks;

  // node marking options
  long all_sphere, all_mark, all_value;
  long extrema_sphere, extrema_mark;
  long pick_sphere, pick_mark;
  long lead_sphere, lead_mark;
  long size_ll; /*** Size of leadlinks ***/

  MatlabIO::matlabarray* preloadedDataArray;
};
typedef Surf_Input *Surf_Input_p;

struct Global_Input
{
  int numgeomfiles; /*** Current num of geom files when parsing ***/
  bool qnoborders;  /*** True if no borders on windows ***/
  bool qsinglewin;  /*** True if we put all surfaces in one window ***/
  bool qnovalidity; /*** Set true if we do not perform validity check ***/
  char *imagefile;    /*** Outout image file basefilename ***/
  char *bgimage;    /*** Image to display in the background ***/
  float bgcoords[6]; /*** geometry coordinates for bgimages (0 for fullscreen) ***/
  long report_level;    /*** Report level ***/
  bool qabsolute; /*** True for display in absolute coords ***/
  char *datapathname; /*** Path to the data files ***/
  char *geompathname; /*** Path to the geometry files ***/
  Surf_Input *SurfList[MAX_SURFS]; // have last one be a global settings list
  int borderWidth;
  int titleHeight;
  int scale_scope; /*** Scale range ***/
  int scale_model; /*** Scale function***/
  int scale_mapping; /*** Scale mapping ***/
  int lockgeneral; /*** Whether General lock is enabled ***/
  int lockrotate; /*** Whether Transformation lock is enabled ***/
  int lockframes; /*** Whether Frames lock is enabled ***/
  int pickmode; /*** Pick Mode ***/
  int framestep;  // how many frames to advance when advancing time
  bool frameloop; // whether the frames will loop

  int frames_per_time_unit; // num of (s, ms, us, ns) per frame
  char* time_unit;  // s, ms, us, ns
  bool same_scale;
};
typedef Global_Input *Global_Input_p;

class Map3d_Info
{
public:
  Map3d_Info();
  ~Map3d_Info();
  long maxnumframes; /*** Max number of frames in any surface ***/
  long reportlevel; /*** error reporting/debugging level ***/
  long framenum; /*** framenum for alignment purposes ***/
  bool qabscoord; /*** True to keep abcolute coords for geom. ***/
  bool qnoborders;  /*** True if it is a composition window ***/
  bool qnovalidity; /*** True to not perform validity checks ***/
  float picksize;
  long pickmode;
  bool picked;     /*** To only pop up one scalar instead of many ****/
  bool useonepickwindow;       /* indicates whether a pick will open a new window or use existing one */

  bool singlewin;  /*** True if all meshes are displayed in same window ***/

  float sizeincrement; /*** this repesents the change in a value if the +- keys are hit ***/
  float transincrement; /*** this, rotincrement, and scaleincrement are how much to change with +/- ***/
  float rotincrement;
  float scaleincrement;

  bool use_spacing;
  bool use_spacing_fid;

  // all these variables are used in 'proper' positioning of windows and surfaces
  bool borderInitialized;
  int borderWidth, titleHeight;
  int posx, posy;    // default placement values
  int mainwidth, mainheight;
  int screenWidth, screenHeight;

  int parentid;                 // will be set to the borderless window id if one exists
  Global_Input* gi;           // store to create additional surfaces
  
  LegendWindow* legendwins[MAX_SURFS];
  PickWindow* pickwins[MAX_PICKS];
  int numLegendwins;
  int numPickwins;

  // texture images
  Texture* dot_texture;
  Texture* lock_texture;
  Texture* rainbow_texture;
  Texture* jet_texture;
  Texture* bg_texture;

  // dynamic scaling info
  long scale_scope;
  long scale_model;
  long scale_mapping;
  float scale_frame_max;/*** info for Global_Frame ***/
  float scale_frame_min;
  char scale_frame_set;
  float global_potmax;
  float global_potmin;
  long user_fstep;
  bool frame_loop; // Initially, we will not go to extraordinary measures to keep surfs with differing frame counts in sync

  // If a group is set to LOCK_GROUP, then this is the group number to work on.
  // Should be set when an event happens, as you can do events on multiple groups.
  int selected_group; 
  int lockgeneral;  /*** Lock the general things like menu stuff ***/
  int lockframes;     /*** Lock the frame numbers of all surfaces ***/
  int lockrotate;   /*** Lock the tranformations over all surface ***/

  char* map3d_executable;  /*** To write a script with the same executable name ***/
  char imagefile[1024]; /*** Output file for images (map3d will automatically append 0000,0001, etc.) ***/
  int imagesuffix; /*** suffix num for image file - 0, 1, etc, ***/
  bool saving_animations;

  bool subseries_mode = false; // whether pick windows will only show the current subseries (beat)

  // for time display in the time series window
  int frames_per_time_unit; // num of (s, ms, us, ns) per frame
  const char* time_unit;  // s, ms, us, ns


  bool same_scale;
  
  bool contour_antialiasing;
};

/*** A single arrow--one of these for each arraw in each surface ***/
struct Arrow
{
  float p1[3], p2[3], p3[3], p4[3];
  /*** 4 points thet make the base of the arrowhead ***/
  float ptip[3]; /*** Point at the tip of the arrow ***/
  float pbase[3]; /*** Point at the base of the arrow ***/
};

class MinMax_Frame  /*** Potential extrema, one per frame. ***/
{
public:
  float potmax; /*** Max pot value for each frame ***/
  float potmin; /*** Min pot value for each frame. ***/
  long nodemax; /*** Max node number for each frame ***/
  long nodemin; /*** Min node number for each frame ***/
  long channelmax; /*** Max channels for each frame  ***/
  long channelmin; /*** Min channels for each frame  ***/
  long leadnummax;
  long leadnummin;
};

class MinMax_Fids /*** Min and max for one fidtype in surface ***/
{
public:
  long fidnum; /*** NUmber of this fid ***/
  short fidtype; /*** Type of this fid ***/
  float min; /*** Min fid values ***/
  float max; /*** Max fid val ***/
  long nodemax; /*** Max node number for each frame ***/
  long nodemin; /*** Min node number for each frame ***/
  long channelmax; /*** Max channels for each frame  ***/
  long channelmin; /*** Min channels for each frame  ***/
  long leadnummax;
  long leadnummin;
};


struct Frame_Data /*** Info for each frame of data ***/
{
  float potmax; /*** Maximum potential in this frame ***/
  float potmin; /*** Minimum potential in this frame ***/
  long nodemax;
  long nodemin;
  float gradmax;
  float gradmin;
  long gnodemax;
  long gnodemin;
};

struct Fid_Info /*** A structure for fid info of one type. ***/
{
  long fidnum; /*** Number of the fiducial. ***/
  short fidtype; /*** Fiducial type. ***/
#if 0
  char fidname[10]; /*** string for this fiducial ***/
#else
  char *fidname;
#endif
  float min, max; /*** Extrema over all surfaces of this fid type ***/
  bool qdraw; /*** Set true if we are to draw this fid type. ***/
};


class Bandpoly/*** One band polygon for band shading. ***/
{
public:
  ~Bandpoly();
  long maxpts; /*** number of points allocated ***/
  long numpts; /*** Number of points in this band polygon ***/
  long bandcol; /*** Isolevel number (not the colour!) of this polygon ***/
  float bandpotval; /*** Value for this band ***/
  float **nodes; /*** Nodes for this polygon ***/
  float **normals; /*** normals for shading ***/
};

#define MAXNUMPICKS 500

class Landmark_Draw /*** Landmark display controls ***/
{
public:
  Landmark_Draw();
  long surfnum;  /*** Surface number to which this landmark belongs ***/
  long drawtype;    /*** Type of drawing to be used for coronaries ***/
  long view;            /*** Viewing direction for coronaries. ***/
  bool qshowlmark;   /*** True if any landmarks to be shown. ***/
  bool qshowlabels; /*** True if landmark labels are to be drawn ***/
  bool qshowpoint;
  bool qshowrods;   /*** True if nay landmark in od category to be drawn. ***/
  bool qshowocclus; /*** True if any point type landmarks shown. ***/
  bool qshowstitch; /*** True if any point type landmarks shown. ***/
  bool qshowstim; /*** True if any point type landmarks shown. ***/
  bool qshowlead; /*** True if any point type landmarks shown. ***/
  bool qshowcor;        /*** True if coronaries are to be drawn. ***/
  bool qshowplane;      /*** True if planes are to be drawn. ***/
  bool qtransplane;  /*** True if planes are to be translucent. ***/
  bool qshowrod;   /*** True if we want to show rods. ***/
  bool qshowrecneedle;     /*** True if we want to show recneedle. ***/
  bool qshowpaceneedle;   /*** True if we want to show paceneedle. ***/
  bool qshowcath; /*** True if we want to show catheter. ***/
  bool qshowfiber;          /*** True if we want to show fiber. ***/
  bool qshowcannula;    /*** True if we want to show cannula. ***/
  float coronarycolor[4];
  float occluscolor[4];
  float stitchcolor[4];
  float stimcolor[4];
  float leadcolor[4];
  float planecolor[4];
  float rodcolor[4];
  float recneedlecolor[4];
  float paceneedlecolor[4];
  float fibercolor[4];
  float cathcolor[4];
  float cannulacolor[4];

  int picked_segnum;  /*** the segnum a picked landmark ***/
  int picked_ptnum;   /*** the point within the seg of a picked landmark ***/

  bool modified_lm;
};

#define SURF_GROUP_MAX 10

struct Surface_Group
{
  float potmin, potmax;
  float framemin, framemax;
};

struct Clip_Planes
{
  Clip_Planes();
  void init();
  BallData bd;                  //arcball rotation
  bool lock_with_object;
  bool lock_together;
  bool front_enabled, back_enabled;
  float front_init, back_init;  //remember init values for reset
  float front, back;            // d values for planes
  float step;
  float frontmax, backmax;      //max values for clipping planes
};

enum DataUnits {UNIT_MVOLTS = 1, UNIT_UVOLTS, UNIT_MSECS, UNIT_VOLTS,
	    	UNIT_MVOLTMSECS};



#ifdef __cplusplus
extern "C"
{
#endif

  void Init_Global_Input(Global_Input * s);
  void Init_Scalar_Input(Scalar_Input * s);
  void Init_Surf_Input(Surf_Input * s);
  void Init_Surface_Group(Surface_Group * s);

#ifdef __cplusplus
}
#endif

#endif
