/* readfiles.cxx */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <set>
#include <string.h>
#include <stdlib.h>

#include "landmarks.h"
#include "readfiles.h"
#include "Surf_Data.h"
#include "Map3d_Geom.h"
#include "MeshList.h"
#include "geomlib.h"
#include "fids.h"
#include "map3dmath.h"
#include "MatlabIO.h"
#include "dialogs.h"
#include "Contour_Info.h"
#include <math.h>

#ifdef _WIN32
#pragma warning(disable:4172 4514)  /* quiet visual c++ */
#endif


extern Map3d_Info map3d_info;
extern Map3d_Geom *map3d_geom;
extern char *units_strings[5];
extern char datalabels[][100];
extern char geomlabels[][100];
extern Map3d_Info map3d_info;
FileCache file_cache;
using namespace std;

matlabarray* FileCache::readMatlabFile(string str)
{
  if (cache.find(str) != cache.end())
    return (matlabarray*) cache[str];
  matlabfile mf(str, "r");
  matlabarray *ma = new matlabarray;
  *ma = mf.getmatlabarray(0);
  cache[str] = (void*) ma;
  return ma;  
}

//Try filename, and then path1/filename, and then path2/filename.  Return a FILE* if the
//  user wants to use it (some files are opened with openfile_), and the filename in out.
//  path1 is the datapath/geompath environment var, and path2 is command-line overriding of
//  those paths.
bool getFileWithPath(char* filename, char* path1, char* path2, char out[])
{
  FILE* luin_p;
  // try original filename
  string file_to_test = filename;
  if ((luin_p = fopen(file_to_test.c_str(), "r")) == NULL) {
    // if it fails, prepend path1
    if (path1)
      file_to_test = string(path1) + '/' + filename;
    if ((luin_p = fopen(file_to_test.c_str(), "r")) == NULL) {
      //if path1/filename fails, prepend path2
      if (path2)
        file_to_test = string(path2) + '/' + filename;
      if ((luin_p = fopen(file_to_test.c_str(), "r")) == NULL) {
        fprintf(stderr,"Could not not file: %s\n",filename);
        strcpy(out, filename);
        return false;
      }
    }
  }
  if (luin_p)
    fclose(luin_p);
  strcpy(out, file_to_test.c_str());
  if (map3d_info.reportlevel >= 2)
    printf("Final filename is %s\n",out);
  return true;
}
long ReadPts(Map3d_Geom* onemap3dgeom)
/*** 
Read the point locations from the already open file
Keep track of the number of points in each part or surface.

This routine also uses dynamic memory allocation to set space
aside for the points array.

***/
{
  long error = 0, insurfnum, numsurfsread = 1;
  char filename[100], basefilename[100];
  Surf_Geom *onesurfgeom = NULL;
  /*******************************************************************/
  
  insurfnum = onemap3dgeom->surfnum;
  if (map3d_info.reportlevel > 1)
    fprintf(stderr, "In ReadPts for surface #%ld\n", insurfnum + 1);
  
  if ((onesurfgeom = AddASurfGeom(onesurfgeom, 0, &numsurfsread)) == NULL)
    exit(ERR_MEM);
  
  /*** Use the standard routine to read the points in. ***/
  
  strcpy(basefilename, onemap3dgeom->basefilename);
  StripExtension(basefilename);
  strcpy(filename, basefilename);
  strcat(filename, ".pts");
    
  strcpy(onesurfgeom->filename, filename);
  
  if (map3d_info.reportlevel)
    fprintf(stderr, "Reading pts file: %s\n", filename);
  
  error = ReadPtsFile(onesurfgeom, map3d_info.reportlevel > 1);
  if (error < 0)
    return (error);
    
  /*** Now transfer stuff between structures. ***/
  
  onemap3dgeom->points[onemap3dgeom->geom_index] = onesurfgeom->nodes;
  onemap3dgeom->numpts = onesurfgeom->numpts;
  onemap3dgeom->channels = onesurfgeom->channels;
  
  //strcpy(onemap3dgeom->label, basefilename);
  return (error);
}


/*=====================================================================*/

