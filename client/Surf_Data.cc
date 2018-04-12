/* Surf_Data.cxx */

#include "Surf_Data.h"

#include "Map3d_Geom.h"
#include "WindowManager.h"
#include "scalesubs.h"
#include "map3dmath.h"
#include "MeshList.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

void *calloc(size_t, size_t);

extern vector<Surface_Group> surf_group;
extern Map3d_Info map3d_info;
extern const char *units_strings[5];

Surf_Data::Surf_Data()
{
  qgotpots = 0;
  qgotgrads = 0;
  surfnum = 0;
  numframes = 0;
  numleads = 0;
  maxnumgrads = 0;
  framenum = 0;
  framestep = 0;
  seriesnum = 0;
  numseries = 0;
  frametimestep = 0.0f;
  frametime = 0.0f;
  ts_start = 0;
  ts_end = 0;
  ts_available_frames = 0;
  user_step = 0;
  ts_sample_step = 0;
  timestart = 0.0f;
  zerotimeframe = 0;
  units = 0;
  scalesurf = 0;
  numconts = 20;
  label[0] = '\0';
  filepath[0] = '\0';
  potfilename[0] = '\0';
  userpotmin = 0.0f;
  userpotmax = 0.0f;
  usercontourstep = 0.0f;
  usergradmin = 0.0f;
  usergradmax = 0.0f;
  userfidmin = 0.0f;
  userfidmax = 0.0f;
  potmax = 0.0f;
  potmin = 0.0f;
  potscale = 0.0f;
  gradmax = 0.0f;
  gradmin = 0.0f;
  fidmin = 0.0f;
  fidmax = 0.0f;
  potvals = 0;
  mastersurf = 0;
  meanpotvals = 0;
  rmspotvals = 0;
  reference = 0;
  referencelock = 0;
  minmaxframes = 0;
  numglobalfids = 0;
  globalfids = 0;
  globalfidnames = 0;
  fids.leadfids = 0;
  fids.numfidleads = 0;
  minmaxfids = 0;
  user_scaling = 0;


}

Surf_Data::~Surf_Data()
{
  if (potvals)
    Free_fmatrix(potvals, numframes);
  if (meanpotvals)
    free(meanpotvals);
  if (rmspotvals)
    free(rmspotvals);
  if (reference)
    free(reference);

  if (minmaxframes)
    free(minmaxframes);
  if (minmaxfids)
    free(minmaxfids);

  if (fids.leadfids)                     // Series_Fids (defined in fids.h) doesn't have a destructor of its own
  {
    if (fids.fidtypes)
      free(fids.fidtypes);
    if (fids.fidnames)
      free(fids.fidnames);
    if (fids.leadfids) {
      if (fids.leadfids->fidtypes)
        free(fids.leadfids->fidtypes);
      if (fids.leadfids->fidvals)
        free(fids.leadfids->fidvals);
    }
  }
}

