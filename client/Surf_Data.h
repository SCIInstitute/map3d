/* Surf_Data.h */

#ifndef SURFDATA_H
#define SURFDATA_H

#include "map3d-struct.h"

#include <set>

#define NUMCONTS 21

class Map3d_Geom;
class Mesh_Info;

class Surf_Data  /*** Data for a single surface. ***/
{
public:
  Surf_Data();
  ~Surf_Data();
  static Surf_Data *AddASurfData(Surf_Data * surfdata, long newsurfnum, long numframes, long numleads);

  //! Advance specified number of frames.  Loop is to largely be controlled by keyboard advancement IF global frame Loop
  //! option is set
  void FrameAdvance(int delta_frames, bool loopIfPastEnd = false);
  void SubseriesAdvance(int delta_subseries);
  int CurrentSubseries();
  void StackSubseries();
  void UnstackSubseries();
  void get_minmax(float &min, float &max);
  void get_fid_minmax(float &min, float &max, int type);
  void MinMaxPot(Map3d_Geom * onemap3dgeom);
  void ChangeReference(long leadnum, Map3d_Geom * onemap3dgeom);
  void ChangeReferenceMean(Map3d_Geom * onemap3dgeom);
  void ChangeBackReference(Map3d_Geom * onemap3dgeom);
  void SetupGlobalFids();
  void setUnits(int localunits);
  int getRealFrameNum() { return framenum * ts_sample_step + ts_start;}
  bool qgotpots;  /*** True if there pots for this surface ***/
  bool qgotgrads; /*** True if there pots for this surface ***/
  long surfnum; /*** Surface number for this set of potentials ***/
  long numframes; /*** Number of frames of data in this surface ***/
  long numleads; /*** Number of leads of data in this surface ***/
  long maxnumgrads; /*** Number of gradient vectors/. ***/
  long framenum; /*** Current framenumber. ***/
  long framestep; /*** # frames to jump at each arrow click ***/
  long seriesnum; /*** number of the time series in the .data file. ***/
  long numseries; /*** Total number of time series in the .data file. ***/
  float frametimestep; /*** Time between frames in msec ***/
  float frametime; /*** Frame time in msec ***/
  long ts_start;       /*** Starting frame of the display ***/
  long ts_end;         /*** Ending frame of the display ***/
  long ts_sample_step; /*** Step between frames of the display ***/
  long ts_available_frames; /*** Total number of available frames ***/
  long user_step;      /*** The original step for selecting from commandline*/
  float timestart;     /*** Starting time of the display ***/
  long zerotimeframe; /*** The frame number that we have as zero ***/
  long units;
  long scalesurf; /*** Surface to which scaling is locked (-1 = no lock) */
  int numconts; /*** Number of contours for displaying in this surface ***/
#if 1
  char label[100]; /*** String to use for label ***/
  char shortlabel[100]; /*** same as label, without series and surface number ***/
  char filepath[512]; /*** Datafilepath for .data files ***/
  char potfilename[512]; /*** Filename for this surface of data ***/
#else
  char *label;
  char *shortlabel;
  char *filepath;
  char *potfilename;
#endif
  float userpotmin, userpotmax; /*** User's selected values for max/min */
  float usercontourstep; /*** User's step between contours ***/
  float usergradmin, usergradmax;
  float userfidmin, userfidmax;
  float potmax, potmin; /*** Max/min pot values over surface ***/
  float potscale; /*** User scaling applied to data at read ***/
  float gradmax, gradmin; /*** Grad extrema over all frames ***/
  float fidmin, fidmax; /*** Extrame over all fids in this surface ***/
  float **potvals; /*** Array of data values ***/
  float *meanpotvals;           /* The mean potential over the surface at each time */
  float *rmspotvals;           /* The root mean square potential at each time */
  float *reference;             /* The reference signal for this surface */
  int referencelock;

  bool user_scaling;
  
  Surf_Data *mastersurf;
  MinMax_Frame *minmaxframes; /*** Potential extrema for each frame ***/
  int numglobalfids;
  float* globalfids; /*** Global fiducials, whatever that means! ***/
  short* globalfidtypes;
  char** globalfidnames;
  Series_Fids fids; /*** All lead by lead fiducials for this surface ***/
  MinMax_Fids *minmaxfids; /*** Min and max values for all the fids. ***/
  std::vector<int> subseriesStartFrames; /*** special type of global fid that represents when a subseries starts (eg, heartbeat) */
  std::vector<int> subseriesToStack; /* this should be a set, but I get weird crashes on my compiler */
  Mesh_Info *mesh; /*** pointer to its owning mesh ***/
};


#endif