long ReadSegs(FILE * luin_p, Map3d_Geom* onemap3dgeom)
/*** Read the segment connectivities from the open file.
This version has dynamic meory allocatiom but no new format
with a line specifying number of lines in the file.

***/
{
  long nfsegs, nsegread, i, j;
  long i1, i2, i3;
  char instring[100];
  bool gotgroups;
  int base = 1;
  /*********************************************************************/
  
  if (fgets(instring, 100, luin_p) == NULL) {
    fprintf(stderr, "**** ReadSegs Error: end of file hit " " at start of file" " ***\n");
    exit(ERR_FILE);
  }
  
  /*** See if we have a group number appended to the triangle
    connectivity. ***/
  
  gotgroups = (((sscanf(instring, " %ld %ld %ld ", &i1, &i2, &i3)) == 3) ? true : false);
  if (gotgroups) {
    fprintf(stderr, " Looks like there are group " "number in the segment" " file\n");
  }
  
  /*** Now rewind the file and see how many segments are in the file. ***/
  
  rewind(luin_p);
  nfsegs = GetFileLength(luin_p);
  rewind(luin_p);
  if (map3d_info.reportlevel > 1)
    fprintf(stderr, " Number of segments in the file should be %ld\n", nfsegs);
  
  /*** Allocate space for the map3dgeom segment array. ***/
  
  long error = onemap3dgeom->SetupMap3dSurfElements(nfsegs, 2);
  if (error < 0) {
    return error;
  }
  
  if (gotgroups) {
    if ((onemap3dgeom->seggroups = (long *)calloc((size_t) nfsegs, sizeof(long))) == NULL) {
      printf("+++ ReadSegs: no memory for point groups\n");
      gotgroups = false;
    }
  }
  
  /*** Read some data finally.  ***/
  
  nsegread = 0;
  if (gotgroups) {
    nsegread = 0;
    rewind(luin_p);
    for (i = 0; i < nfsegs; i++) {
      if (fscanf(luin_p, "%ld %ld %ld\n",
                 &onemap3dgeom->elements[i][0], &onemap3dgeom->elements[i][1], &onemap3dgeom->seggroups[i]) != 3) {
        fprintf(stderr, "***** In ReadSeg, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nfsegs, nsegread);
        exit(ERR_FILE);
      }
      nsegread++;
    }
  }
  else {
    nsegread = 0;
    rewind(luin_p);
    for (i = 0; i < nfsegs; i++) {
      if (fscanf(luin_p, "%ld %ld\n", &onemap3dgeom->elements[i][0], &onemap3dgeom->elements[i][1]) != 2) {
        fprintf(stderr, "***** In ReadSeg, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nfsegs, nsegread);
        exit(ERR_FILE);
      }
      if (onemap3dgeom->elements[i][0] == 0 || onemap3dgeom->elements[i][1] == 0)
        base = 0;
      nsegread++;
    }
  }
  
  /* allow user to specify 0-based files.  Default is 1-based, but 
    if there is a 0 in the file, make the whole file 0-based. 
    If there is a 0, don't subtract 1. */
  
  
  if (base == 1) {
    for (i = 0; i < nfsegs; i++) {
      for (j = 0; j < 3; j++) {
        onemap3dgeom->elements[i][j]--;
      }
    }
  }
  onemap3dgeom->numelements = nfsegs;
  return 0;
}

/*=====================================================================*/

long ReadTris(Map3d_Geom* onemap3dgeom)
/*** Read the triangle connectivities from the open file 
This version uses dynamic memory allocation to read the data
from the file.

Dymanic memory allocation version.
***/
{
  long error = 0, numsurfsread = 1;
  char filename[100], basefilename[100];
  Surf_Geom *onesurfgeom = NULL;
  /*********************************************************************/
    
  if ((onesurfgeom = AddASurfGeom(onesurfgeom, 0, &numsurfsread)) == NULL)
    exit(ERR_MEM);
  
  
  /*** Use the standard routine to read the points in. ***/
  
  strcpy(basefilename, onemap3dgeom->basefilename);
  StripExtension(basefilename);
  strcpy(filename, basefilename);
  strcat(filename, ".fac");
  strcpy(onesurfgeom->filename, filename);
  
  if (map3d_info.reportlevel)
    fprintf(stderr, "Reading tris file %s\n", filename);

  error = ReadFacFile(onesurfgeom, map3d_info.reportlevel > 1);
  if (error < 0)
    return (error);
  
  if (onesurfgeom->elementsize != 3) {
    fprintf(stderr, "**** ReadTri: elements in file are of size %ld\n"
            "**** Filename: %s\n", onesurfgeom->numelements, filename);
    return (ERR_MISC);
  }
  
  /*** Now transfer stuff between structures. ***/
  
  onemap3dgeom->elements = onesurfgeom->elements;
  onemap3dgeom->elementsize = 3;
  onemap3dgeom->numelements = onesurfgeom->numelements;
  onemap3dgeom->maxnumelts = onesurfgeom->numelements;
  return (error);
}

/*================================================================*/

long ReadTetras(FILE * luin_p, Map3d_Geom* onemap3dgeom)
/*** Read the tetrahedral connectivities from the open file 
New format using dynamic memory allocation.

***/
{
  long nftetras, ntetraread, i, j;
  long i1, i2, i3, i4, i5;
  char instring[100];
  bool gotgroups;
  int base = 1;
  /*********************************************************************/
  
  if (fgets(instring, 100, luin_p) == NULL) {
    fprintf(stderr, "**** ReadTetra Error: end of file hit " " at start of file" " ***\n");
    return ERR_FILE;
  }
  
  
  /*** See if we have a group number appended to the tetrahedron
    connectivity. ***/
  
  gotgroups = (((sscanf(instring, " %ld %ld %ld %ld %ld", &i1, &i2, &i3, &i4, &i5)) == 5) ? true : false);
  if (gotgroups) {
    fprintf(stderr, " Looks like there are group number in the tetra" " file\n");
  }
  
  /*** Now rewind the file and see how many tetra are in the file. ***/
  
  rewind(luin_p);
  nftetras = GetFileLength(luin_p);
  rewind(luin_p);
  fprintf(stderr, " Number of tetras in the file should be %ld\n", nftetras);
  
  /*** Allocate space for the map3dgeom tetra array. ***/
  int error = onemap3dgeom->SetupMap3dSurfElements(nftetras, 4);
  if (error < 0) {
    fprintf(stderr, "*** ReadTeteras: error getting memory" " for tetras\n");
    return ERR_MEM;
  }
  
  if (gotgroups) {
    if ((onemap3dgeom->tetragroups = (long *)calloc((size_t) nftetras, sizeof(long))) == NULL) {
      printf("+++ ReadPts: no memory for point groups\n");
      return ERR_MEM;
    }
    printf(" Set up space for %ld group numbers\n", nftetras);
  }
  
  /*** Now read the data from the file.  ***/
  
  ntetraread = 0;
  if (gotgroups) {
    rewind(luin_p);
    for (i = 0; i < nftetras; i++) {
      if (fscanf(luin_p, "%ld %ld %ld %ld %ld",
                 &onemap3dgeom->elements[i][0],
                 &onemap3dgeom->elements[i][1],
                 &onemap3dgeom->elements[i][2], &onemap3dgeom->elements[i][3], &onemap3dgeom->tetragroups[i]) != 5)
      {
        fprintf(stderr, "***** In ReadTetra, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nftetras, ntetraread);
        return ERR_FILE;
      }
      ntetraread++;
    }
  }
  else {
    rewind(luin_p);
    for (i = 0; i < nftetras; i++) {
      if (fscanf(luin_p, "%ld %ld %ld %ld",
                 &onemap3dgeom->elements[i][0],
                 &onemap3dgeom->elements[i][1],
                 &onemap3dgeom->elements[i][2], &onemap3dgeom->elements[i][3]) != 4) {
        fprintf(stderr, "***** In ReadTetra, error\n"
                "    fscanf failed before points all read\n"
                "     We expected %ld and only got %ld\n", nftetras, ntetraread);
        return ERR_FILE;
      }
      if (onemap3dgeom->elements[i][0] == 0 || onemap3dgeom->elements[i][1] == 0 || 
          onemap3dgeom->elements[i][2] == 0 || onemap3dgeom->elements[i][3] == 0)
        base = 0;
      ntetraread++;
    }
  }
  
  /* allow user to specify 0-based files.  Default is 1-based, but 
    if there is a 0 in the file, make the whole file 0-based. 
    If there is a 0, don't subtract 1. */
  
  if (base == 1) {
    for (i = 0; i < nftetras; i++) {
      for (j = 0; j < 4; j++) {
        onemap3dgeom->elements[i][j]--;
      }
    }
  }
  onemap3dgeom->numelements = nftetras;
  return 0;
}

/*=====================================================================

NOTE: This function right now is in a state to only open 1 matlab time series at a time
i.e., numsurfsread = 1.
This is fine, as right now map3d only reads one series at a time.  To change this
later, you'll need to move the majority of the code into one for loop (instead of all of the
                                                                       for (displaysurfnum ...) loops right before the retrieval of units. */
Surf_Data *ReadMatlabDataFile(Surf_Input * sinputlist, Surf_Data * surfdata, long numsurfsread, long insurfnum)
{
  /***
Input:
  sinputlist	    structure of all the surface information we need
  surfdata	    the surfdata array for all the surfaces
  numsurfsread    number of surfaces for which we have geometry
  and hence, expect to find data.
  insurfnum	    surface number in which to start storing data
  
Output:	    Loads of externals
Return:	    pointer to updated surf_data array or NULL for error
  
  
  ***********************************************************************/
  long errorlevel;
  long numseries = 1, numfileframes = 0;
  long framenum, modelframenum;
  long framestart, frameend, framestep, numreadframes;
  long displaysurfnum, surfstart, surfend, surfcount;
  long nodenum, numsurfnodes;
  long numleads, numfileleads = 0;
  long index, maxindex;
  long localunits = 0;
  long reportlevel;
  float potscale;
  float *databuff;
  char  clabel[100], csurface[20], cseries[20];
  
  clabel[0] = '\0'; // init it in case there is no label in the matlab file
  matlabarray unit;
  matlabarray label;
  matlabarray potvals;
  matlabarray cell;
  matlabarray fids;

  /*********************************************************************/
  reportlevel = map3d_info.reportlevel;
  errorlevel = 1;
  
  
  
  if (reportlevel)
    fprintf(stderr, "Reading matlab file %s, series #%ld\n", sinputlist->potfilename, insurfnum+1);
  
  if (insurfnum < 0) {
    fprintf(stderr, "*** In ReadMatlabDataFile: Error since insurfnum = %ld\n" " Exit here\n", insurfnum);
    return NULL;
  }
  
  // Try an open the data file.  If it fails to find it in the current path,
  // prepend the MAP3D_DATAPATH environment variable
  
  char datafilename[256];
  char* filename = (char *)sinputlist->potfilename;
  
  if (!getFileWithPath(filename,getenv("MAP3D_DATAPATH"),sinputlist->parent->datapathname,datafilename)) {
    fprintf(stderr, "Could not locate file: %s", filename);
    return NULL;
  }
  
  matlabarray ma;
  string name;
  
  if (sinputlist->preloadedDataArray == NULL)
  {
    // open the matlab file and get the root array
    // DON'T assign back to dataMA here like in other places, we'll be done with it soon
	matlabfile mf; // let it go out of scope here - it will save the file's baggage so we don't compound here
	mf.open(datafilename, "r");
    ma = mf.getmatlabarray(0);
  }
  else
    ma = *(sinputlist->preloadedDataArray);

  name = ma.getname();

  /*** Set the datapath if we have one for the data files. ***/
  
  if (sinputlist->parent->datapathname) {
    if (reportlevel > 1)
      fprintf(stderr, " We have an alternate data path!\n" " Path: %s\n", sinputlist->parent->datapathname);
  }
  
  /*** Get the basic info from the file.  ***/
  
  if (ma.isstruct()) {
    numseries = ma.getnumelements();
    if (ma.isfieldCI("unit")) unit = ma.getfieldCI(insurfnum,"unit");
    if (ma.isfieldCI("label")) label = ma.getfieldCI(insurfnum,"label");
    if (ma.isfieldCI("potvals")) potvals = ma.getfieldCI(insurfnum,"potvals");
    if (ma.isfieldCI("data")) potvals = ma.getfieldCI(insurfnum,"data");
    if (ma.isfieldCI("field")) potvals = ma.getfieldCI(insurfnum,"field");
    if (ma.isfieldCI("scalarfield")) potvals = ma.getfieldCI(insurfnum,"scalarfield");
    if (ma.isfieldCI("fids")) fids = ma.getfieldCI(insurfnum,"fids");

  }
  else if (ma.iscell()) {
    numseries = ma.getnumelements();
    cell = ma.getcell(insurfnum);
    if (cell.isfieldCI("unit")) unit = cell.getfieldCI(0,"unit");
    if (cell.isfieldCI("label")) label = cell.getfieldCI(0,"label");
    if (cell.isfieldCI("potvals")) potvals = cell.getfieldCI(0,"potvals");
    if (cell.isfieldCI("data")) potvals = cell.getfieldCI(0,"data");
    if (cell.isfieldCI("field")) potvals = cell.getfieldCI(0,"field");
    if (cell.isfieldCI("scalarfield")) potvals = cell.getfieldCI(0,"scalarfield");
    if (cell.isfieldCI("fids")) fids = cell.getfieldCI(0,"fids");
  }
  else if (ma.isdense()) { 
    potvals = ma;
  }
  
  if (reportlevel) {
    printf(" Back with the info on the data file\n" " number of time series available = %ld\n", numseries);
  }
  
  /*** Now select the index of the series we want to look at.
    If we have a tsdf file, then there is only one time series.  ***/
  
  framestart = sinputlist->ts_start;
  frameend = sinputlist->ts_end;
  framestep = sinputlist->ts_sample_step;
  
  if (reportlevel > 1)
    printf(" Passed index value is #%ld\n", insurfnum);
  
  /*** Scaling of potentials ***/
  
  potscale = sinputlist->potscale;
  
  if (!unit.isempty() && unit.isstring())
  {
    localunits = 0;
    if (unit.compareCI("mv")) localunits = UNIT_MVOLTS;
    if (unit.compareCI("uv")) localunits = UNIT_UVOLTS;
    if (unit.compareCI("V")) localunits = UNIT_VOLTS;
  }
  
  if (!label.isempty() && label.isstring()) {
    strcpy(clabel, label.getstring().c_str());
  }
  
  
  
  /***  Frame Selection
    
    Set up the frame numbers here.  We may specify them explicitly
    desire to set them interactively based on the power curve. 
    Note that internally, frames begin at 0, while externally,
    they start at 1.
    ***/
  surfstart = insurfnum;
  surfend = insurfnum + numsurfsread - 1; 
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    numfileframes = 0;
    numfileleads = 0;
    
    if (!potvals.isempty()) {
      numfileleads = potvals.getm();
      numfileframes = potvals.getn();
      
      if((numfileleads != map3d_geom[displaysurfnum].numpts)&&
         (numfileframes == map3d_geom[displaysurfnum].numpts)){
        potvals.transpose();
        numfileleads = potvals.getm();
        numfileframes = potvals.getn();
      }
    }
    else {
      fprintf(stderr, "Cannot find potential data in file %s\n", sinputlist->potfilename);
      fprintf(stderr, "Use matlab matrix with name 'potvals', 'data', 'field', or 'scalarfield'\n");
      return 0;
    }
  }
  if (reportlevel > 1)
    printf(" In the data file, number of leads = %ld \n" " number of frames = %ld\n", numfileleads, numfileframes);
  
  /*** If we only have one frame, then start and end frames are
    decided for us. If we have zero or less, we have a problem.
    ***/
  if (numfileframes == 1) {
    framestart = frameend = 0;
    framestep = 1;
  }
  else if (numfileframes <= 0) {
    fprintf(stderr, "*** (ReadMatlabDataFile)\n" "*** There are no frames in this datafile\n" "*** So have to quit\n");
    return 0;
  }
  else if (frameend < framestart){
    framestart = 0;
    frameend = numfileframes - 1;
  }
  
  if (framestep <= 0)
    framestep = 1;
  
  
  /*** Make sure the last frame does not go beyond the end of the file.
    Then make sure frame numbers are OK.
    ***/
  if (frameend >= numfileframes) {
    if (reportlevel)
      fprintf(stderr, "+++ ReadMatlabDataFile: Selected last frame = %ld "
              "is too large\n"
              "    The last one in the file is %ld\n" "    so we update here\n", frameend + 1, numfileframes);
    frameend = numfileframes - 1;
  }
  
  if (framestart >= numfileframes) {
    if (reportlevel)
      fprintf(stderr, "+++ ReadMatlabDataFile: Selected last frame = %ld "
              "is too large\n"
              "    The last one in the file is %ld\n" "    so we update here\n", frameend + 1, numfileframes);
    framestart = numfileframes - 1;
  }
  
  if (reportlevel > 1)
    printf(" After selecting, first frame is %ld and last is  %ld" " with inc %ld\n", framestart, frameend, framestep);
  
  if (framestart > frameend || framestep < 0) {
    fprintf(stderr, "**** After selecting, first frame is %ld"
            " and last is  %ld"
            " with inc %ld\n" "**** Start frame must be smaller than finish frame\n", framestart, frameend, framestep);
    return 0;
  }
  
  /*** Now see where we are with all this. ***/
  //numreadframes = frameend - framestart + 1;
  numreadframes = (long) floor ( (double) (frameend - framestart + 1) / framestep + .999 );
  
  /*** Allocate the memeory we need.  ***/
  /*** First, the data storage buffer. ***/
  //databuff = (float *)calloc((size_t) (numfileleads * numfileframes), sizeof(float));
  databuff = new float[numfileleads*numfileframes];

  if (databuff == NULL) {
    fprintf(stderr, "*** In ReadMatlabDataFile I cannot get enough" " dynamic memory to buffer the data\n");
    return 0;
  }
  
  /*** Now set up the memory for the potential data array.
    ***/
  
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    if (numreadframes > map3d_info.maxnumframes)
      map3d_info.maxnumframes = numreadframes;
    /*  numleads = pnt_end[displaysurfnum] - pnt_start[displaysurfnum] + 1; */
    numleads = map3d_geom[displaysurfnum].numpts;
    if ((surfdata = Surf_Data::AddASurfData(surfdata, displaysurfnum, numreadframes, numleads)) == NULL) {
      fprintf(stderr, "Failed to allocate memory for data file: ", datafilename);
      return (NULL);
    }
    
    /*** Set up some parameters that we know already. ***/
    Surf_Data* surf = &surfdata[displaysurfnum];
    
    surf->setUnits(localunits);
    surf->ts_start = framestart;
    surf->ts_end = frameend;
    surf->ts_sample_step = framestep;
    surf->ts_available_frames = numfileframes;
    surf->user_step = framestep;
    surf->userpotmin = sinputlist->potusermin;
    surf->userpotmax = sinputlist->potusermax;
    surf->potscale = potscale;
    surf->seriesnum = insurfnum;
    surf->numseries = numseries;
    
#pragma mark Matlab Fiducials
    /******************** Fiducials ***********************/
    if (!fids.isempty())
    {
      //printf("Start reading matlab fiducials\n");
      float qtime = 1, stime = 1, ttime = 1, pontime = 1, pofftime = 1, rpeaktime = 1;
      float tpeaktime = 1, stofftime = 1;
      int numfidsets = 0;
      numsurfnodes = map3d_geom[displaysurfnum].numpts;


      //set up global fids memory
      set<int> gfidIndices;
      long numgfids = 0;
      for(int numFidTypes = 0; numFidTypes < fids.getnumelements(); numFidTypes++){
        vector<double> values;
        fids.getfieldCI(numFidTypes,"value").getnumericarray(values);
        if (values.size() == 1) // then it's a globalfid
        {
          gfidIndices.insert(numFidTypes);
          numgfids++;
        }
      }
      
      long numsfids = fids.getnumelements() - numgfids;
      
	  surf->numglobalfids = numgfids;
	  if ((surf->globalfids = (float *)calloc((size_t)numgfids, sizeof(float))) == NULL)
	  {
		  ReportError("ReadMatlabDatafile", "error getting basic fid mem",
			  ERR_MEM, "");
		  return(NULL);
	  }

	  if ((surf->globalfidtypes = (short *)calloc((size_t)numgfids, sizeof(short))) == NULL)
	  {
		  ReportError("ReadMatlabDatafile", "error getting fidtypes mem",
			  ERR_MEM, "");
		  return(NULL);
	  }

      if (( surf->globalfidnames = (char**) 
           calloc( (size_t) numgfids, sizeof( char* ) )) == NULL )
      {
        ReportError( "ReadMatlabDatafile", 
                    "error getting fidnames memory", ERR_MEM, "" );
        return( NULL );
      }

      //set up series fids memory
      if (( surf->fids.leadfids = (Lead_Fids *) 
            calloc( (size_t)  numsurfnodes, sizeof( Lead_Fids ) )) == NULL )
      {
        ReportError( "ReadMatlabDatafile", 
                     "error getting leadfids memory", ERR_MEM, "" );
        return( NULL );
      }
      
      //load the seriesfids structure
      surf->fids.tsnum = insurfnum;
      surf->fids.pakfilenum = -1;
      surf->fids.numfidleads = numsurfnodes;
      surf->fids.numfidtypes = numsfids;
      
      //set up series fidnames memory
      if (( surf->fids.fidnames = (char**) 
            calloc( (size_t) numsfids, sizeof( char* ) )) == NULL )
      {
        ReportError( "ReadMatlabDatafile", 
                     "error getting fidnames memory", ERR_MEM, "" );
        return( NULL );
      }
      if (( surf->fids.fidtypes =
            (short *) calloc( (size_t) numsfids, sizeof( short )) ) == NULL )
      {
        ReportError( "ReadMatlabDatafile", "error getting fidtypes memory",
                     ERR_MEM, "" );
        return( NULL );
      }
      
      
      matlabarray value;
      matlabarray type;
      FI_Init(1);
      
      int sfidnum = 0;
      int gfidnum = 0;
      
      
      for(int numFidTypes = 0; numFidTypes < fids.getnumelements(); numFidTypes++){
        FiducialInfo* tmp = (FiducialInfo*) malloc(sizeof(FiducialInfo));
        
        type = fids.getfieldCI(numFidTypes,"type");
        vector<short> vtype;
        type.getnumericarray(vtype);
        
        tmp->type = vtype[0];

        char* name = NULL;
        if(FI_GetInfoByType(tmp)){
          if ((name = (char *)calloc((size_t) strlen(tmp->name)+1, sizeof(char))) == NULL) {
            ReportError("ReadMatlabDatafile", "error getting fidnames memory", ERR_MEM, "");
            return (NULL);
          }
          strcpy(name, tmp->name);
        }
        
        if (gfidIndices.find(numFidTypes) != gfidIndices.end())
        {
          vector<double> values;
          matlabarray value = fids.getfieldCI(numFidTypes,"value");
          value.getnumericarray(values);
          // values should be size 1 here, based on presence in the gfidIndices list
          // set the actual value
          surf->globalfids[gfidnum] = values[0];
		  surf->globalfidnames[gfidnum] = name;
		  surf->globalfidtypes[gfidnum] = tmp->type;

          gfidnum++;
        }
        else
        {
          surf->fids.fidtypes[sfidnum] = vtype[0];
          surf->fids.fidnames[sfidnum] = name;
          sfidnum++;
        }
      }

      if ( map3d_info.reportlevel > 1 ){
        printf(" Some global fidicials from the file\n");
        for (int i = 0; i < surf->numglobalfids; i++)
          printf(" %s = %f\n", surf->globalfidnames[i], surf->globalfids[i]);
      }
	  surf->SetupGlobalFids();

      for(long leadnum = 0; leadnum <numsurfnodes; leadnum++){
        index = map3d_geom[displaysurfnum].channels[leadnum];
        maxindex = numfileleads - 1;
        if (index > maxindex || index < 0) {
          fprintf(stderr, "*** In ReadMatlabDataFile(fiducials)\n"
                  "    For surface #%ld \n"
                  "    Data buffer index points to %ld "
                  "in data buffer\n" "    but max size is %ld\n", displaysurfnum + 1, index, maxindex);
          fprintf(stderr, " numberofnodes = %ld, nodenum = #%ld"
                  " channel = #%ld\n", numsurfnodes, nodenum + 1, map3d_geom[displaysurfnum].channels[nodenum] + 1);
          return (0);
        }

        //printf("index %d leadnum %d\n", index, leadnum);
        surf->fids.leadfids[leadnum].leadnum = leadnum;
        surf->fids.leadfids[leadnum].numfids = 0;
        vector<int> whichfid;

        for(int numFidTypes = 0; numFidTypes < fids.getnumelements(); numFidTypes++){
          if (gfidIndices.find(numFidTypes) != gfidIndices.end())
            continue; // don't count global fids
          value = fids.getfieldCI(numFidTypes,"value");
          vector<double> vvalue;
          value.getnumericarray(vvalue);
          if(index <= vvalue.size()){
            whichfid.push_back(numFidTypes);
            surf->fids.leadfids[leadnum].numfids++;
          }
        }
        //printf("Lead %d numfids %d\n", leadnum,surf->fids.leadfids[leadnum].numfids);
        
        if (( surf->fids.leadfids[leadnum].fidtypes =
              (short *) calloc( (size_t) surf->fids.leadfids[leadnum].numfids, sizeof( short )) ) == NULL )
        {
          ReportError( "ReadMatlabDatafile", "error getting fidtypes memory",
                       ERR_MEM, "" );
          return( NULL );
        }
        
        if (( surf->fids.leadfids[leadnum].fidvals =
              (float *) calloc( (size_t) surf->fids.leadfids[leadnum].numfids, sizeof( float )) )== NULL )
        {
          ReportError( "ReadMatlabDatafile", "error getting fidvals memory",
                       ERR_MEM, "" );
          return( NULL );
        }
        
        sfidnum = 0;
        for(int fidnum = 0; fidnum < whichfid.size(); fidnum++){
          int fid = whichfid[fidnum];
          value = fids.getfieldCI(fid,"value");
          type = fids.getfieldCI(fid,"type");
          vector<long> vtype;
          vector<double> vvalue;

          type.getnumericarray(vtype);
          value.getnumericarray(vvalue);
          
          //series fids
          surf->fids.leadfids[leadnum].fidtypes[fidnum] = vtype[0];
          surf->fids.leadfids[leadnum].fidvals[fidnum] = vvalue[leadnum];
          
//          printf("fid type %d fid value %f\n", surf->fids.leadfids[leadnum].fidtypes[fidnum],
//                 surf->fids.leadfids[leadnum].fidvals[fidnum]);
            
          //get min-max fids
          if( surf->fids.leadfids[leadnum].fidvals[sfidnum] > surf->fidmax)
            surf->fidmax =  surf->fids.leadfids[leadnum].fidvals[sfidnum];
          if( surf->fids.leadfids[leadnum].fidvals[sfidnum] < surf->fidmin)
            surf->fidmin =  surf->fids.leadfids[leadnum].fidvals[sfidnum];
          sfidnum++;
        }
      }
    }
      
  }
    
  /*** Series Label (used for display later) ***/
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    Surf_Data* surf = &surfdata[displaysurfnum];

    sprintf(cseries, "Series %ld: ", insurfnum + 1);
    strcpy(surf->label, cseries);
    sprintf(csurface, ": Surface %ld", surf->surfnum + 1);
    strcat(surf->label, clabel);
    strcat(surf->label, csurface);
    
    // short display name - put as series number if label is not in file
    if (strlen(clabel) == 0)
      sprintf(clabel, "%d", insurfnum+1);
    strcpy(surf->shortlabel, clabel);
    
    if (reportlevel > 2)
      fprintf(stderr, " For surface #%ld the label is %s\n",
              surf->surfnum + 1, surf->label);
    if (sinputlist->parent->datapathname)
      strcpy(surf->filepath, sinputlist->parent->datapathname);
    strcpy(surf->potfilename, sinputlist->potfilename);
  }
  
  /*** Get the data buffer. ***/
  if (!potvals.isempty()) potvals.getnumericarray(databuff, potvals.getnumelements());
  
  
  if (reportlevel > 2) {
    printf(" Back with the data in the buffer\n");
    printf(" #leads = %ld #frames = %ld \n", numfileleads, numfileframes);
    printf(" #frames selected to run from %ld to %ld\n", framestart, frameend);
  }
  
  /*** Now move the data from the buffer to the proper locations in the
    data array, using indirection via the channels array. 
    ***/
  
  maxindex = numfileleads * numfileframes -1;
  surfcount = 0;
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    Surf_Data* surf = &surfdata[displaysurfnum];
    numsurfnodes = map3d_geom[displaysurfnum].numpts;
    
    if (numfileleads < numsurfnodes)
    {
      fprintf(stderr, "*** MAP3D ERROR: In ReadMatlabDataFile\n"
        "    For surface #%ld \n"
        "    Num Leads (%ld) < Geom Points (%ld)\n", surf->surfnum + 1, numfileleads, numsurfnodes);

    }

    for (framenum = framestart, modelframenum = 0; framenum <= frameend;
         framenum += framestep, modelframenum++) {
      for (nodenum = 0; nodenum < numsurfnodes; nodenum++) {
        if (map3d_geom[displaysurfnum].channels[nodenum] > -1 && 
            map3d_geom[displaysurfnum].channels[nodenum] < numfileleads ) { // don't read higher nodes than your geom
          index = framenum * numfileleads + map3d_geom[displaysurfnum].channels[nodenum];
          if (index > maxindex || index < 0) {
            fprintf(stderr, "*** MAP3D ERROR: In ReadMatlabDataFile\n"
                    "    For surface #%ld \n"
                    "    Data buffer index points to %ld "
                    "in data buffer\n"
                    "    but max size is %ld\n",
                    surf->surfnum + 1, index, maxindex);
            fprintf(stderr, " framenum = %ld, numberofframes = %ld\n"
                    " numberofleads = %ld, channelnum = %ld\n"
                    " numberofnodes = %ld, nodenum = %ld\n",
                    framenum, numfileframes, numfileleads,
                    map3d_geom[displaysurfnum].channels[nodenum], numsurfnodes, nodenum);
            return (NULL);
          }
          surf->potvals[modelframenum][nodenum] = databuff[index] * potscale;
        }
        else {
          surf->potvals[modelframenum][nodenum] = UNUSED_DATA;
        }
      }
    }
    surfcount++;
  }
  
  /*** Clean up and get back to calling routine.  ***/
  
  free(databuff);
  return (surfdata);
  
  
}