//static Surf_Data member
Surf_Data *Surf_Data::AddASurfData(Surf_Data * surfdata, long newsurfnum, long numframes, long numleads)
{

/*** Set up a new surface in the surfdata array

  Input:
  surfdata	    	the pointer to the array of surf_data structures
  newsurfnum 	number of the new surface 
  numframes	    	number of frames in the new surface
  numleads	    	number of leads or channels of data in the new surface
  Return:
  number of surfaces in the array of structures
  or value < 0 for error
  ***/

/**********************************************************************/

  /*** If this is the first surface, allocate. ***/

  if (newsurfnum == 0 || surfdata == NULL) {
    if ((surfdata = (Surf_Data *) calloc((size_t) 1, sizeof(Surf_Data))) == NULL) {
      fprintf(stderr, "*** AddaDataSurf: error getting first memory\n");
      return (NULL);
    }

  /*** If not, reallocate. ***/

  }
  else {
    if ((surfdata = (Surf_Data *) realloc(surfdata, (size_t) (newsurfnum + 1) * sizeof(Surf_Data))) == NULL) {
      fprintf(stderr, "*** AddaDataSurf: error reallocating memory\n");
     return (NULL);
    }
  }

    /*** Now allocate some memory for the potentials in the surface. ***/

  if (numframes > 0 && numleads > 0) {
    if ((surfdata[newsurfnum].potvals = Alloc_fmatrix(numframes, numleads)) == NULL) {
      fprintf(stderr, "*** AddaDataSurf: error getting first memory\n");
      return (NULL);
    }
  }
  else {
    surfdata[newsurfnum].potvals = NULL;
  }

    /*** Add some memory for the mean values and reference signal 
  of the potentials too. ***/

  if (numframes > 0) {
    if ((surfdata[newsurfnum].meanpotvals = (float *)calloc((size_t) numframes, sizeof(float))) == NULL) {
      fprintf(stderr, "*** AddaDataSurf: error allocating memory" " for means\n");
      return (NULL);
    }
    if ((surfdata[newsurfnum].rmspotvals = (float *)calloc((size_t) numframes, sizeof(float))) == NULL) {
      fprintf(stderr, "*** AddaDataSurf: error allocating memory" " for RMS\n");
      return (NULL);
    }
    if ((surfdata[newsurfnum].reference = (float *)calloc((size_t) numframes, sizeof(float))) == NULL) {
      fprintf(stderr, "*** AddaDataSurf: error allocating memory" " for reference signal\n");
      return (NULL);
    }
  }
  else {
    surfdata[newsurfnum].reference = NULL;
    surfdata[newsurfnum].meanpotvals = NULL;
    surfdata[newsurfnum].rmspotvals = NULL;
  }

    /*** Set up some parameters. ***/

  surfdata[newsurfnum].surfnum = newsurfnum;
  surfdata[newsurfnum].numframes = numframes;
  surfdata[newsurfnum].numleads = numleads;
  surfdata[newsurfnum].maxnumgrads = 0;
  surfdata[newsurfnum].framenum = 0;
  surfdata[newsurfnum].framestep = 1;
  surfdata[newsurfnum].frametimestep = 1.0;
  surfdata[newsurfnum].frametime = 0.0;
  surfdata[newsurfnum].ts_start = 0;
  surfdata[newsurfnum].ts_end = 0;
  surfdata[newsurfnum].ts_available_frames = 0;
  surfdata[newsurfnum].ts_sample_step = 1;
  surfdata[newsurfnum].timestart = 0.0;
  surfdata[newsurfnum].zerotimeframe = 0;
  surfdata[newsurfnum].units = 0;
  surfdata[newsurfnum].scalesurf = -1;
  if (numframes > 0 && numleads > 0)
    surfdata[newsurfnum].qgotpots = true;
  else
    surfdata[newsurfnum].qgotpots = false;
  surfdata[newsurfnum].qgotgrads = false;
  surfdata[newsurfnum].potmax = -1.e10;
  surfdata[newsurfnum].potmin = 1.e10;
  surfdata[newsurfnum].gradmax = -1.e10;
  surfdata[newsurfnum].gradmin = 1.e10;
  surfdata[newsurfnum].fidmax = -1.e10;
  surfdata[newsurfnum].fidmin = 1.e10;

  surfdata[newsurfnum].userpotmin = 0.0;
  surfdata[newsurfnum].userpotmax = 0.0;
  surfdata[newsurfnum].usergradmin = 0.0;
  surfdata[newsurfnum].usergradmax = 0.0;
  surfdata[newsurfnum].userfidmin = 0.0;
  surfdata[newsurfnum].userfidmax = 0.0;
  surfdata[newsurfnum].minmaxframes = NULL;
  surfdata[newsurfnum].fids.leadfids = NULL;
  surfdata[newsurfnum].fids.numfidleads = NULL;
  surfdata[newsurfnum].potfilename[0] = '\0';
  //strcpy(surfdata[newsurfnum].potfilename,"");
  if (newsurfnum > 0)
    surfdata[newsurfnum].numconts = surfdata[newsurfnum - 1].numconts;
  else
    surfdata[newsurfnum].numconts = NUMCONTS;
  strcpy(surfdata[newsurfnum].label, "");
  strcpy(surfdata[newsurfnum].filepath, "");
  return surfdata;
}

void Surf_Data::FrameAdvance(int delta_frames, bool loopIfPastEnd /* = false */)
{
  int newframe = framenum + delta_frames;
  if (newframe < 0 && loopIfPastEnd)
    newframe = newframe + numframes; // loop back to the end
  else if (newframe >= numframes && loopIfPastEnd)
    newframe = newframe - numframes; // loop to the beginning

  // even if looping, sanity check against weird conditions like frame step being bigger than dataset
  if (newframe < numframes && newframe >= 0) {
    framenum = newframe;
  }
}

