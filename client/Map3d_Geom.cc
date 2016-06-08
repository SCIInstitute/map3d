/* Map3d_Geom.cxx */

#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "landmarks.h"
#include "map3d-struct.h"
#include "map3dmath.h"
#include "geomlib.h"            //for OrderEnodes
#include <limits.h>
#include <stdlib.h>

extern Map3d_Info map3d_info;

Map3d_Geom::Map3d_Geom()
{
  init();
}
void Map3d_Geom::init()
{
  surfnum = 0;
  subsurf = 0;
  numpts = 0;
  numelements = 0;
  maxnumelts = 0;
  numleadlinks = 0;
  cubescale = 0.0f;

  // set up first set of points
  points.clear();
  points.push_back(0);
  elements = 0;
  geom_index = 0;
  channels = 0;
  leadlinks = 0;
  leadlinklabels = 0;
  label[0] = '\0';
  filepath[0] = '\0';
  basefilename[0] = '\0';

  ptnormals = 0;
  fcnormals = 0;
  adjacent_triangles = 0;
  landmarks = 0;

  xmin = ymin = zmin = 0;
  xmax = ymax = zmax = 0;
  xcenter = ycenter = zcenter = 0;
  l2norm = 0;
  modified = 0;
}

Map3d_Geom::~Map3d_Geom()
{
  destroy();
}

void Map3d_Geom::destroy()
{
  for (unsigned i = 0; i < points.size(); i++)
    if (points[i])
      Free_fmatrix(points[i], numpts);
  if (elements)
    Free_lmatrix(elements, numelements);
  if (channels)
    free(channels);
  if (leadlinks)
    free(leadlinks);
  if (leadlinklabels)
    free(leadlinklabels);

  // have a landmarks destructor called
  if (landmarks) {
    for (int i = 0; i < landmarks->numsegs; i++) {
      Free_fmatrix(landmarks->segs[i].pts, landmarks->segs[i].numpts);
      Free_cmatrix(landmarks->segs[i].labels, landmarks->segs[i].numpts);
      free(landmarks->segs[i].rad);
    }
    free(landmarks->segs);
    free(landmarks);
  }

  if (ptnormals)
    Free_fmatrix(ptnormals, numpts);
  if (fcnormals)
    Free_fmatrix(fcnormals, numelements);
  init();
}

long Map3d_Geom::SetupMap3dSurfElements(long numelts, long eltsize)
{
  if ((elements = Alloc_lmatrix(numelts, eltsize)) == NULL) {
    fprintf(stderr, "*** SetupMap3dSurfElements: error getting memory" " for elements\n");
    return (ERR_MEM);
  }
  numelements = numelts;
  elementsize = eltsize;
  return 0;
}


/*================================================================*/

int Map3d_Geom::CheckPointValidity()
/*** See whether all points are actually used in the triangles
or tetrahedra.***/
{
  // return val for function - return error in CheckElementPoints if there is a point out of range,
  // warnings in the other two if necessary
  int retval = GEOM_OK;

  if (numelements > 0) {
    retval = CheckElementPoints();
    if (retval == GEOM_ERROR)
      return GEOM_ERROR;
    if (!map3d_info.qnovalidity) {
      // make sure if one if these is warning we return warning
      retval = CheckElementValidity();
      int check = CheckElementDoubles();
      if (check < retval)
        retval = check;
    }
  }

#if 0
  // find another way to check this later.  Not being part of a polygon
  // does not make it 'invalid'
  if (map3d_info.reportlevel >= 3) {
    for (i = 0; i <= numpts; i++) {
      if (validpoints[i] == false) {
        fprintf(stderr, " Point #%ld " " is not part of any polygon\n", i + 1);
        retval = GEOM_WARNING;
      }
    }
  }
#endif
  return retval;
}

/*================================================================*/
int Map3d_Geom::CheckElementPoints()
{
  long i, j;
  /**********************************************************************/
  if (!elements)
    return GEOM_OK;
  for (i = 0; i < numelements; i++) {
    for (j = 0; j < elementsize; j++) {
      if (elements[i][j] >= numpts) {
        fprintf(stderr, "*** In CheckPointValidity"
                " error in elements #%ld of length %ld\n"
                "    The point %ld is beyond the last"
                " one in the dataset at %ld\n", i + 1, elementsize, elements[i][j] + 1, numpts);
        return GEOM_ERROR;
      }
      else if (elements[i][j] < 0) {
        fprintf(stderr, "*** In CheckPointValidity"
                " error in element #%ld of length %ld\n"
                "    The point #%ld = %ld is zero or less\n", i + 1, elementsize, j + 1, elements[i][j] + 1);
        return GEOM_ERROR;
      }
    }
  }
  return GEOM_OK;
}

/*================================================================*/

int Map3d_Geom::CheckElementDoubles()
{

  /*** See whether there are any doubled elements in the geometry file. ***/

  long i, j, k;
  long nmatches;
  long enodes[4], tenodes[4];
  char errstring[100];
  int retval = GEOM_OK;
  /**********************************************************************/
  if (!elements)
    return retval;

  for (i = 0; i < numelements; i++) {
    OrderEnodes(elements[i], elementsize, enodes);
    for (j = i + 1; j < numelements; j++) {
      OrderEnodes(elements[j], elementsize, tenodes);
      nmatches = 0;
      for (k = 0; k < elementsize; k++) {
        if (enodes[k] == tenodes[k])
          nmatches++;
      }
      if (nmatches == elementsize) {
        sprintf(errstring, "In surface %ld, numbers %ld and %ld", surfnum + 1, i + 1, j + 1);
        ReportError("CheckDoubleElements", "found two matching elements", 0, errstring);
        retval = GEOM_WARNING;
      }
    }
  }
  return retval;
}

/*================================================================*/

int Map3d_Geom::CheckElementValidity()
{

  /*** See whether all connectivites in the elements are valid ***/

  long i, j, k, l;
  int retval = GEOM_OK;
  /**********************************************************************/
  if (!elements)
    return retval;

  for (i = 0; i < numelements; i++) {
    j = 0;
    for (k = j + 1; k < elementsize; k++) {
      if (elements[i][j] == elements[i][k]) {
        fprintf(stderr, "+++ In CheckElementValidity in surface"
                "#%ld\n"
                "    Element #%ld of size %ld "
                " has pointers to the same node\n"
                "    Node numbers are = %ld, %ld",
                surfnum + 1, i + 1, elementsize, elements[i][0] + 1, elements[i][1] + 1);
        for (l = 2; l < elementsize; l++)
          fprintf(stderr, ", %ld", elements[i][l] + 1);
        fprintf(stderr, "\n");
        retval = GEOM_WARNING;
        break;
      }
    }

  }
  return retval;
}

void Map3d_Geom::SetTimestep(int)
{
  //geom_index = data_timestep;
}

void Map3d_Geom::UpdateTimestep(Surf_Data* data)
{
  // need to use these metrics in case the user specified a frame step or frame end limit on the command line
  // (which actually stop the data from loading into memory)
  int dataFrame = data->getRealFrameNum();
  int numDataFrames = data->ts_available_frames;

  // the frame rate should be proportional to the data frame rate
  geom_index = ((double) dataFrame/numDataFrames) * points.size();
  //printf("geom update %d %d = frame %d\n", dataFrame, numDataFrames, geom_index);

}