/*======================================================================*/



long ReadMap3dFidfile(Surf_Input * sinputlist, Surf_Data * surfdata, long numsurfsread, long insurfnum)
{
  /***
Input:
  sinputlist	    structure of all the surface information we need
  surfdata	    the surfdata array for all the surfaces
  numsurfsread    number of surfaces for which we have geometry
  and hence, expect to find data.
  insurfnum	    surface number in which to start storing data
  fidfilename     the place to find the fids, a .fid file.
  
Output:	    surfdata gets updated
Return:	    fidinfo (allocated and set up here--updated laer.
                     
                     
                     ***********************************************************************/
  long error = 0;
  long reportlevel;
  long numfseriesinfile, index, maxindex, displaysurfnum;
  long nodenum, numsurfnodes;
  long surfstart, surfend;
  Series_Fids *infidseries = NULL;
  /*********************************************************************/
  reportlevel = map3d_info.reportlevel;
  if (reportlevel)
    fprintf(stderr, "Reading fid file %s\n", sinputlist->fidfilename);
  
  /*** Initialize the fid library ***/
  
  FI_Init(reportlevel);
  
  if (insurfnum < 0) {
    fprintf(stderr, "*** In ReadMap3dFidfile: Error since insurfnum = %ld\n" " Exit here\n", insurfnum);
    exit(ERR_MISC);
  }
  
  /*** See how many series are in the fid file. ***/
  
  numfseriesinfile = GetNumFidfileSeries(sinputlist->fidfilename);
  if (numfseriesinfile < 1) {
    fprintf(stderr, "*** In ReadMap3dFidfile: Error since no fids in file\n" " Exit here\n");
    return ERR_FILE;
  }
  
  /*** Now see if the numbers all match.  This will be primitive, but
    we can refine it later if need be. ***/
  
  if (sinputlist->fidseriesnum >= numfseriesinfile) {
    fprintf(stderr, "*** In ReadMap3dFidfile: Error since there are %ld"
            " fids in file\n" "*** But you want number %ld\n", numfseriesinfile, sinputlist->fidseriesnum + 1);
    return (ERR_MISC);
  }
  
  /*** Now get the series we want from the file. ***/
  
  infidseries = (Series_Fids *) calloc((size_t) 1, sizeof(Series_Fids));
  
  ReadFidfileSeries(reportlevel, sinputlist->fidfilename, sinputlist->fidseriesnum, 0, infidseries);
  
  /*** Now we have to sort these into the right order to match the 
    potentials, or at least to map to the right geometry nodes.
    Note that one input fid series gets parcelled out to a set of
    output fid series.
    ***/
  
  surfstart = insurfnum;
  surfend = insurfnum + numsurfsread - 1;
  maxindex = infidseries->numfidleads - 1;
  for (displaysurfnum = 0; displaysurfnum <= surfend - surfstart; displaysurfnum++) {
    Series_Fids *outfidseries = &surfdata[displaysurfnum].fids;
    numsurfnodes = map3d_geom[displaysurfnum].numpts;
    outfidseries->tsnum = infidseries->tsnum;
    outfidseries->pakfilenum = infidseries->pakfilenum;
    outfidseries->numfidtypes = infidseries->numfidtypes;
    CopyFidTypes(outfidseries, infidseries);
    CopyFidNames(outfidseries, infidseries);
    strcpy(outfidseries->fidlabel, infidseries->fidlabel);
    outfidseries->numfidleads = numsurfnodes;
    outfidseries->leadfids = (Lead_Fids *) calloc((size_t) numsurfnodes, sizeof(Lead_Fids));
    
    for (nodenum = 0; nodenum < numsurfnodes; nodenum++) {
      index = map3d_geom[displaysurfnum].channels[nodenum];
      if (index > maxindex || index < 0) {
        fprintf(stderr, "*** In ReadMap3dFidfile\n"
                "    For surface #%ld \n"
                "    Data buffer index points to %ld "
                "in data buffer\n" "    but max size is %ld\n", displaysurfnum + 1, index, maxindex);
        fprintf(stderr, " numberofnodes = %ld, nodenum = #%ld"
                " channel = #%ld\n", numsurfnodes, nodenum + 1, map3d_geom[displaysurfnum].channels[nodenum] + 1);
        return (0);
      }
      outfidseries->leadfids[nodenum].leadnum = nodenum;
      outfidseries->leadfids[nodenum].numfids = infidseries->leadfids[index].numfids;
      
      int numfidtypes = infidseries->leadfids[index].numfids;
      outfidseries->leadfids[nodenum].fidtypes = 
        (short *) calloc( (size_t) numfidtypes, sizeof( short ) );
      outfidseries->leadfids[nodenum].fidvals = 
        (float *) calloc( (size_t) numfidtypes, sizeof( float ) );
      
      for ( int fnum=0; fnum < numfidtypes; fnum++ ) {
        outfidseries->leadfids[nodenum].fidtypes[fnum] = infidseries->leadfids[nodenum].fidtypes[fnum];
        outfidseries->leadfids[nodenum].fidvals[fnum] = infidseries->leadfids[nodenum].fidvals[fnum];
      }
    }
  }
  
  /*** Clean up and get back to calling routine.  ***/
  
  free(infidseries);
  return (error);
}