int Surf_Data::CurrentSubseries()
{
  int subseriesNum = 0;
  for (; subseriesNum < subseriesStartFrames.size(); subseriesNum++)
  {
    int nextSubseriesFrame = subseriesNum < subseriesStartFrames.size() - 1 ? subseriesStartFrames[subseriesNum + 1] : numframes;
    if (nextSubseriesFrame >= framenum)
      break;
  }
  return subseriesNum;
}

void Surf_Data::SubseriesAdvance(int delta_subseries)
{
  if (subseriesStartFrames.size() == 0)
    return; // nothing to do!
  for (int i = 0; i < abs(delta_subseries); i++)
  {
    // find the current subseries.  We don't care about efficiency since this function was designed
    // to really should be called with +/- 1
    int currentSubseries = CurrentSubseries();
    int currentSubseriesFrame = subseriesStartFrames[currentSubseries];
    int prevSubseriesFrame = currentSubseries > 0 ? subseriesStartFrames[currentSubseries - 1] : 0;
    int nextSubseriesFrame = currentSubseries < subseriesStartFrames.size() - 1 ? subseriesStartFrames[currentSubseries + 1] : numframes;

    int posWithinSubseries = framenum - currentSubseriesFrame;
    if (delta_subseries < 0 && prevSubseriesFrame + posWithinSubseries > 0)
      framenum = prevSubseriesFrame + posWithinSubseries;
    if (delta_subseries > 0 && nextSubseriesFrame+posWithinSubseries < numframes)
      framenum = nextSubseriesFrame + posWithinSubseries;
  }
}

void Surf_Data::StackSubseries()
{
  int series = CurrentSubseries();
  for (int i = 0; i < subseriesToStack.size(); i++)
    if (subseriesToStack[i] == series)
      return;
  subseriesToStack.push_back(series);
  // should be subseriesToStack.insert(series), but that crashes for some reason
}

void Surf_Data::UnstackSubseries()
{
  int series = CurrentSubseries();
  for (int i = 0; i < subseriesToStack.size(); i++)
    if (subseriesToStack[i] == series)
    {
      // sight, this crashes too. subseriesToStack.erase(subseriesToStack.begin() + i);
      for (int j = i; j < subseriesToStack.size() - 1; j++) // do manual erase
      {
        subseriesToStack[j] = subseriesToStack[j + 1];
      }
      subseriesToStack.pop_back();
      i--;
    }

  // should be subseriesToStack.erase(CurrentSubseries());
}


void Surf_Data::get_minmax(float &min, float &max)
{
  int curframe = framenum;

  // enable the user to use user-scaling on 1 surf while the rest are using whatever
  // scaling is selected.
  if (user_scaling && userpotmin < userpotmax) {
    min = userpotmin;
    max = userpotmax;
    return;
  }
  switch (map3d_info.scale_scope) {
  case GLOBAL_SURFACE:         /* min and max over collection of frames (one surface) */
    max = potmax;
    min = potmin;
    break;
  case GLOBAL_FRAME:           /* min and max over one frame (all surfaces) */
    if (!map3d_info.scale_frame_set) {
      FrameMinMax();
      map3d_info.scale_frame_set = 1;
    }
    max = map3d_info.scale_frame_max;
    min = map3d_info.scale_frame_min;
    break;
  case GROUP_FRAME:
    if (!map3d_info.scale_frame_set) {
      for (unsigned i = 0; i < surf_group.size(); i++) {
        recalcGroup(i);
        map3d_info.scale_frame_set = 1;
      }
    }
    min = surf_group[mesh->groupid].framemin;
    max = surf_group[mesh->groupid].framemax;
    break;
  case GROUP_GLOBAL:
    min = surf_group[mesh->groupid].potmin;
    max = surf_group[mesh->groupid].potmax;
    break;
  case LOCAL_SCALE:                  /* relative only to one frame of one surface */
    if (minmaxframes != NULL) {
      max = minmaxframes[curframe].potmax;
      min = minmaxframes[curframe].potmin;
    }
    else {
      max = 0;
      min = 0;
    }
    break;
  case GLOBAL_GLOBAL:          /* global min and max */
    max = map3d_info.global_potmax;
    min = map3d_info.global_potmin;
    break;
  case SLAVE_FRAME:
    if (mastersurf && mastersurf != this)
      mastersurf->get_minmax(min, max);
#if 0
      if (mastersurf->minmaxframes != NULL) {
        max = mastersurf->minmaxframes[mastersurf->framenum].potmax;
        min = mastersurf->minmaxframes[mastersurf->framenum].potmin;
      }
      else {
        max = 0;
        min = 0;
      }
#endif
    else if (minmaxframes != NULL) {
      max = minmaxframes[curframe].potmax;
      min = minmaxframes[curframe].potmin;
    }
    else {
      max = 0;
      min = 0;
    }
    break;
  case SLAVE_GLOBAL:
    if (mastersurf) {
      mastersurf->get_minmax(min, max);
#if 0
      max = mastersurf->potmax;
      min = mastersurf->potmin;
#endif
    }
    else {
      max = potmax;
      min = potmin;
    }
    break;
  default:                     /* same here */
    max = minmaxframes[curframe].potmax;
    min = minmaxframes[curframe].potmin;
    break;
  }
}