/*======================================================================*/
long CopyFidTypes(Series_Fids * outfidseries, Series_Fids * infidseries)
{
  /*** Copy all the fid types from one struct to the other ***/
  
  long numfidtypes, fidnum, error = 0;
  /**********************************************************************/
  numfidtypes = infidseries->numfidtypes;
  outfidseries->fidtypes = (short *)calloc((size_t) numfidtypes, sizeof(short));
  for (fidnum = 0; fidnum < numfidtypes; fidnum++) {
    outfidseries->fidtypes[fidnum] = infidseries->fidtypes[fidnum];
  }
  return (error);
}

/*======================================================================*/
long CopyFidNames(Series_Fids * outfidseries, Series_Fids * infidseries)
{
  /*** Copy all the fid type names from one struct to the other ***/
  
  long numfidtypes, fidnum, error = 0;
  /**********************************************************************/
  numfidtypes = infidseries->numfidtypes;
  outfidseries->fidnames = (char **)calloc((size_t) numfidtypes, sizeof(char *));
  for (fidnum = 0; fidnum < numfidtypes; fidnum++) {
    if ((outfidseries->fidnames[fidnum] =
         (char *)calloc((size_t) strlen(infidseries->fidnames[fidnum]) + 1, sizeof(char))) == NULL) {
      ReportError("CopyFidNames", "error getting fidnames memory", ERR_MEM, "");
      return (ERR_MEM);
    }
    strcpy(outfidseries->fidnames[fidnum], infidseries->fidnames[fidnum]);
  }
  return (error);
}

long ReadMatlabGeomFile(Map3d_Geom* geom, long insurfnum)
{
  if ( map3d_info.reportlevel > 0 ) { 
    if (insurfnum == -1)
      fprintf(stderr, "Reading geom file %s with all surfaces\n", geom->basefilename);
    else
      fprintf(stderr, "Reading geom file %s, surface# %d\n", geom->basefilename, insurfnum);
  }
  
  matlabfile mf(geom->basefilename, "r");
  int numArrays = mf.getnummatlabarrays();
  if (numArrays != 1) {
    printf("%s - invalid file format\n", geom->basefilename);
    return 0;
  }
  
  matlabarray cell;
  
  matlabarray ma = mf.getmatlabarray(0);  // Get the first matlab array from the file
  string name = ma.getname();		// Get the name of this array
  int num = ma.getnumelements();
  if (insurfnum > num) {
    printf("%s does not contain %d surfaces\n", geom->basefilename, insurfnum);
    return 0;
  }
  
  int index = insurfnum > 0 ? insurfnum-1 : 0;
  
  matlabarray pts;
  matlabarray pts_mv;
  matlabarray fac;
  matlabarray tet;
  matlabarray seg;
  matlabarray cha;
  
  if (ma.isstruct()) {
    if (ma.isfieldCI("pts")) pts = ma.getfieldCI(index,"pts");
    if (ma.isfieldCI("pts_mv")) pts_mv = ma.getfieldCI(index,"pts_mv");
    if (ma.isfieldCI("node")) pts = ma.getfieldCI(index,"node");

    if (ma.isfieldCI("fac")) fac = ma.getfieldCI(index,"fac");
    if (ma.isfieldCI("face")) fac = ma.getfieldCI(index,"face");
    
    if (ma.isfieldCI("cell")) tet = ma.getfieldCI(index,"cell"); 
    if (ma.isfieldCI("tet")) tet = ma.getfieldCI(index,"tet");
    if (ma.isfieldCI("tetra")) tet = ma.getfieldCI(index,"tetra");
    
    if (ma.isfieldCI("channels")) cha = ma.getfieldCI(index,"channels");
    
    if (ma.isfieldCI("edge")) seg = ma.getfieldCI(index,"edge");
    if (ma.isfieldCI("seg")) seg = ma.getfieldCI(index,"seg");
  }
  else if (ma.iscell()) {
    
    cell = ma.getcell(index);
	
    if(cell.isstruct()) {
      if (cell.isfieldCI("pts")) pts = cell.getfieldCI(0,"pts");
      if (cell.isfieldCI("pts_mv")) pts_mv = cell.getfieldCI(0,"pts_mv");
      if (cell.isfieldCI("node")) pts = cell.getfieldCI(0,"node");
      
      if (cell.isfieldCI("fac")) fac = cell.getfieldCI(0,"fac");
      if (cell.isfieldCI("face")) fac = cell.getfieldCI(0,"face");
      
      if (cell.isfieldCI("cell")) tet = cell.getfieldCI(0,"cell"); 
      if (cell.isfieldCI("tet")) tet = cell.getfieldCI(0,"tet");
      if (cell.isfieldCI("tetra")) tet = cell.getfieldCI(0,"tetra");
      
      if (cell.isfieldCI("channels")) cha = cell.getfieldCI(0,"channels");
      
      if (cell.isfieldCI("edge")) seg = cell.getfieldCI(0,"edge");
      if (cell.isfieldCI("seg")) seg = cell.getfieldCI(0,"seg");
    }
	
    
  }
  
  /* BUILD IN SOME MORE ROBUSTNESS */
  /* if matrix define the other way around, just transpose the matrix
    also check if arrays have correct dimensions */

  int base = 1;

  if (!pts.isempty() && !pts_mv.isempty()) {
    printf("Cannot specify pts and pts_mv\n");
    return 0;
  }

  if (!pts.isempty()) {
    if ((pts.getm() != 3)&&(pts.getn()==3)) pts.transpose();
    if (pts.getm() != 3) {
      printf("Pts matrix must have dimensions 3xn\n");
      return 0;
    }
  }

  if (!pts_mv.isempty()) {
    if (((pts_mv.getm() % 3) != 0)&&((pts_mv.getn() % 3) == 0)) pts_mv.transpose();
    if (pts_mv.getm() % 3 != 0) {
      printf("Pts_mv matrix must have (multiple-of-3)xn - %dx%d provided\n", pts_mv.getm(), pts_mv.getn());
      return 0;
    }
  }

  if (!fac.isempty()) {
    if ((fac.getm() != 3)&&(fac.getn()==3)) fac.transpose();
    if (fac.getm() != 3) {
      printf("Fac matrix must have dimensions 3xn\n");
      return 0;
    }
  }
  
  if (!tet.isempty()) {
    if ((tet.getm() != 4)&&(tet.getn()==4)) tet.transpose();
    if (tet.getm() != 4) {
      printf("Tetra matrix must have dimensions 4xn\n");
      return 0;
    }
  }
  
  if (!seg.isempty()) {
    if ((seg.getm() != 2)&&(seg.getn()==2)) seg.transpose();
    if (seg.getm() != 2) {
      printf("Seg matrix must have dimensions 2xn\n");
      return 0;
    }
  }
  
  int num_element_types = 0;
  if (!fac.isempty()) num_element_types++;
  if (!seg.isempty()) num_element_types++;
  if (!tet.isempty()) num_element_types++;

  if (num_element_types > 1 || num_element_types == 0) {
    printf("Must only specify one of fac (%sspeficied), seg (%sspecified), or tetra (%sspecified)\n", (fac.isempty() ? "not " : ""), (seg.isempty() ? "not " : ""), (tet.isempty() ? "not " : ""));
    return 0;
  }

  if (!pts.isempty()) {
    geom->numpts = pts.getnumelements()/3;
    geom->points[geom->geom_index] = Alloc_fmatrix(geom->numpts, 3);
    float** points = geom->points[geom->geom_index];
    float* elements = new float[geom->numpts*3];
    pts.getnumericarray(elements, geom->numpts*3);
    for (int i = 0; i < geom->numpts; i++) {
      points[i][0] = elements[i*3+0];
      points[i][1] = elements[i*3+1];
      points[i][2] = elements[i*3+2];
    }
    delete[] elements;
    
  }
  else if (!pts_mv.isempty()) {
    // m is # columns, n is # nodes.  Each group of 3 columns is an x,y,z point
    int numcols = pts_mv.getm();
    geom->numpts = pts_mv.getn();
    int num_timesteps = numcols/3;
    printf("Reading time-based point array, m(cols)=%d, n(pts)=%d (timesteps=%d)\n", numcols, geom->numpts, num_timesteps);

    geom->points.resize(num_timesteps);

    float* elements = new float[geom->numpts*numcols];
    pts_mv.getnumericarray(elements, geom->numpts*numcols);

    for (int timestep = 0; timestep < num_timesteps; timestep++) {
      geom->points[timestep] = Alloc_fmatrix(geom->numpts, 3);
      float** points = geom->points[timestep];
      for (int i = 0; i < geom->numpts; i++) {
        for (int j = 0; j < 3; j++) {
          points[i][j] = elements[i*numcols + timestep*3 + j];
        }
      }
    }
    delete[] elements;
  }
  else {
    printf("Empty or non-existent pts array\n");
    return 0;
  }

  int pts_per_element = 0;
  matlabarray* element_array = NULL;
  if (!fac.isempty()) {
    pts_per_element = 3;
    element_array = &fac;
  }
  else if (!seg.isempty()) {
    pts_per_element = 2;
    element_array = &seg;
  }
  else if (!tet.isempty()) {
    pts_per_element = 4;
    element_array = &tet;
  }
  else {
    printf("Non-existent fac, seg, or tet array\n");
    return 0;
  }

  geom->SetupMap3dSurfElements(element_array->getn(), pts_per_element);
  long* elements = new long[geom->numelements*pts_per_element];
  element_array->getnumericarray(elements, geom->numelements*pts_per_element);

  // allow for 0-based files if there is a zero in the file
  for (int i = 0; i < geom->numelements; i++) {
    for (int j = 0; j < pts_per_element; j++) {
      if (elements[i*pts_per_element+j] == 0) {
        base = 0;
      }
    }
  }

  for (int i = 0; i < geom->numelements; i++) {
    for (int j = 0; j < pts_per_element; j++) {
      geom->elements[i][j] = elements[i*pts_per_element+j]-base;
    }
  }

  delete[] elements;
    
  geom->channels = (long*) calloc(sizeof(long), geom->numpts);
  if (!cha.isempty()) {
    long* elements = new long[geom->numpts];
    int numchannels = cha.getnumelements();
    cha.getnumericarray(elements, numchannels);
    
    // make sure we don't seg fault if there are more channels in the file
    // than there are points
    for (int i = 0; i < numchannels && i < geom->numpts; i++) {
      geom->channels[i] = elements[i] -1;
    }
    
    // initialize the rest of the channels if there aren't enough 
    // channels in the file
    for (int i = numchannels; i < geom->numpts; i++) {
      geom->channels[i] = -1;
    }
    delete[] elements;
  }
  else {
    for (int i = 0; i < geom->numpts; i++) {
      geom->channels[i] = i;
    }
  }
  return 1;
}

long ReadLandmarkGeomFile(Map3d_Geom* onemap3dgeom)
{
  if (map3d_info.reportlevel)
    fprintf(stderr, "Reading landmark geom file %s\n", onemap3dgeom->basefilename);
  Land_Mark* lm = DefALandMarkSurf(onemap3dgeom->surfnum);
  ReadLandMarkFile(onemap3dgeom->basefilename, lm, map3d_info.reportlevel);
  
  // figure out how many points to create
  
  const int slices = 8;
  int connections = 0;
  for (int i = 0; i < lm->numsegs; i++) {
    connections += lm->segs[i].numpts - 1;
  }
  
  // double the non-end points
  onemap3dgeom->numpts = connections*slices + lm->numsegs*slices;
  onemap3dgeom->points[onemap3dgeom->geom_index] = Alloc_fmatrix(onemap3dgeom->numpts, 3);
  // two triangles per slice, try to connect the nodes with another set
  onemap3dgeom->SetupMap3dSurfElements(connections*slices*2, 3);
  
  onemap3dgeom->channels = (long*) calloc(sizeof(long), onemap3dgeom->numpts);
  
  int ptnum = 0;
  int channelnum = 0;
  int trinum = 0;
  
  // calculate these here to not have to create the same points 
  // a million times
  float circle_points[slices][3];
  for (int i = 0; i < slices; i++) {
    float radian = 6.2831853f*(((float)i)/slices);
    circle_points[i][0] = cos(radian);
    circle_points[i][1] = sin(radian);
    circle_points[i][2] = 0;
  }
  
  // normal vector of circle for the points
  float vec1[3] = {0,1,0};  //default for only 1 point in the seg  
  float vec2[3];
  float avgvec[3];
  float matrix[9];
  float seg_top[slices][3];
  float seg_bot[slices][3];
  
  for (int i = 0; i < lm->numsegs; i++) {
    LandMarkSeg seg = lm->segs[i];
    for (int j = 0; j < seg.numpts - 1; j++) {
      float *pt = seg.pts[j];
      float *nextpt = seg.pts[j+1];
      float rad = seg.rad[j];
      float nextrad = seg.rad[j+1];
      
      // first segment
      if (j == 0) {
        // compute the top points of the segment, and rotate.
        // the next iterations will use these points as the bottom
        // points
        vec1[0] = nextpt[0] - pt[0];
        vec1[1] = nextpt[1] - pt[1];
        vec1[2] = nextpt[2] - pt[2];
        if (vectorLength(vec1) < .00001) break;
        
        float sin_alpha, cos_alpha;
        float cross[3]; // pseudo-cross product
                        // yz length
        float vecprojYZ = sqrt(vec1[1]*vec1[1] + vec1[2]*vec1[2]);
        if (vecprojYZ > .00001) {
          sin_alpha = vec1[1] / vecprojYZ;
          cos_alpha = vec1[2] / vecprojYZ;
        }
        else {
          sin_alpha = 0;
          cos_alpha = 1;
        }
        cross[0] = vec1[0];
        cross[1] = vec1[1] * cos_alpha - vec1[2] * sin_alpha;
        cross[2] = vec1[Y] * sin_alpha + vec1[Z] * cos_alpha;
        float cross_length = vectorLength(cross);
        float sin_beta = cross[0] / cross_length;
        float cos_beta = cross[2] / cross_length;
        
        matrix[0] = cos_beta;
        matrix[1] = -sin_alpha*sin_beta;
        matrix[2] = -sin_beta*cos_alpha;
        matrix[3] = 0;
        matrix[4] = cos_alpha;
        matrix[5] = -sin_alpha;
        matrix[6] = -sin_beta;
        matrix[7] = sin_alpha*cos_beta;
        matrix[8] = cos_alpha*cos_beta;
        
        float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
        for (int k = 0; k < slices; k++) {
          float newpt[3];
          MultMatrix9x3(matrix, circle_points[k], newpt);
          seg_top[k][0] = pt[0] + newpt[0]*rad;
          seg_top[k][1] = pt[1] + newpt[1]*rad;
          seg_top[k][2] = pt[2] + newpt[2]*rad;
          pts[ptnum+k][0] = seg_top[k][0];
          pts[ptnum+k][1] = seg_top[k][1];
          pts[ptnum+k][2] = seg_top[k][2];
          onemap3dgeom->channels[ptnum+k] = channelnum;
        }
        
        ptnum += slices;
        channelnum++;
        // if there are >2 points
        if (seg.numpts > 2) {
          vec2[0] = seg.pts[2][0] - seg.pts[1][0];
          vec2[1] = seg.pts[2][1] - seg.pts[1][1];
          vec2[2] = seg.pts[2][2] - seg.pts[1][2];
        }
        else {
          vec2[0] = vec1[0];
          vec2[1] = vec1[1];
          vec2[2] = vec1[2];
        }
        avgvec[0] = (vec1[0] + vec2[0]) / 2.0f;
        avgvec[1] = (vec1[1] + vec2[1]) / 2.0f;
        avgvec[2] = (vec1[2] + vec2[2]) / 2.0f;
      }
      // any in-between segment 
      else if (j > 0 && j < seg.numpts-2) {
        float* nextnextpt = seg.pts[j+2];
        for (int k = 0; k < slices; k++) {
          seg_top[k][0] = seg_bot[k][0];
          seg_top[k][1] = seg_bot[k][1];
          seg_top[k][2] = seg_bot[k][2];
        }
        vec1[0] = vec2[0];
        vec1[1] = vec2[1];
        vec1[2] = vec2[2];
        
        vec2[0] = nextnextpt[0] - nextpt[0];
        vec2[1] = nextnextpt[1] - nextpt[1];
        vec2[2] = nextnextpt[2] - nextpt[2];
        avgvec[0] = (vec1[0] + vec2[0]) / 2.0f;
        avgvec[1] = (vec1[1] + vec2[1]) / 2.0f;
        avgvec[2] = (vec1[2] + vec2[2]) / 2.0f;
      }
      // last segment
      else if (j == seg.numpts-2) {
        for (int k = 0; k < slices; k++) {
          seg_top[k][0] = seg_bot[k][0];
          seg_top[k][1] = seg_bot[k][1];
          seg_top[k][2] = seg_bot[k][2];
        }
        vec1[0] = vec2[0];
        vec1[1] = vec2[1];
        vec1[2] = vec2[2];
        avgvec[0] = vec2[0] = nextpt[0]-pt[0];
        avgvec[1] = vec2[1] = nextpt[1]-pt[1];
        avgvec[2] = vec2[2] = nextpt[2]-pt[2];
      }
      
      // prepare the next point
      float sin_alpha, cos_alpha;
      float cross[3]; // pseudo-cross product
                      // yz length
      float vecprojYZ = sqrt(avgvec[1]*avgvec[1] + avgvec[2]*avgvec[2]);
      if (vecprojYZ > .00001) {
        sin_alpha = avgvec[1] / vecprojYZ;
        cos_alpha = avgvec[2] / vecprojYZ;
      }
      else {
        sin_alpha = 0;
        cos_alpha = 1;
      }
      cross[0] = avgvec[0];
      cross[1] = avgvec[1] * cos_alpha - avgvec[2] * sin_alpha;
      cross[2] = avgvec[Y] * sin_alpha + avgvec[Z] * cos_alpha;
      float cross_length = vectorLength(cross);
      float sin_beta = cross[0] / cross_length;
      float cos_beta = cross[2] / cross_length;
      
      matrix[0] = cos_beta;
      matrix[1] = -sin_alpha*sin_beta;
      matrix[2] = -sin_beta*cos_alpha;
      matrix[3] = 0;
      matrix[4] = cos_alpha;
      matrix[5] = -sin_alpha;
      matrix[6] = -sin_beta;
      matrix[7] = sin_alpha*cos_beta;
      matrix[8] = cos_alpha*cos_beta;
      
      float circ_bottom[slices][3];
      for (int k = 0; k < slices; k++) {
        float newpt[3];
        MultMatrix9x3(matrix, circle_points[k], newpt);
        circ_bottom[k][0] = nextpt[0] + newpt[0]*nextrad;
        circ_bottom[k][1] = nextpt[1] + newpt[1]*nextrad;
        circ_bottom[k][2] = nextpt[2] + newpt[2]*nextrad;
      }
      
      // find the best (nontwisted) configuration of points
      // vec1 SHOULD be between this point and the next...
      int bestconfig = 0;
#if 0
      float dist = vectorLength(vec1)/2;
      float mincircum = 1.E10;
      float segvec[slices][3];
      float midp[slices][3];
      for(int k=0; k < slices; k++ )
      {
        int pnum = k;
        float circum = 0;
        for( int l=0; l<slices; l++ )
        {
          segvec[l][0] = circ_bottom[pnum][0] - seg_top[l][0];
          segvec[l][1] = circ_bottom[pnum][1] - seg_top[l][1];
          segvec[l][2] = circ_bottom[pnum][2] - seg_top[l][2];
          float segvecmag = vectorLength(segvec[j]);
          midp[l][0] = segvec[l][0] / segvecmag * dist;
          midp[l][1] = segvec[l][1] / segvecmag * dist;
          midp[l][2] = segvec[l][2] / segvecmag * dist;
          pnum++;
          if ( pnum >= slices) pnum = 0;
          if (l > 0) {
            float temp[3] = {midp[l][0] - midp[l-1][0], midp[l][1] - midp[l-1][1],midp[l][2] - midp[l-1][2]};
            circum += vectorLength(temp);
          }
        }
        float temp[3] = {midp[0][0] - midp[slices-1][0], midp[0][1] - midp[slices-1][1],midp[0][2] - midp[slices-1][2]};
        circum +=  vectorLength(temp);
        if ( circum < mincircum )
        {
          mincircum = circum;
          bestconfig = k;
        }
      }
#endif 
      float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
      for(int k=0; k<slices; k++ )
      {
        seg_bot[k][0] = circ_bottom[(bestconfig+k)%slices][0];
        seg_bot[k][1] = circ_bottom[(bestconfig+k)%slices][1];
        seg_bot[k][2] = circ_bottom[(bestconfig+k)%slices][2];
        pts[ptnum+k][0] = seg_bot[k][0];
        pts[ptnum+k][1] = seg_bot[k][1];
        pts[ptnum+k][2] = seg_bot[k][2];
        onemap3dgeom->channels[ptnum+k] = channelnum;
        
        onemap3dgeom->elements[trinum][0] = ptnum+k;
        onemap3dgeom->elements[trinum][1] = ptnum+((k+1)%slices);
        onemap3dgeom->elements[trinum][2] = ptnum+k-slices;
        onemap3dgeom->elements[trinum+1][0] = ptnum+k-slices;
        onemap3dgeom->elements[trinum+1][1] = ptnum+((k+1)%slices)-slices;
        onemap3dgeom->elements[trinum+1][2] = ptnum+((k+1)%slices);
        trinum += 2;
      }
      
      // increment here so we can refer to the correct point in the triangles
      ptnum += slices;
      channelnum++;
    }
  }
  return 0;
}