void Surf_Data::get_fid_minmax(float &min, float &max, int type)
{
  
  float minval = 1.0e10;
  float maxval = -1.0e10;
  float val;
  for(int leadnum = 0; leadnum < fids.numfidleads; leadnum++){
    for(int fidnum = 0; fidnum < fids.leadfids[leadnum].numfids; fidnum++){
      if(fids.leadfids[leadnum].fidtypes[fidnum] == type){
        val = fids.leadfids[leadnum].fidvals[fidnum];
        if (val < minval) {
          minval = val;
        }
        if (val > maxval) {
          maxval = val;
        }
      }
    }
  }
  min = minval;
  max = maxval;
}

void Surf_Data::MinMaxPot(Map3d_Geom * onemap3dgeom)
{
  /*** Find the extrema in the potentials.  ***/

  long framenum, leadnum, numvals;
  long nodemin = 0, nodemax = 0;
  float val, minval, maxval, meanval, rmsval;
  double sum, rmsSum;
  /**********************************************************************/
  /*** Set up some memory for the values. ***/

  if (minmaxframes)
    free(minmaxframes);
  if ((minmaxframes = (MinMax_Frame *) calloc(numframes, sizeof(MinMax_Frame))) == NULL) {
    printf("SetMinMaxDataSurf: error getting memory\n");
    exit(ERR_MEM);
  }

  /*** Loop through all the frames.  ***/

  for (framenum = 0; framenum < numframes; framenum++) {
    minval = 1.0e10;
    maxval = -1.e10;
    nodemin = -1;
    nodemax = -1;

    /*** Loop through all the nodes in the geometry = values of potential. ***/

    sum = 0.0;
    rmsSum = 0.0;
    numvals = 0;//Check for channels later
    for (leadnum = 0; leadnum < numleads; leadnum++) {
      val = potvals[framenum][leadnum];
      if (val == UNUSED_DATA)
        continue;
      numvals++;
      sum += val;
      rmsSum += val * val;
      if (val < minval) {
        minval = val;
        nodemin = leadnum;
      }
      if (val > maxval) {
        maxval = val;
        nodemax = leadnum;
      }
    }

    meanval = (float)(sum / (float)numvals);
    if (map3d_info.reportlevel > 2)
      if (framenum < 20)
        printf(" Mean value for surface %ld, frame %ld is %f\n", surfnum + 1, framenum + 1, meanval);

    rmsval = (float)((sqrt(rmsSum))/(float)numvals);

      /*** Set up the extrema and mean for this frame. ***/
    rmspotvals[framenum] = rmsval;
    meanpotvals[framenum] = meanval;
    minmaxframes[framenum].potmax = maxval;
    minmaxframes[framenum].potmin = minval;
    minmaxframes[framenum].nodemax = nodemax;
    minmaxframes[framenum].nodemin = nodemin;
    minmaxframes[framenum].channelmax = onemap3dgeom->channels[nodemax];
    minmaxframes[framenum].channelmin = onemap3dgeom->channels[nodemin];

        /*** Update the lead number of the extrema according to the leadlinks
      information. ***/

    minmaxframes[framenum].leadnummax = -1;
    minmaxframes[framenum].leadnummin = -1;
    for (leadnum = 0; leadnum < onemap3dgeom->numleadlinks; leadnum++) {
      if (onemap3dgeom->leadlinks[leadnum] == nodemax)
        minmaxframes[framenum].leadnummax = leadnum;
      if (onemap3dgeom->leadlinks[leadnum] == nodemin)
        minmaxframes[framenum].leadnummin = leadnum;
    }

      /*** Update the surface extrema, based on what we have found for this frame.
      ***/
    potmax = cjmax(potmax, maxval);
    potmin = cjmin(potmin, minval);
  }

  /*+++ */
  if (map3d_info.reportlevel > 2)
    printf("\n For surface %ld over %ld frames potmax = %f "
           "at node %ld and\n"
           " potmin = %f at node %ld\n", surfnum + 1, numframes, potmax, nodemax + 1, potmin, nodemin + 1);
}