void WriteMatlabGeomFile(char * filename, vector<Map3d_Geom *> geoms)
{
  matlabarray data;
  vector<long> dims(2);
  dims[0] = geoms.size();
  dims[1] = 1;
  // create a cell array so we can store different stuff (if necessary) for each geom
  data.createcellarray(dims);

  for (unsigned i = 0; i < geoms.size(); i++) {
    Map3d_Geom* geom = geoms[i];
    matlabarray cell;
    cell.createstructarray();

    cell.addfieldname("pts");
    matlabarray pts;
    pts.createdensearray(3, geom->numpts, matlabarray::miDOUBLE);
    // TODO - save all geom indices
    pts.setnumericarray(geom->points[geom->geom_index], 3, geom->numpts);
    cell.setfield(0, "pts", pts);
    if (geom->elements) {
      // tri case
      long** offset = Alloc_lmatrix(geom->numelements, 3);
      for (int j = 0; j < geom->numelements; j++)
        for (int k = 0; k < 3; k++)
          offset[j][k] = geom->elements[j][k]+1; // make it one-based
      cell.addfieldname("fac");
      matlabarray fac;
      fac.createdensearray(3, geom->numelements, matlabarray::miINT32);
      fac.setnumericarray(offset, 3, geom->numelements);
      cell.setfield(0, "fac", fac);
      Free_lmatrix(offset, geom->numelements);
    }
    else if (geom->elementsize == 2) {
      // seg case
      long** offset = Alloc_lmatrix(geom->numelements, 2);
      for (int j = 0; j < geom->numelements; j++)
        for (int k = 0; k < 2; k++)
          offset[j][k] = geom->elements[j][k]+1; // make it one-based
      cell.addfieldname("seg");
      matlabarray seg;
      seg.createdensearray(2, geom->numelements, matlabarray::miINT32);
      seg.setnumericarray(offset, 2, geom->numelements);
      cell.setfield(0, "seg", seg);
      Free_lmatrix(offset, geom->numelements);
    }
    else if (geom->elements) {
      // tetra case
      long** offset = Alloc_lmatrix(geom->numelements, 4);
      for (int j = 0; j < geom->numelements; j++)
        for (int k = 0; k < 4; k++)
          offset[j][k] = geom->elements[j][k]+1; // make it one-based
      cell.addfieldname("tetra");
      matlabarray tet;
      tet.createdensearray(4, geom->numelements, matlabarray::miINT32);
      tet.setnumericarray(offset, 4, geom->numelements);
      cell.setfield(0, "tetra", tet);
      Free_lmatrix(offset, geom->numelements);
    }
    if (geom->channels) {
      // if there are channels
      long* offset = new long[geom->numpts];
      for (int j = 0; j < geom->numpts; j++)
        offset[j] = geom->channels[j]+1; // make it one-based
      cell.addfieldname("channels");
      matlabarray chan;
      chan.createdensearray(1, geom->numpts, matlabarray::miINT32);
      chan.setnumericarray(offset, geom->numpts);
      cell.setfield(0, "channels", chan);
      delete [] offset;
    }
    data.setcell(i, cell);
  }

  matlabfile file(filename, "w");
  file.putmatlabarray(data, "geom");
}

Surf_Data *ReadPotFiles(Surf_Input * sinputlist, Surf_Data * surfdata,
                        Map3d_Geom * map3dgeom, long numsurfsread, long insurfnum)
{
  /*** Read a set of pot files for a single entry surface in map3d.
  The idea is that for each entry surface there can be more than
  one display surface, which is all set up so that a geometry
  file can have more then one surface in it, i.e. a single entry
  surface has info for a single .geom file, which can have more
  than one surface in it.  Then we get more display surfaces than
  input or entry surfaces.
  
  Here we read one pot file, or one set of pot files, for each
  display surface.  This is a weird way to use pot files
  but is legal if we either use the same set of pots over and over 
  again, or have channels files that select portions of the pot file
  data.
  
Input:
  sinputlist   	the structure of information for this input
  or entry surface. We get lots of stuff from here.
  surfdata	    	the surfdata array for all the surfaces
  numsurfsread 	the number of display surfaces that were read for 
  this entry surface.  If this is more than 1,
  we have to re-read the same pot files
  insurfnum	        number of display surfaces at which to start
  loading new potentials.
  
  ***/
  long surfcount;
  long displaysurfnum; /*** Surface into which we place data. ***/
  long framenum, numpotsurfs;
  long numframes, numleads;
  long filenum, filenum1, filenum2, filestep;
  long *channels_p;  /*** Pointer to one surface worth of channels ***/
  char filename[200], basefilename[200];
  char datafilename[256];
  float potscale;
  /********************************************************************/
  
  
  filenum1 = sinputlist->ts_start;
  filenum2 = sinputlist->ts_end;
  filestep = sinputlist->ts_sample_step;
  
  
  if (filenum2 < filenum1)
    filenum2 = filenum1;
  if (filestep < 0)
    filestep = 1;
  strcpy(basefilename, sinputlist->potfilename);
  StripExtension(basefilename);
  if (filenum2 > filenum1)
    basefilename[strlen(basefilename) - 3] = 0;

  if (map3d_info.reportlevel > 0) {
    if (filenum2 > filenum1) {
      fprintf(stderr, "Reading pot files starting with %s%-3.3ld\n", basefilename, filenum1+1);
      printf("   (basefilename is %s from %s)\n", basefilename, sinputlist->potfilename);
    }
    else {
      printf("Reading pot file: %s\n", sinputlist->potfilename);
    }
  }

  potscale = sinputlist->potscale;
  numpotsurfs = 0;
  
  /*** Loop through all the surfaces read for this entry surface. 
    ***/
  
  for (surfcount = 0; surfcount < numsurfsread; surfcount++) {
    
    displaysurfnum = insurfnum + surfcount;
    numleads = map3dgeom[displaysurfnum].numpts;
    channels_p = map3dgeom[displaysurfnum].channels;
    
    /*** Set up the sequence of files that we want to read in for this
      surface.
      ***/
    framenum = 0;
    if (map3d_info.reportlevel > 1)
      fprintf(stderr, " Loop of pot files for display surface %ld "
              "from %ld to %ld step %ld\n", displaysurfnum + 1, filenum1 + 1, filenum2 + 1, filestep);
    for (filenum = filenum1; filenum <= filenum2; filenum+=filestep) {
      
      /*** Make the .pot filename and open the file. ***/
      
      if (filenum2 > filenum1) {
        sprintf(filename, "%s%-3.3ld", basefilename, filenum + 1);
      }
      else {
        strcpy(filename, basefilename);
      }
      strcat(filename, ".pot");
      
      if (!getFileWithPath(filename,getenv("MAP3D_DATAPATH"),sinputlist->parent->datapathname,datafilename))
        return surfdata;
      
          /*** Now go read the file into this surface's arrays.  ***/     
      /*** Allocate memory for the new surface of data in the surfdata array. ***/
      
      if (filenum == filenum1) {
        numframes = (long) floor ( (double) (filenum2-filenum1 + 1) / filestep + .999 );
        //numframes = (long)((filenum2 - filenum1) + 1);
        if (numframes > map3d_info.maxnumframes)
          map3d_info.maxnumframes = numframes;
        if ((surfdata = Surf_Data::AddASurfData(surfdata, displaysurfnum, numframes, numleads)) == NULL)
          return (NULL);
        surfdata->ts_end = filenum2;
        surfdata->ts_start = filenum1;
        surfdata->ts_sample_step = filestep;
        surfdata->ts_available_frames = numframes;
        
      }
      
      if (map3d_info.reportlevel > 1)
        fprintf(stderr, " Reading potentials from \n" "file: %s\n", filename);
      FILE* luin_p = fopen(datafilename,"r");
      ReadOnePotFile(luin_p, potscale, &surfdata[displaysurfnum], framenum, channels_p);
      fclose(luin_p);
      framenum++;
    }
    
    numpotsurfs++;
    if (sinputlist->parent->datapathname && strlen(sinputlist->parent->datapathname) > 1)
      strcpy(surfdata[displaysurfnum].filepath, sinputlist->parent->datapathname);
    strcpy(surfdata[displaysurfnum].potfilename, sinputlist->potfilename);
  }
  return surfdata;
}