void Surf_Data::ChangeReference(long leadnum, Map3d_Geom * onemap3dgeom){
  if(referencelock > 0){
    printf("Reset Reference First\n");
  }
  else{
    for(long i = 0; i < numframes; i++){
      reference[i] = potvals[i][leadnum];
      for(long j = 0; j < numleads; j++){
	potvals[i][j] = potvals[i][j] - reference[i];
      }
    }
    MinMaxPot(onemap3dgeom);
    GlobalMinMax();
    referencelock++;
  }
}

void Surf_Data::ChangeReferenceMean(Map3d_Geom * onemap3dgeom){
  if(referencelock > 0){
    printf("Reset Reference First\n");
  }
  else{
    for(long i = 0; i < numframes; i++){
      reference[i] = meanpotvals[i];
      for(long j = 0; j < numleads; j++){
	potvals[i][j] = potvals[i][j] - reference[i];	
      }
    }
    MinMaxPot(onemap3dgeom);
    GlobalMinMax();
    referencelock++;
  }
}

void Surf_Data::ChangeBackReference(Map3d_Geom * onemap3dgeom){
  if(referencelock <=0){
  }
  else{
    printf("Reseting Reference\n");
    for(long i = 0; i < numframes; i++){
      for(long j = 0; j < numleads; j++){
	potvals[i][j] = potvals[i][j] + reference[i];
      }
    }
    MinMaxPot(onemap3dgeom);
    GlobalMinMax();
    referencelock--;
  }
}

void Surf_Data::setUnits(int localunits)
{
  if (localunits < 0) {
    fprintf(stderr, " No unit available for data\n");
    localunits = 0;

  }
  else {
    if (localunits > 0 && localunits <= 5) {
      if (map3d_info.reportlevel)
        printf(" Units of the original data series are %s\n", units_strings[localunits - 1]);

      /*** Now see if we have applied a user scale factor that we can
	   figure out, and see if that affects the units we believe the data
	   are in. 
      ***/
      if (fabs(((float)fabs(potscale) - 1.0)) > .0001) {
        /*** If pot scale == 1000. ***/
        if (potscale > 999 && potscale < 1001) {
          if (localunits == UNIT_MVOLTS) {
            localunits = UNIT_UVOLTS;
            fprintf(stderr, " Units changed from milli to micro" "volts by scaling value of %f\n", potscale);
          }
          else if (localunits == UNIT_VOLTS) {
            localunits = UNIT_MVOLTS;
            fprintf(stderr, " Units changed from volts to" "millivolts by scaling value of %f\n", potscale);
          }
          /*** Else if potscale == .001 ***/
        }
        else if (potscale > .00099 && potscale < .0011) {
          if (localunits == UNIT_UVOLTS) {
            localunits = UNIT_MVOLTS;
            fprintf(stderr, " Units changed from micro to milli" "volts by scaling value of %f\n", potscale);
          }
          else if (localunits == UNIT_MVOLTS) {
            localunits = UNIT_VOLTS;
            fprintf(stderr, " Units changed from millivolts to" "volts by scaling value of %f\n", potscale);
          }
        }
      }
    }
    else {
      fprintf(stderr, " Illegal or unknown unit value (%ld)\n", localunits);
      fprintf(stderr, " Set to millivolts\n");
      localunits = UNIT_MVOLTS;
    }
  }
  units = localunits;
}

void Surf_Data::SetupGlobalFids()
{
	// for the global fids that are subseries markers, organize and sort them
	for (int i = 0; i < numglobalfids; i++)
	{
		if (globalfidtypes[i] == FI_SUBSERIES)
		{
			subseriesStartFrames.push_back((int)globalfids[i] - 1); // they come in 1-based, and 1 should be the first
		}
	}
	// TODO - this asserts on win32 debug
	//std::sort(subseriesStartFrames.begin(), subseriesStartFrames.end());
};