/*================================================================*/
void ReadOnePotFile(FILE * luin_p, float potscale, Surf_Data * onesurfdata, long lframenum, long *onechannels)
/*** Read the potentials values from the already open file 
Dynamic allocation version.

Input:
potscale	      float value multiplied by each value in the file
as it is read in.
onesurfdata     one structure to load up with data
lsurfnum        local surface number
onechannels     pointer to channels for this surface.

***/
{
  long i;
  long npotread, ninpots, index;
  long numleads, nfields;
  float *potbuff;   /*** Buffer to hold a set of potentials ***/
  /******************************************************************/
  
  numleads = onesurfdata->numleads;
  
  /*** Set up the buffer used to store one set of potentials. ***/
  
  ninpots = GetFileLength(luin_p);
  rewind(luin_p);
  
  if ((potbuff = (float *)calloc((size_t) ninpots, sizeof(float))) == NULL) {
    fprintf(stderr, "*** In ReadOnePotFile error getting memory"
            " for pot data\n" "    in frame #%ld\n", lframenum + 1);
    fprintf(stderr, "   num_values = %ld\n", ninpots);
    exit(ERR_MEM);
  }
  
  /*** Read the data finally, first into a buffer.  ***/
  
  npotread = 0;
  for (i = 0; i < ninpots; i++) {
    if ((nfields = fscanf(luin_p, "%f\n", &potbuff[i])) != 1) {
      fprintf(stderr, "*** In ReadOnePotFile problems because the file\n"
              "     yielded wrong number of fields or less"
              " data than expected\n"
              "     We expect %ld values but are only at %ld\n"
              "     and have a line with %ld fields\n", ninpots, npotread + 1, nfields);
      exit(ERR_FILE);
    }
    npotread++;
  }
  
  /*** Now transfer the data to the final location, using the channels
    buffer to redirect the effort. ***/
  
  for (i = 0; i < numleads; i++) {
    index = onechannels[i];
    if (index < 0) {
      /*** Mark as unused data ***/
      onesurfdata->potvals[lframenum][i] = UNUSED_DATA;
      
      /*** Check if we have valid index pointer. ***/
      
    }
    else if (index > ninpots - 1) {
      fprintf(stderr, "*** In ReadOnePotFile  we have an invalid"
              " redirection\n"
              "    The value of the channel index is %ld but"
              " we have %ld values\n"
              "    The index must always be less than the number" " of values in the file\n", index, ninpots);
      exit(ERR_MEM);
    }
    else {
      onesurfdata->potvals[lframenum][i] = potbuff[index] * potscale;
    }
  }
  
  /*** Free up the space needed for the buffer. ***/
  
  free((float *)potbuff);
  potbuff = NULL;
  
  /*** Since pot files don't have units, set a 0 in theunits. ***/
  
  onesurfdata->units = 0;
  
  if (map3d_info.reportlevel > 3) {
    fprintf(stderr, " Potentials for frame %ld read\n", lframenum);
    fprintf(stderr, " First values are\n");
    for (i = 0; i < cjmin(ninpots, 10); i++)
      fprintf(stderr, " %ld  %f\n", i, onesurfdata->potvals[lframenum][i]);
  }
}



void ReadChannelsFile(FILE * luin_p, Map3d_Geom * onemap3dgeom)
/*** Read the channels file, which is already open.

The channels file contains a column which links the nodes of the  
geometry with entries in the data file.  The 'channels'
array points, for each node, to the number of the location in the
data array where the data associated with that node is located.
This is used mostly with multiplexed data that contains data that
is either stored in a different order to the points in the geometry
to which it is associated, or contains multiple surfaces of data
which are not otherwise divided.  Examples of the latter are the 
data files that come from the tank since there are not real surfaces
in the tank (aside from the tank surface itself).  By setting up
'channel' arrays, we can functionally connect data values to match
the connections made by linking the points in triangles.

***/
{
  long i, j, nfnodes, nptssurf, nodenum;
  long pointnum;
  long surfnum;
  char instring[100];
  /******************************************************************/
  
  /*** Read the first line and make sure we are OK with this file. ***/
  surfnum = onemap3dgeom->surfnum;
  nptssurf = onemap3dgeom->numpts;
  fscanf(luin_p, "%ld %s \n", &nfnodes, instring);
  if (map3d_info.reportlevel > 1) {
    fprintf(stderr, "In ReadChannelFile for surface #%ld\n", surfnum + 1);
    fprintf(stderr, " First line read as %ld %s\n", nfnodes, instring);
  }
  if (nfnodes != nptssurf) {
    fprintf(stderr, "*** Warning: we have %ld nodes in this surface\n"
            "    but the channel files has info on %ld nodes\n", nptssurf, nfnodes);
  }
  
  /*** Make sure we have a place to put these ***/
  
  if (onemap3dgeom->channels == NULL) {
    printf("*** ReadChannelsFile: no space for channels\n");
    exit(ERR_MEM);
  }
  
  /*** Now go through and read all the channels entries, one per each
    line, and per node in the geometry. ***/
  
  int maxx = 0;

  for (nodenum = 0; nodenum < nfnodes; nodenum++) {
    if (fscanf(luin_p, "%ld %ld \n", &i, &j) != 2) {
      fprintf(stderr, "*** ReadChannelFile, error reading link #%ld\n", nodenum);
      exit(ERR_FILE);
    }
    
    if (nodenum == 312){
      int a;
      a = 5;
    }

    if (i - 1 != nodenum) {
      fprintf(stderr, "Node number read as %ld while counter" " says it should be %ld\n", i, nodenum + 1);
      exit(ERR_FILE);
    }
    onemap3dgeom->channels[nodenum] = j - 1;
    
    if (onemap3dgeom->channels[nodenum] > maxx) {
      maxx = onemap3dgeom->channels[nodenum];
    }

    /*** Make sure we have a legal point being referred to in the channels ***/
    // note: map3d allows '0' or '-1' entries to be "unused" nodes.
    
    if (onemap3dgeom->channels[nodenum] < -2) {
      fprintf(stderr, "*** Warning: in ReadChannelsFile\n" "    node %ld points to a data element < 0\n", nodenum);
    }
  }

  // if there are more points in the surf than the file, pad the rest to be 'unused'
  for (; nodenum < nptssurf; nodenum++)
    onemap3dgeom->channels[nodenum] = -1;
  
  /*** Now look for doubles or any other funky stuff. Don't worry about unused values though ***/
  
  for (nodenum = 0; nodenum < nfnodes; nodenum++) {
    pointnum = onemap3dgeom->channels[nodenum];
    for (i = nodenum + 1; i < nfnodes; i++) {
      if (onemap3dgeom->channels[i] == pointnum && pointnum >= 0) {
        fprintf(stderr, "+++ Warning in ReadChannelsFile\n"
                "    datapoint #%ld is pointed at by node #%ld" " and node #%ld\n", pointnum, nodenum, i);
      }
    }
  }
  return;
}

/*================================================================*/

long ReadChannelLinksFile(FILE * luin_p, Map3d_Geom * onemap3dgeom)
/*** Read the channellinks file, which is already open.

The purpose of the channelinks file is to mark subsets of leads by 
channel number.  So an entry in this file would be:

channelnum labelnum 

where channelnum indicates the channel that should be marked
with the value "labelnum".

Once read in, we have to scan the channelnums for their
associated points and set up the leadlinks array accordingly.

NOTE: TO WORK PROPERLY, WE MUST RUN THIS FUNCTION ONLY AFTER THE CHANNELS
INFORMATION IS AVAILABLE!!!!
***/
{
  long i, nclinks, nvalidlinks, noutleads, leadnum;
  long chlink;
  long pointnum;
  long *leadlinks = NULL;
  char instring[100];
  char chlabel[100];
  char **leadlinklabels = NULL;
  /******************************************************************/
  /*** Read in the file , setting up the channel links array.
    ***/
  
  fscanf(luin_p, "%ld %s \n", &nclinks, instring);
  fprintf(stderr, "In ReadChannelLinksFile\n");
  fprintf(stderr, " First line read as %ld %s\n", nclinks, instring);
  
  /*** Get some local memory for all the links. ***/
  
  if ((leadlinks = (long *)calloc((size_t) nclinks, sizeof(long))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for local leadlinks\n");
    return (ERR_MEM);
  }
  if ((leadlinklabels = (char **)calloc((size_t) nclinks, sizeof(char *))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting local memory" " for leadlinklabels\n");
    return (ERR_MEM);
  }
  
  /*** Now do the reading of the lead link file. ***/
  
  nvalidlinks = 0;
  for (leadnum = 0; leadnum < nclinks; leadnum++) {
    if (fscanf(luin_p, "%ld %s \n", &chlink, chlabel) != 2) {
      fprintf(stderr, "*** ReadChannelLinksFile: error reading" " link #%ld\n", leadnum + 1);
      exit(ERR_FILE);
    }
    
    /*** Load the values into the structures.  ***/
    
    leadlinks[leadnum] = ChanneltoNode(chlink, onemap3dgeom->channels, onemap3dgeom->numpts);
    if (leadlinks[leadnum] >= 0)
      nvalidlinks++;
    if ((leadlinklabels[leadnum] = (char *)calloc((size_t) LEADLABELSIZE + 1, sizeof(char))) == NULL) {
      fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for leadlinklabels lead #%ld\n", leadnum + 1);
      return (ERR_MEM);
    }
    if (strlen(chlabel) <= LEADLABELSIZE) {
      strcpy(leadlinklabels[leadnum], chlabel);
    }
    else {
      strncpy(leadlinklabels[leadnum], chlabel, LEADLABELSIZE);
      leadlinklabels[leadnum][LEADLABELSIZE] = '\0';
      printf("+++ ReadChannelLinksFile: terminated leadlink label\n");
    }
  }
  /*** Now look for doubles or any other funky stuff. ***/
  
  for (leadnum = 0; leadnum < nclinks; leadnum++) {
    pointnum = leadlinks[leadnum];
    if (pointnum >= 0) {
      for (i = leadnum + 1; i < nclinks; i++) {
        if (leadlinks[i] == pointnum) {
          fprintf(stderr, "+++ Warning in ReadChannelLinksFile\n"
                  "    point #%ld is pointed at by lead #%ld" " and lead #%ld\n", pointnum + 1, leadnum + 1, i + 1);
        }
      }
      strcpy(chlabel, leadlinklabels[leadnum]);
      for (i = leadnum + 1; i < nclinks; i++) {
        if (!strcmp(leadlinklabels[i], chlabel)) {
          fprintf(stderr, "+++ Warning in ReadChannelLinksFile\n"
                  "+++ leadlabel %s is found in entry #%ld"
                  " and entry #%ld of channel links file\n", chlabel, leadnum + 1, i + 1);
        }
      }
    }
  }
  
  /*** Now load the subset of links that were actually found in this
    surface and load just those into the data structure. ***/
  
  onemap3dgeom->numleadlinks = nvalidlinks;
  
  if ((onemap3dgeom->leadlinks = (long *)calloc((size_t) nvalidlinks, sizeof(long))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for leadlinks\n");
    return (ERR_MEM);
  }
  if ((onemap3dgeom->leadlinklabels = (char **)calloc((size_t) nvalidlinks, sizeof(char *))) == NULL) {
    fprintf(stderr, "*** ReadChannelLinksFile: error getting memory" " for leadlinklabels\n");
    return (ERR_MEM);
  }
  noutleads = 0;
  for (leadnum = 0; leadnum < nclinks; leadnum++) {
    pointnum = leadlinks[leadnum];
    if (pointnum >= 0) {
      onemap3dgeom->leadlinks[noutleads] = pointnum;
      if ((onemap3dgeom->leadlinklabels[noutleads] = (char *)
           calloc((size_t) strlen(leadlinklabels[leadnum]) + 1, sizeof(char))) == NULL) {
        fprintf(stderr, "*** ReadChannelLinksFile: error getting memory"
                " for leadlinklabels lead #%ld\n", leadnum + 1);
        return (ERR_MEM);
      }
      strcpy(onemap3dgeom->leadlinklabels[noutleads], leadlinklabels[leadnum]);
      noutleads++;
    }
  }
  
  free(leadlinklabels);
  free(leadlinks);
  
  if (noutleads != nvalidlinks) {
    printf("ReadChannelLinks error because noutleads = %ld and" " nvalidlinks = %ld\n", noutleads, nvalidlinks);
    return (ERR_FILE);
  }
  else {
    printf(" For this surface there are %ld valid channel links\n", nvalidlinks);
  }
  
  /*** If we get to here, we have leadlinks, so have then displayed ***/
  
  //    if ( map3d_info.reportlevel > 2 )
  printf(" Setting qshowleadlabels = true\n");
  
  //draw_info[onemap3dgeom->surfnum].qshowleadlabels = true;
  return 0;
}


/*================================================================*/
long ChanneltoNode(long chlink, long *channels, long numpts)
{
  /*** Convert a channel number  to its assoicated node number. ***/
  
  long nodenum;
  chlink--;
  for (nodenum = 0; nodenum < numpts; nodenum++) {
    if (chlink == channels[nodenum]) {
      return (nodenum);
    }
  }
  return (-1);
}

/*======================================================================*/

long NodetoLeadlink(long nodenum, Map3d_Geom * onemap3dgeom, char *leadlabel)
{
  
  /*** Convert a node number to a lead link number, if there is one
  for this node.
  
Input:
  nodenum	    number of the node we want to convert
  onemap3dgeom the complete geometry for this surface
  *leadlabel   poiner to the label that this lead has (NULL is no match)
  
Returns:
  the number of the leadlink that we found to match
  -1 if there is no match.
  ***/
  
  long linknum;
  /**********************************************************************/
  if (onemap3dgeom->numleadlinks <= 0) {
    leadlabel = NULL;
    return (-1);
  }
  
  for (linknum = 0; linknum < onemap3dgeom->numleadlinks; linknum++) {
    if (onemap3dgeom->leadlinks[linknum] == nodenum && linknum != nodenum) {
      leadlabel = onemap3dgeom->leadlinklabels[linknum];
      return (linknum);
    }
  }
  leadlabel = NULL;
  return (-1);
}

/*================================================================*/

long ReadLeadFile(FILE * luin_p, Map3d_Geom * onemap3dgeom)
/*** Read the leadlinkfile file, which is already open.

Note the use of the leadlinks array, which contains the pointers for all the
leads in each surface. So to see which point corresponds to a lead, simply
look the value up in leadlinks that corresponds to the lead number. 

The actual number assigned to each lead link is stored in a separate
array leadlinklabels, and this determines the labels applied to the
leads.  So lead #4 could point to node 343 and be labelled V6.
***/
{
  long i, nfleads, leadnum;
  long pointnum, leadlinknum;
  char instring[100], chlabel[100];
  char **leadlinklabels = NULL;
  int base = 1; // default is to do 1-based files, but if there is a 0, make it 0-based.
  /******************************************************************/
  /*** Read in the file , setting up the lead array.
    ***/
  
  fscanf(luin_p, "%ld %s \n", &nfleads, instring);
  fprintf(stderr, "In ReadLeadFile\n");
  fprintf(stderr, " First line read as %ld %s\n", nfleads, instring);
  
  /*** Get some memory for these links. ***/
  
  if ((onemap3dgeom->leadlinks = (long *)calloc((size_t) nfleads, sizeof(long))) == NULL) {
    fprintf(stderr, "*** ReadLeadFile: error getting memory" " for leadlinks\n");
    return (ERR_MEM);
  }
  if ((leadlinklabels = (char **)calloc((size_t) nfleads, sizeof(char *))) == NULL) {
    fprintf(stderr, "*** ReadLeadFile: error getting memory" " for leadlinklabels\n");
    return (ERR_MEM);
  }
  
  /*** Now do the reading of the lead link file. ***/
  
  for (leadnum = 0; leadnum < nfleads; leadnum++) {
    if (ReadLine(instring, luin_p) == NULL) {
      fprintf(stderr, "*** ReadLeadFile: error reading link #%ld\n", leadnum + 1);
      exit(ERR_FILE);
    }
    /*** Scan the line and check how many values are on each.  We expect
    two values for each line: the label of the leads, and the node
    it should be drawn at.  Eg., V1 234 ***/
    
    if (sscanf(instring, "%s %ld \n", chlabel, &leadlinknum) != 2) {
      fprintf(stderr, "*** ReadLeadFile: error scanning line\n %s\n", instring);
      exit(ERR_FILE);
    }
    
    /*** Load the values into the structures.  ***/
    
    if (leadlinknum == 0)
      base = 0;
    onemap3dgeom->leadlinks[leadnum] = leadlinknum - 1;
    if ((leadlinklabels[leadnum] = (char *)calloc((size_t) LEADLABELSIZE + 1, sizeof(char))) == NULL) {
      fprintf(stderr, "*** ReadLeadLinksFile: error getting memory" " for leadlinklabels lead #%ld\n", leadnum + 1);
      return (ERR_MEM);
    }
    if (strlen(chlabel) <= LEADLABELSIZE) {
      strcpy(leadlinklabels[leadnum], chlabel);
    }
    else {
      strncpy(leadlinklabels[leadnum], chlabel, LEADLABELSIZE);
      leadlinklabels[leadnum][LEADLABELSIZE] = '\0';
      printf("+++ ReadChannelLinksFile: terminated leadlink label\n");
    }
  }
  onemap3dgeom->numleadlinks = nfleads;
  
  /*** Now look for doubles or any other funky stuff. ***/
  
  for (leadnum = 0; leadnum < nfleads; leadnum++) {
    // add 1 to make the file 0-based (as we already subtracted 1) if it is 0-based.
    if (base == 0)
      onemap3dgeom->leadlinks[leadnum]++;
    
    /*** Make sure we have a legal point being referred to in the leadlink ***/
    if (onemap3dgeom->leadlinks[leadnum] > onemap3dgeom->numpts - 1) {
      fprintf(stderr, "+++Warning: in ReadLeadFile\n"
              "+++lead %ld points to a node that is\n"
              "+++larger than what has been read for this surface\n"
              "+++The node expected is %ld, while we have only read"
              " %ld nodes in\n"
              "+++ Setting leadlink pointers to -1\n", leadnum, onemap3dgeom->leadlinks[leadnum], onemap3dgeom->numpts);
      onemap3dgeom->leadlinks[leadnum] = -1;
    }
    
    /*** If there is a zero entry in the leadlink file, flag it by a -1
      for internal checking in the program.
      ***/
    
    else if (onemap3dgeom->leadlinks[leadnum] < 0) {
      onemap3dgeom->leadlinks[leadnum] = -1;
      strcpy(leadlinklabels[leadnum], "");
      fprintf(stderr, "***In ReadLeadFile\n" "   Leadlink #%ld points to a zero point number\n", leadnum + 1);
    }
    pointnum = onemap3dgeom->leadlinks[leadnum];
    if (pointnum >= 0) {
      for (i = leadnum + 1; i < nfleads; i++) {
        if (onemap3dgeom->leadlinks[i] == pointnum) {
          fprintf(stderr, "+++ Warning in ReadLeadFile\n"
                  "    point #%ld is pointed at by lead #%ld" " and lead #%ld\n", pointnum + 1, leadnum + 1, i + 1);
        }
      }
    }
    strcpy(chlabel, leadlinklabels[leadnum]);
    for (i = leadnum + 1; i < nfleads; i++) {
      if (!strcmp(leadlinklabels[i], chlabel)) {
        fprintf(stderr, "+++ Warning in ReadLeadFile\n"
                "+++ leadlebel %s is found in entry #%ld" " and entry #%ld of leadlinks\n", chlabel, leadnum + 1,
                i + 1);
      }
    }
  }
  
  /*** If we get to here, we have leadlinks, so have them displayed ***/
  
  onemap3dgeom->leadlinklabels = leadlinklabels;
  
  if (map3d_info.reportlevel > 2) {
    for (i = 1; i < cjmin(10, nfleads); i++) {
      printf(" Lead #%ld has label %s\n", onemap3dgeom->leadlinks[i], onemap3dgeom->leadlinklabels[i]);
    }
    fprintf(stderr, " Setting qshowleadlabels = true\n");
  }
  //    draw_info[onemap3dgeom->surfnum].qshowleadlabels = true;
  return 0;
}
