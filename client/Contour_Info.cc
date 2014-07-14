/* Contour_Info.cxx */

#include "Contour_Info.h"
#include "Map3d_Geom.h"
#include "MeshList.h"
#include "Surf_Data.h"

#include "scalesubs.h"
#include "map3d-struct.h"
#include "geomlib.h"
#include "geomutilsubs.h"
#include "cutil.h"
#include "map3dmath.h"

#include <math.h>
#include <float.h>
#include <stdlib.h>

extern Map3d_Info map3d_info;

Contour_Info::Contour_Info(Mesh_Info * m)
{
  mesh = m;
  surfnum = surfnum;
  numlevels = 0;
  numisosegs = 0;
  maxnumisosegs = -1;
  potthreshold = 0.0;
  distthreshold = 0.0;
  isolevels = NULL;
  contpt1 = NULL;
  contpt2 = NULL;
  contcol = NULL;
  trinum = NULL;
  datatype = -2;
  
  fidmap = 0;
  // FIX gdk_color_parse ("red", &fidcolor);
  fidContSize = 3.0;
  numfidconts = 10;
  fidcontourspacing = 0.0;
  userfidmax = 0;
  userfidmin = 0;
  user_fid_range = false;
  use_spacing_fid = 0;
  
  maxnumbandpolys = 0;
  numbandpolys = 0;
  bandpolys = NULL;
  
  numtrisegs = 0;
  numtris = 0;
  maxnumtris = 0;
  occlusion_gradient = FLT_MAX;
}

Contour_Info::~Contour_Info()
{
  if (bandpolys) {
    for (int i = 0; i < numbandpolys; i++) {
      Free_fmatrix(bandpolys[i].nodes, bandpolys[i].numpts);
      Free_fmatrix(bandpolys[i].normals, bandpolys[i].numpts);
    }
    free(bandpolys);
    bandpolys = 0;
    maxnumbandpolys = 0;
    numbandpolys = 0;
  }
  if (contpt1)
    Free_fmatrix(contpt1, CONTBLOCK);
  if (contpt2)
    Free_fmatrix(contpt2, CONTBLOCK);
  if (isolevels)
    free(isolevels);
  if (contcol)
    free(contcol);
  if (trinum)
    free(trinum);
  if (numtrisegs)
    free(numtrisegs);
}

/* generate contours/bands */
int Contour_Info::buildContours()
{
  if (!mesh->data)
    return -1;
  
  // this is to tell it to go through all the polys again
  // intead of generating more
  numbandpolys = 0;
  
  float min = 0, max = 0;
  
  // give the contours enough triangles
  int numtris = mesh->geom->numelements;
  if (!this->numtrisegs) {
    this->numtrisegs = (long *)calloc((size_t) numtris + 10, sizeof(long));
    this->maxnumtris = numtris + 10;
    this->numtris = numtris;
  }
  else if (this->maxnumtris < numtris) {
    this->numtrisegs = (long *)realloc(this->numtrisegs, (size_t) (numtris + 10) * sizeof(long));
    this->maxnumtris = numtris + 10;
    this->numtris = numtris;
  }
  if (!this->numtrisegs) {
    printf("Error getting trinumsegs memory.\n");
    exit(ERR_MEM);
  }
  
  if(this->datatype == -2){
    mesh->data->get_minmax(min, max);
    //this->potthreshold = (mesh->data->potmax - mesh->data->potmin) / 1.0e5;
    this->potthreshold = (max - min) / 1.0e5;
  }
  this->distthreshold = mesh->geom->cubescale / 1.0e3;
  //printf("buildContours: min: %8.2f max: %8.2f\n",min,max);
  
  if (map3d_info.use_spacing) {
    if (mesh->contourspacing) {
      float conts = ((max - min) / mesh->contourspacing - 1);
      bool roundup = false;
      // round up
      if (conts - (int) conts > 0) {
        mesh->data->numconts = (int) conts + 1;
        roundup = true;
      }
      else
        mesh->data->numconts = (int) conts;
      if (mesh->data->numconts > MAX_CONTOURS) {
        fprintf(stderr, "Too many conts for spacing %f, temporarily setting spacing to %f\n", mesh->contourspacing, (max-min)/((float) MAX_CONTOURS + 1));
        mesh->data->numconts = MAX_CONTOURS;
      }
      // if the CS is set, the adjust the range to make sure we have that exact CS, rather than rounding it by
      // the current numconts (as long as we have a CS-worthy scaling function/map)
      if (roundup && (map3d_info.scale_model == LINEAR && map3d_info.scale_mapping == TRUE_MAP)) {
        float newrange = (mesh->data->numconts+1)*mesh->contourspacing;
        float diff = newrange-(max-min);
        max = max+diff/2.0f;
        min = min-diff/2.0f;
      }
      
    }
  }
  if (this->use_spacing_fid) {
    float fidmin, fidmax;
    if(this->user_fid_range){
      fidmin = this->userfidmin;
      fidmax = this->userfidmax;
    }
    else{
      mesh->data->get_fid_minmax(fidmin, fidmax, this->datatype);
    }
    if (this->fidcontourspacing)
      this->numfidconts = (int)((fidmax - fidmin) / this->fidcontourspacing - 1);
  }
  if (mesh->data->numconts < 1)
    mesh->data->numconts = 1;
  if(datatype ==  -2){
    GenSurfContourBand(min, max, mesh->geom, mesh->data);
  }
  else{
    float fidmin, fidmax;
    if(this->user_fid_range){
      fidmin = this->userfidmin;
      fidmax = this->userfidmax;
    }
    else{
      mesh->data->get_fid_minmax(fidmin, fidmax, this->datatype);
    }   
    this->potthreshold = (fidmax - fidmin) / 1.0e5;
    //printf("fidmin %f, fidmax %f, fidset %d,  fidtype %d\n",fidmin, fidmax, this->fidset, this->datatype);
    GenSurfContourBand(fidmin, fidmax, mesh->geom, mesh->data);
  }
  
#if 0 // turn off until we find a more efficient way
      // generate normals for the band polys
  for (int origpt = 0; origpt < numbandpolys; origpt++) {
    float **pts = bandpolys[origpt].nodes;
    float **normals = bandpolys[origpt].normals;
    
    for (int pt1 = 0; pt1 < bandpolys[origpt].numpts; pt1++) {
      normals[pt1][0] = 0;
      normals[pt1][1] = 0;
      normals[pt1][2] = 0;
      
      // add this normal to any point who shares space with any point on this triangle
      // (yes, I know it's inefficient)
      for (int comp_pt = 0; comp_pt < numbandpolys; comp_pt++) {
        float** comp_pts = bandpolys[comp_pt].nodes;
        for (int pt2 = 0; pt2 < bandpolys[comp_pt].numpts; pt2++) {
          if (pts[pt1][0] == comp_pts[pt2][0] &&
              pts[pt1][1] == comp_pts[pt2][1] &&
              pts[pt1][2] == comp_pts[pt2][2]) {
            
            // add this triangle's normal to the original normal, to average them all out later
            float v1x = comp_pts[1][0] - comp_pts[0][0];
            float v1y = comp_pts[1][1] - comp_pts[0][1];
            float v1z = comp_pts[1][2] - comp_pts[0][2];
            
            float v2x = comp_pts[2][0] - comp_pts[0][0];
            float v2y = comp_pts[2][1] - comp_pts[0][1];
            float v2z = comp_pts[2][2] - comp_pts[0][2];
            
            normals[pt1][0] = v1y * v2z - v2y * v1z;
            normals[pt1][1] = v1z * v2x - v2z * v1x;
            normals[pt1][2] = v1x * v2y - v2x * v1y;
          }
        }
      }
      normalizeVector(normals[pt1]);
    }
  }
#endif
  
  if(datatype ==  -2)
    return mesh->data->numconts;
  else
    return this->numfidconts;
  
}

long Contour_Info::AddContourSeg(long *numsegs, float node1[3], float node2[3], long levelnum, long trinum)
{
  
  /*** Add the next contour segment to the list.  ***/
  long segnum, k;
  long error = 0;
  /**********************************************************************/
  /*** See if we need more memory allocated here and get more if necessary. ***/
  
  if ((*numsegs + 3) > this->maxnumisosegs) {
    error = GrowConts(CONTBLOCK);
    if (error < 0)
      exit((int)error);
  }
  
  /*** See if we are going too crazy with the contours and if so,
    force a view by shading so that the user at least sees something
    useful. ***/
  
  if (this->maxnumisosegs > ABSMAXCONTS) {
    fprintf(stderr, "+++ Warning from GenSurfConts\n"
            "    The number of contour segments"
            " just climbed to %ld\n"
            "    This is getting silly so "
            " let's try and look at the data\n"
            "    in shaded view and see what is" " going on around here!!\n", this->maxnumisosegs);
    
    return (ERR_MISC);
  }
  
  /*** Check the distance between nodes on the segment against
    the threshold. ***/
  
  if ((Dist3D(node1, node2)) < this->distthreshold)
    return (error);
  segnum = *numsegs;
  for (k = 0; k < 3; k++) {
    this->contpt1[segnum][k] = node1[k];
    this->contpt2[segnum][k] = node2[k];
  }
  this->contcol[segnum] = levelnum;
  this->trinum[segnum] = trinum;
  segnum++;
  *numsegs = segnum;
  return (error);
}


long Contour_Info::GrowConts(long contblocksize)
{
  /*** Add some size to the arrays for contour segments.
  ***/
  long oldsize, newsize;
  /**********************************************************************/
  oldsize = maxnumisosegs;
  newsize = oldsize + contblocksize;
  maxnumisosegs = newsize;
  if (map3d_info.reportlevel > 1)
    fprintf(stderr, " Growing space for "
            "additional " "contour segments\n" " We want to expand from" " %ld to %ld \n", oldsize, newsize);
  if ((contpt1 = Grow_fmatrix(contpt1, oldsize, newsize, 3, 3)) == NULL)
    return (ERR_MEM);
  if ((contpt2 = Grow_fmatrix(contpt2, oldsize, newsize, 3, 3)) == NULL)
    return (ERR_MEM);
  if ((contcol = (long *)realloc(contcol, (size_t) newsize * sizeof(long))) == NULL) {
    fprintf(stderr, "***** In GrowConts, " "error allocating space" " for model_contcol array\n");
    return (ERR_MEM);
  }
  if ((trinum = (long *)realloc(trinum, (size_t) newsize * sizeof(long))) == NULL) {
    fprintf(stderr, "***** In GrowConts, " "error allocating space" " for model_trinum array\n");
    return (ERR_MEM);
  }
  return 0;
  
}

void Contour_Info::AddBandPoly(float **nodes, long numnodes, float potval, long isonum, float** normals)
{
  /*** Add a new bandshade poly to the list. 
Input:
  nodes	pointer to nodes of the poly
  numnodes	number of nodes in the poly
  potval	real value associated with this poly
  isonum	number of the isolevel value to assign to poly
  normals	normal cosines for the poly
  
  ***/
  
  long i, j;
  /************************************************************************/
  
  /*** See if we need to set up some new memory. ***/
  
  if (!bandpolys || maxnumbandpolys < 1) {
    if ((bandpolys = (Bandpoly *) calloc((size_t) BANDBLOCK, sizeof(Bandpoly))) == NULL) {
      printf("*** AddBandpoly: could not get first bandpoly memory\n");
      exit(ERR_MEM);
    }
    maxnumbandpolys = BANDBLOCK;
    numbandpolys = 0;
    for (int i = 0; i < maxnumbandpolys; i++) {
      bandpolys[i].nodes = 0;
      bandpolys[i].normals = 0;
    }
    if (map3d_info.reportlevel > 2)
      printf(" Setting up new memory for %ld bandpolys in cont\n", BANDBLOCK);
    /*** Or just expand existing. ***/
    
  }
  else if (numbandpolys + 1 >= maxnumbandpolys) {
    maxnumbandpolys += BANDBLOCK;
    bandpolys = (Bandpoly *) realloc(bandpolys, (size_t) maxnumbandpolys * sizeof(Bandpoly));
    
    // set the nodes arrays to null
    for (int i = maxnumbandpolys - BANDBLOCK; i < maxnumbandpolys; i++) {
      bandpolys[i].nodes = 0;
      bandpolys[i].normals = 0;
    }
    if (map3d_info.reportlevel)
      printf(" Added more memory for %ld bandpolys in cont\n", maxnumbandpolys);
  }
  
  /*** Now set up this poly in the array. ***/
  
  Bandpoly *bp = &bandpolys[numbandpolys];
  
  if (!bp->nodes) {
    bp->nodes = Alloc_fmatrix(numnodes, 3);
    bp->normals = Alloc_fmatrix(numnodes, 3);
    bp->maxpts = numnodes;
  }
  else if (bp->maxpts < numnodes) {
    //Free_fmatrix(bandpolys[numbandpolys].nodes, bandpolys[numbandpolys].numpts);
    bp->nodes = Grow_fmatrix(bp->nodes, bp->numpts, numnodes, 3, 3);
    bp->normals = Grow_fmatrix(bp->normals, bp->numpts, numnodes, 3, 3);
    bp->maxpts = numnodes;
  }
  bp->numpts = numnodes;
  for (i = 0; i < numnodes; i++) {
    for (j = 0; j < 3; j++) {
      bp->nodes[i][j] = nodes[i][j];
      bp->normals[i][j] = normals[i][j];
    }
  }
  
  /*** Now set the colour. ***/
  bp->bandcol = isonum;
  bp->bandpotval = potval;
  numbandpolys++;
  
  /*** Return the updated array. ***/
  
  //return (cont);
  
}

long Contour_Info::GenSurfContourBand(float min, float max, Map3d_Geom * onemap3dgeom, Surf_Data * onesurfdata)
{
  /*
   * Routine to compute the contour segments and bands for a single surface.
   */
  
  long surfnum;
  long i, j;
  long maxsegs;
  long error = 0;
  long maxlevels;
  long numsegs;
  long oldnumsegs;
  long numconts;
  long levelnum, framenum, ltrinum;
  long npts, ncontours;
  long nbadtris;  /*** Number of tris that have diffs below threshold ***/
  long nmissedtris;  /*** Number of tris that get missed ***/
  long noutsidetris;  /*** Number of tris outside the contours ***/
  long nodes[3];
  double potdiffthreshold;
  float polymin, polymax, polymed;
  float level;
  float **cont_pts;
  float pot[3];
  bool qusedtri;
  
  static float **trinodes = NULL;
  static float **trinorms = NULL;
  float nextlevel;
  static TriContBandStruct cb;
  
  if (map3d_info.reportlevel > 2)
    fprintf(stderr, "In GenSurfCont for surface #%ld\n", onesurfdata->surfnum + 1);
  
  /*** Set the local frame and surface number. ***/
  
  framenum = onesurfdata->framenum;
  surfnum = onesurfdata->surfnum;
  if((this->datatype !=-2) && (this->fidmap == 0)){
    numconts = 1;
  }
  else{
    if(this->datatype == -2)
      numconts = onesurfdata->numconts;
    else
      numconts = this->numfidconts;
  }
  //bandshades->numbandpolys = 0;
  
  /***
    * See if we need to allocate memory to the arrays in the 
    * data structure
    ***/
  
  if (maxnumisosegs < 1) {
    maxsegs = maxnumisosegs = CONTBLOCK;
    maxlevels = maxnumlevels = numconts + 5;
    if ((contpt1 = Alloc_fmatrix(maxsegs, 3)) == NULL)
      exit(ERR_MEM);
    if ((contpt2 = Alloc_fmatrix(maxsegs, 3)) == NULL)
      exit(ERR_MEM);
    if ((isolevels = (float *)calloc((size_t) maxlevels, sizeof(float))) == NULL)
      exit(ERR_MEM);
    if ((contcol = (long *)calloc((size_t) maxsegs, sizeof(long))) == NULL)
      exit(ERR_MEM);
    if ((trinum = (long *)calloc((size_t) maxsegs, sizeof(long))) == NULL)
      exit(ERR_MEM);
  }
  else {
    maxsegs = maxnumisosegs;
    maxlevels = maxnumlevels = numconts + 5;
    //free(isolevels);
    //if ((isolevels = (float *)calloc((size_t) maxlevels, sizeof(float))) == NULL)
    //exit(ERR_MEM);
  }
  
  if (!trinodes) {
    trinodes = Alloc_fmatrix(5, 3);
    trinorms = Alloc_fmatrix(5, 3);
    if (!trinorms)
      exit(ERR_MEM);
  }
  
  
  /*** Now the loop through all the triangles to find the contours
    * that pass through each side of each triangle.
    * The scheme is pretty simple and yields a set of line segments, each
    * of which has 2 endpoints and a pointer to a potential value array.
    ****/
  
  numsegs = 0;
  
  /*** Set a threshold to use for the minimum difference between potentials
    * across a triangle--if a triangle does not span this range, then
    * ignore it. 
    ***/
  
  potdiffthreshold = potthreshold / 10.E2;
  
  if (map3d_info.reportlevel > 1) {
    printf(" The two thresholds for surface #%ld are\n"
           " diffs:  %g and levels: %g\n", surfnum + 1, potdiffthreshold, potthreshold);
  }
  
  /*** Get the contour levels based on the scaling model, the extrema, and 
    * the desired number of contours 
    ***/
  if (isolevels)
    free(isolevels);
  isolevels = GetContVals(min, max, numconts, onesurfdata->usercontourstep, &ncontours);
  numlevels = ncontours;
  
  if (map3d_info.reportlevel > 1) {
    fprintf(stderr, " The contour levels for surface #%ld are set\n", surfnum + 1);
    fprintf(stderr, " With PotMaxLo = %f PotMaxHi = %f" " and scale_model = %ld\n", min, max, map3d_info.scale_model);
    for (i = 0; i < ncontours; i++) {
      fprintf(stderr, " Level %ld is at %f\n", i + 1, isolevels[i]);
    }
  }
  
  /*** Loop through  all the triangles in the surface. ***/
  
  nbadtris = nmissedtris = noutsidetris = 0;
  for (ltrinum = 0; ltrinum < onemap3dgeom->numelements; ltrinum++) {
    cb.ltrinum = ltrinum;
    cb.ifsetprev = false;
    cb.currnum = cb.prevnum = 0;
    numtrisegs[ltrinum] = 0;
    
    oldnumsegs = numsegs;
    polymin = 1.0e+30f;
    polymax = -1.0e+30f;
    polymed = 1.0e+30f;
    for (j = 0; j < 3; j++) {
      nodes[j] = onemap3dgeom->elements[ltrinum][j];
      if(this->datatype == -2){
        pot[j] = onesurfdata->potvals[framenum][nodes[j]];
      }
      else{
        pot[j] = 0;
        if(nodes[j] < onesurfdata->fids.numfidleads){
          for(int numfids = 0; numfids < onesurfdata->fids.leadfids[nodes[j]].numfids; numfids++){
            if((onesurfdata->fids.leadfids[nodes[j]].fidtypes[numfids] == this->datatype)){
              pot[j] = onesurfdata->fids.leadfids[nodes[j]].fidvals[numfids];
            }
          }
        }
      }
      //      else{
      //        pot[j] = 0;
      //        for(int fidsets = 0; fidsets < onesurfdata->numfs; fidsets++){
      //          if(nodes[j] < onesurfdata->fids[fidsets].numfidleads){
      //            for(int numfids = 0; numfids < onesurfdata->fids[fidsets].leadfids[nodes[j]].numfids; numfids++){
      //              if((onesurfdata->fids[fidsets].leadfids[nodes[j]].fidtypes[numfids] == this->datatype)){
      //                pot[j] = onesurfdata->fids[fidsets].leadfids[nodes[j]].fidvals[numfids];
      //              }
      //            }
      //          }
      //        }
      //      }
      polymin = cjmin(polymin, pot[j]);
      polymax = cjmax(polymax, pot[j]);
      //printf("nodes[j%d] = %d\n",j,nodes[j]);
    }
   
    // second pass to get pot[j+1] initialized
    for (j = 0; j < 3; j++) {
      if (pot[j] == UNUSED_DATA)
        break;
      if (fabs(pot[j]-pot[(j+1)%3]) > this->occlusion_gradient) {
        // if there is a "high gradient" (greater than specified value), treat as UNUSED
        // (useful for when there would be too many contours clumped together)
        break;
      }
    }

    if (j < 3)
      // we have "UNUSED_DATA" on a node in this triangle, so don't put bands or conts here
      continue;
    
    // we use polymed solely for the purpose if 2 nodes have the same value
    if (pot[0] == pot[1] || pot[0] == pot[2])
      polymed = pot[0];
    else if (pot[1] == pot[2])
      polymed = pot[1];
    
    /*** 
      * Make sure we have a triangle with some variety in values over the
      * nodes.  If not, add it into bandpoly?, but ignore it for contour.
      ***/
    /*
     if (  ( (double) polymax - (double) polymin ) < potdiffthreshold )
     {
       
       nbadtris++;
       qusedtri = true;
       numtrisegs[ltrinum] = 0;
       continue;
     }
     */
    
    /*** 
      * See if the triangle range lies outside the levels we have defined.
      ***/
    if((this->datatype == -2)||(this->fidmap == 1)){
      
      if (polymax < isolevels[0] + potthreshold || max == min) {
        /* triangle above all contours or scale range is constant */
        
        noutsidetris++;
        qusedtri = true;
        numtrisegs[ltrinum] = 0;
        
        /* add into bandpoly */
        cb.prepareBandTriangle(pot, trinodes, nodes, trinorms, onemap3dgeom, ltrinum);
        
        // use -1 to represent that it is below the contour vals
        AddBandPoly(trinodes, 3, min, -1, trinorms);
        
        continue;
      }
      else if (polymin > isolevels[ncontours - 1] - potthreshold) {
       /* triangle below all contours */
        
        noutsidetris++;
        qusedtri = true;
        numtrisegs[ltrinum] = 0;
        
        /* add into bandpoly */
        cb.prepareBandTriangle(pot, trinodes, nodes, trinorms, onemap3dgeom, ltrinum);
        
        AddBandPoly(trinodes, 3, isolevels[ncontours - 1], ncontours - 1, trinorms);
        continue;
      }
    }
    
    /*** 
      * If all is well, scan all the level values to see if one lies
      * between values at the nodes of this triangle. 
      ***/
    
    qusedtri = false;
    cb.currlevel = min;
    cb.currlevelnum = -1;
    for (levelnum = 0; levelnum < ncontours; levelnum++) {
      cb.prevlevel = cb.currlevel;
      cb.prevlevelnum = (float)cb.currlevelnum;
      if((this->datatype !=-2)&&(this->fidmap == 0)){
        cb.currlevel = level = (float) framenum;
        //printf("framenum = %d\n", framenum);
      }
      else{
        cb.currlevel = level = isolevels[levelnum];
      }
      cb.currlevelnum = levelnum;
      
      if (levelnum < ncontours - 1) {
        nextlevel = isolevels[levelnum + 1];
        if (level < polymin + potthreshold && nextlevel > polymax - potthreshold) {
          /* this triangle is between two contours */
          
          cb.prepareBandTriangle(pot, trinodes, nodes, trinorms, onemap3dgeom, ltrinum);
          AddBandPoly(trinodes, 3, level, levelnum, trinorms);
          
          break;
        }
      }
      
      // here we might have a special case - we normally don't want 
      // a contour until we are slightly greater than the min, but
      // we need to handle the case where two nodes are equal to the min
      if (level < polymin + potthreshold && level < polymed)
        continue;
      
      if (level > polymax - potthreshold) {
        if (cb.prevnum == 2)
          cb.checkLastBand(pot, trinodes, nodes, trinorms, onemap3dgeom, this);
        break;
      }
      
      qusedtri = true;
      npts = 0; 
      
      
      cb.checkContourLevel(ltrinum, 0, 1, pot, nodes, onemap3dgeom, &npts, potdiffthreshold);
      cb.checkContourLevel(ltrinum, 1, 2, pot, nodes, onemap3dgeom, &npts, potdiffthreshold);
      cb.checkContourLevel(ltrinum, 2, 0, pot, nodes, onemap3dgeom, &npts, potdiffthreshold);
      
      if (npts == 2 || npts == 3)
        cb.checkPossibleBands(pot, trinodes, nodes, trinorms, onemap3dgeom, this);
      
      cont_pts = cb.currpts;
      
      switch (npts) {
        case 1:
          if(this->datatype == -2){
            printf("GenSurfContourBand: frame number %ld, triangle number %ld\n"
                   "    npts = %ld"
                   "    with nodes %ld, %ld, and %ld\n"
                   "    and pots %f, %f, and %f\n"
                   "    and threshold = %g\n"
                   "    level = %f at %7.2f, %7.2f, %7.2f\n",
                   framenum, ltrinum + 1, npts, nodes[0] + 1, nodes[1] + 1, nodes[2] + 1,
                   pot[0], pot[1], pot[2], potthreshold, level, cont_pts[0][X], cont_pts[0][Y], cont_pts[0][Z]);
          }
          break;
          
        case 2:
          /*** this is the "normal" case ***/
          error = AddContourSeg(&numsegs, cont_pts[0], cont_pts[1], levelnum, ltrinum);
          if (map3d_info.reportlevel > 3)
            printf(" Found 2 points for level #%ld = %f\n", levelnum + 1, level);
            
            if (error < 0) {
              return (error);
            }
              
              
              break;
          
        case 3:
          /*** 
        * Now the case in which there are three hits, 
          * which should only happen when the potential values 
          * at all the triangles are equal to the value
          * of 'level'.  Draw the whole triangle! 
          ***/
          
          error = AddContourSeg(&numsegs, cont_pts[0], cont_pts[1], levelnum, ltrinum);
          error = AddContourSeg(&numsegs, cont_pts[1], cont_pts[2], levelnum, ltrinum);
          error = AddContourSeg(&numsegs, cont_pts[2], cont_pts[0], levelnum, ltrinum);
          if (error < 0) {
            return (error);
          }
            if (map3d_info.reportlevel > 2)
              printf(" Found 3 points for level %f\n", level);
            break;
          
        case 0:
          break;
          
        default:
          fprintf(stderr, "*** Error since there are too"
                  " many intersections of contour "
                  " with triangle.\n" "    There are %ld points of" " intersection\n", npts + 1);
          fprintf(stderr, "At triangle %ld and level = " "%f\n", ltrinum, level);
          fprintf(stderr, "Pot values are %f, %f, " " and %f\n", pot[0], pot[1], pot[2]);
          break;
      }
      
      cb.propergateCurrToPrev();
    }                           /* end of for(levelnum...;;)... */

    if (levelnum == ncontours && cb.prevnum == 2) {
      cb.prevlevel = isolevels[ncontours - 1];
      cb.prevlevelnum = (float)(ncontours - 1), cb.checkLastBand(pot, trinodes, nodes, trinorms, onemap3dgeom, this);
    }

  /*** 
	 * Increment the number of triangles that did not have any contours.
   * this triangle lies between two levels.
	 ***/

    if (!qusedtri) {
      nmissedtris++;
    }

  /*** 
	 * Set the number of line segments we have found in this triangle. 
	 ***/

    numtrisegs[ltrinum] = numsegs - oldnumsegs;

  }                             /* end of for(ltrinum...;;)... */

  numisosegs = numsegs;

    /*** 
    * Report back if we have any strangeness.
    ***/

  if (nbadtris > 0 && map3d_info.reportlevel > 1) {
    fprintf(stderr, "*** For surface #%ld we have %ld triangles of %ld\n"
            "*** without large enough gradient\n", surfnum + 1, nbadtris, onemap3dgeom->numelements);
  }
  if (noutsidetris > 0 && map3d_info.reportlevel > 1) {
    fprintf(stderr, "*** For surface #%ld we have %ld triangles of %ld\n"
            "*** that lie outside the controu range of %.3f--%.3f\n",
            surfnum + 1, noutsidetris, onemap3dgeom->numelements, isolevels[0], isolevels[ncontours - 1]);
  }
  if (nmissedtris > 0 && map3d_info.reportlevel > 1) {
    fprintf(stderr, "*** For surface #%ld we have %ld triangles of %ld\n"
            "*** that did not get a contour segment\n", surfnum + 1, nmissedtris, onemap3dgeom->numelements);
  }

  if (map3d_info.reportlevel > 3) {
    fprintf(stderr, " Finished checking triangles for contour levels\n");
    fprintf(stderr, " numsegs = %ld\n", numsegs);
  }

  return 0;
}

TriContBandStruct::TriContBandStruct()
{
  prevpts = Alloc_fmatrix(3, 3);
  prevnormals = Alloc_fmatrix(3, 3);
  previdx1 = (long *)malloc(3 * sizeof(long));
  previdx2 = (long *)malloc(3 * sizeof(long));
  
  currpts = Alloc_fmatrix(3, 3);
  currnormals = Alloc_fmatrix(3, 3);
  curridx1 = (long *)malloc(3 * sizeof(long));
  curridx2 = (long *)malloc(3 * sizeof(long));
  
  if (!prevpts || !currpts || !previdx1 || !previdx2 || !curridx1 || !curridx2) {
    fprintf(stderr, "Error allocating TriConBandStruct memory");
    exit(ERR_MEM);
  }
}

TriContBandStruct::~TriContBandStruct()
{
  Free_fmatrix(prevpts, 3);
  Free_fmatrix(prevnormals, 3);
  Free_fmatrix(currpts, 3);
  Free_fmatrix(currnormals, 3);
  
  free(previdx1);
  free(previdx2);
  free(curridx1);
  free(curridx2);
  
}

/* Swap current points with previous ones in TriContBandStruct */
void TriContBandStruct::swapCurrPrev()
{
  long *tmp_longpt;
  float **tmp_floatpt;
  
  tmp_longpt = previdx1;
  previdx1 = curridx1;
  curridx1 = tmp_longpt;
  
  tmp_longpt = previdx2;
  previdx2 = curridx2;
  curridx2 = tmp_longpt;
  
  tmp_floatpt = prevpts;
  prevpts = currpts;
  currpts = tmp_floatpt;
  
  tmp_floatpt = prevnormals;
  prevnormals = currnormals;
  currnormals = tmp_floatpt;
}

/*======================================================================*/

/* Before going to next contour, reset the TriContBandStruct */
void TriContBandStruct::propergateCurrToPrev()
{
  prevnum = currnum;
  swapCurrPrev();
  currnum = 0;
  if (!ifsetprev)
    ifsetprev = true;
}


/*======================================================================*/

/* Calculate intercepting point if any,
* and also remember indices of end points.
*/
void TriContBandStruct::checkContourLevel(long trinum, long i1, long i2, float pot[3], long nodes[3],
                                          Map3d_Geom * onemap3dgeom, long *npts, double potdiffthreshold)
{
  double fract;
  int i, prev, curr;
  float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
  /***********************************************************************/
  
  /*** Check if we have level equal to potential at node 1. 
    If so, add that point to the list.
    ***/
  
  if (fabs((double)currlevel - pot[i1]) < potdiffthreshold) {
    for (i = 0; i < 3; i++) {
      currpts[*npts][i] = pts[nodes[i1]][i];
      currnormals[*npts][i] = onemap3dgeom->ptnormals[nodes[i1]][i];
      //printf("nodes[%d] = %d\n",i1,nodes[i1]);
    }
    (*npts)++;
    
    /*** Or else the normal case, where we find a crossing point on a side. ***/
    
  }
  else {
    
    fract = (double)(currlevel - pot[i1]) / (pot[i2] - pot[i1]);
    if (fract <= 0.00 || fract >= 1.0)
      return;
    
    // create the normal as the average of this triangle and the one next to it
    float *trinorm = onemap3dgeom->fcnormals[trinum];
    float *othernorm = 0;
    if (onemap3dgeom->adjacent_triangles[trinum][i1] != -1) {
      othernorm = onemap3dgeom->fcnormals[onemap3dgeom->adjacent_triangles[trinum][i1]];
    }
    
    for (i = 0; i < 3; i++) {
      currpts[*npts][i]
      = pts[nodes[i1]][i]
      + (float)fract *(pts[nodes[i2]][i] - pts[nodes[i1]][i]);
      
      if (othernorm) {
        currnormals[*npts][i] = (trinorm[i] + othernorm[i])/2.0f;
      }
      else {
        currnormals[*npts][i] = trinorm[i];
      }
    }
    (*npts)++;
  }
  if (pot[i1] < pot[i2]) {
    prev = i1;
    curr = i2;
  }
  else {
    prev = i2;
    curr = i1;
  }
  
  
  curridx1[currnum] = prev;     /* with low pot */
  curridx2[currnum] = curr;     /* with high pot */
  currnum++;
  
  if (!ifsetprev) {
    /* encounters 1st contour line. */
    prevnum++;
  }
}


/*======================================================================*/




/*======================================================================*/

void TriContBandStruct::prepareBandTriangle(float * /*pot[3] */ , float **trinodes,
                                            long nodes[3], float **trinorms,
                                            Map3d_Geom * onemap3dgeom, long /*ltrinum */ )
{
  float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
  for (int i = 0; i < 3; i++) {
    trinodes[0][i] = pts[nodes[0]][i];
    trinodes[1][i] = pts[nodes[1]][i];
    trinodes[2][i] = pts[nodes[2]][i];
    trinorms[0][i] = onemap3dgeom->ptnormals[nodes[0]][i];
    trinorms[1][i] = onemap3dgeom->ptnormals[nodes[1]][i];
    trinorms[2][i] = onemap3dgeom->ptnormals[nodes[2]][i];
  }
}

/*======================================================================*/

void TriContBandStruct::addOneBand(float **trinodes, long numnodes, float **trinorms, Map3d_Geom * /*onemap3dgeom */ ,
                                   Contour_Info * onecontourinfo)
{
  onecontourinfo->AddBandPoly(trinodes, numnodes, prevlevel, (long)prevlevelnum, trinorms);
}


/*======================================================================*/

/* given one contour level, find any bands between this contour line
* and previous one. Note: in some cases, one single point is recorded
* in two position in an array,.i.e, a 4-point band may acturally
* a 3-point one.
*/
void TriContBandStruct::checkPossibleBands(float pot[3], float **trinodes, long nodes[3],
                                           float **trinorms, Map3d_Geom * onemap3dgeom, Contour_Info * onecontourinfo)
{
  int i, which;
  int ifSwap = false;
  float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
  
  if (currnum == 3) {
    /***
    * A case in which the three points in the triangle has
    * same pot values, Draw the whole triangle!
    ***/
    
    prepareBandTriangle(pot, trinodes, nodes, trinorms, onemap3dgeom, ltrinum);
    
    onecontourinfo->AddBandPoly(trinodes, 3, currlevel, currlevelnum, trinorms);
    return;
  }
  
  /* check if we get anything strange */
  if (currnum != 2 || prevnum != 2)
    return;
  
  if (!ifsetprev) {
    /* this is the first band in this triangle */
    
    if (curridx1[0] == curridx1[1]) {
      /* up triangle */
      
      if (curridx2[0] == curridx2[1]) {
        /* this should not happen, just in case */
        return;
      }
      
      which = curridx2[0];
      for (i = 0; i < 3; i++) {
        trinodes[0][i] = currpts[0][i];
        trinorms[0][i] = currnormals[0][i];
        if (curridx2[1] == (which + 1) % 3) {
          trinodes[1][i] = currpts[1][i];
          trinodes[2][i] = pts[nodes[(which + 2) % 3]][i];
          trinorms[1][i] = currnormals[1][i];
          trinorms[2][i] = onemap3dgeom->ptnormals[nodes[(which + 2) % 3]][i];
        }
        else {                  /* curridx2[1] == (which+2)%3 */
          trinodes[1][i] = pts[nodes[(which + 1) % 3]][i];
          trinodes[2][i] = currpts[1][i];
          trinorms[1][i] = onemap3dgeom->ptnormals[nodes[(which + 1) % 3]][i];
          trinorms[2][i] = currnormals[1][i];
        }
      }
      addOneBand(trinodes, 3, trinorms, onemap3dgeom, onecontourinfo);
      
    }
    else {                      /* curridx1[0] != curridx1[1] */
      
      /* down triangle */
      
      if (curridx2[0] != curridx2[1]) {
        /* this should not happen, just in case */
        return;
      }
      
      which = curridx1[0];
      for (i = 0; i < 3; i++) {
        trinodes[0][i] = pts[nodes[which]][i];
        trinorms[0][i] = onemap3dgeom->ptnormals[nodes[which]][i];
        if (curridx1[1] == (which + 1) % 3) {
          trinodes[1][i] = pts[nodes[(which + 1) % 3]][i];
          trinodes[2][i] = currpts[1][i];
          trinodes[3][i] = currpts[0][i];
          trinorms[1][i] = onemap3dgeom->ptnormals[nodes[(which + 1) % 3]][i];
          trinorms[2][i] = currnormals[1][i];
          trinorms[3][i] = currnormals[0][i];
        }
        else {
          trinodes[1][i] = currpts[0][i];
          trinodes[2][i] = currpts[1][i];
          trinodes[3][i] = pts[nodes[(which + 2) % 3]][i];
          trinorms[1][i] = currnormals[0][i];
          trinorms[2][i] = currnormals[1][i];
          trinorms[3][i] = onemap3dgeom->ptnormals[nodes[(which + 2) % 3]][i];
        }
      }
      addOneBand(trinodes, 4, trinorms, onemap3dgeom, onecontourinfo);
    }

return;

  }

/* end of if (!ifsetprev)... */
/* Not the 1st band in this triangle */
if (previdx1[0] == curridx1[0] &&
    previdx1[1] == curridx1[1] && previdx2[0] == curridx2[0] && previdx2[1] == curridx2[1]) {
  
  /***
  * the current contour line and previous contour line
  * all cut the same two sides of the triangle.
  ***/
  
  if (curridx2[0] == curridx2[1]) {
    /* down triangle */
    
    if (curridx1[0] == curridx1[1]) {
      /* this should not happen, just in case. */
      return;
    }
    
    which = curridx1[0];
    for (i = 0; i < 3; i++) {
      trinodes[0][i] = prevpts[0][i];
      trinorms[0][i] = prevnormals[0][i];
      if (curridx1[1] == (which + 1) % 3) {
        trinodes[1][i] = prevpts[1][i];
        trinodes[2][i] = currpts[1][i];
        trinodes[3][i] = currpts[0][i];
        trinorms[1][i] = prevnormals[1][i];
        trinorms[2][i] = currnormals[1][i];
        trinorms[3][i] = currnormals[0][i];
      }
      else {
        trinodes[1][i] = currpts[0][i];
        trinodes[2][i] = currpts[1][i];
        trinodes[3][i] = prevpts[1][i];
        trinorms[1][i] = currnormals[0][i];
        trinorms[2][i] = currnormals[1][i];
        trinorms[3][i] = prevnormals[1][i];
      }
    }
    addOneBand(trinodes, 4, trinorms, onemap3dgeom, onecontourinfo);
    
  }
  else {                      /* curridx2[0] != curridx2[1] */
    
    /* up triangle */
    
    if (curridx1[0] != curridx1[1]) {
      /* this should not happen, just in case. */
      return;
    }
    
    which = curridx2[0];
    for (i = 0; i < 3; i++) {
      trinodes[0][i] = currpts[0][i];
      trinorms[0][i] = currnormals[0][i];
      if (curridx2[1] == (which + 1) % 3) {
        trinodes[1][i] = currpts[1][i];
        trinodes[2][i] = prevpts[1][i];
        trinodes[3][i] = prevpts[0][i];
        trinorms[1][i] = currnormals[1][i];
        trinorms[2][i] = prevnormals[1][i];
        trinorms[3][i] = prevnormals[0][i];
      }
      else {
        trinodes[1][i] = prevpts[0][i];
        trinodes[2][i] = prevpts[1][i];
        trinodes[3][i] = currpts[1][i];
        trinorms[1][i] = prevnormals[0][i];
        trinorms[2][i] = prevnormals[1][i];
        trinorms[3][i] = currnormals[1][i];
      }
    }
    addOneBand(trinodes, 4, trinorms, onemap3dgeom, onecontourinfo);
  }
return;
}

/* the following is a five-point band. */


if (previdx2[0] == previdx2[1] && curridx1[0] == curridx1[1]) {
  /* this situation maybe not exist, since the contour line is
  * in ascending order.
  */
  swapCurrPrev();
  ifSwap = true;
}

if (previdx1[0] == previdx1[1] && curridx2[0] == curridx2[1]) {
  which = previdx2[0];
  for (i = 0; i < 3; i++) {
    trinodes[0][i] = prevpts[0][i];
    trinorms[0][i] = prevnormals[0][i];
    if (previdx1[0] == (which + 1) % 3) {
      trinodes[1][i] = prevpts[1][i];
      trinorms[1][i] = prevnormals[1][i];
      if (curridx2[0] == (which + 2) % 3) {
        if (curridx1[0] == previdx1[0]) {
          trinodes[2][i] = currpts[0][i];
          trinodes[3][i] = currpts[1][i];
          trinorms[2][i] = currnormals[0][i];
          trinorms[3][i] = currnormals[1][i];
        }
        else {
          trinodes[2][i] = currpts[1][i];
          trinodes[3][i] = currpts[0][i];
          trinorms[2][i] = currnormals[1][i];
          trinorms[3][i] = currnormals[0][i];
        }
        trinodes[4][i] = pts[nodes[which]][i];
        trinorms[4][i] = onemap3dgeom->ptnormals[nodes[which]][i];
        
      }
      else {                  /* curridx2[0] == which */
        
        trinodes[2][i] = pts[nodes[(which + 2) % 3]][i];
        trinorms[2][i] = onemap3dgeom->ptnormals[nodes[(which + 2) % 3]][i];
        if (curridx1[0] == (which + 2) % 3) {
          trinodes[3][i] = currpts[0][i];
          trinodes[4][i] = currpts[1][i];
          trinorms[3][i] = currnormals[0][i];
          trinorms[4][i] = currnormals[1][i];
        }
        else {
          trinodes[3][i] = currpts[1][i];
          trinodes[4][i] = currpts[0][i];
          trinorms[3][i] = currnormals[1][i];
          trinorms[4][i] = currnormals[0][i];
        }
      }
    }
    else {                    /* previdx1[0] == (which+2)%3 */
      
      if (curridx2[0] == (which + 1) % 3) {
        trinodes[1][i] = pts[nodes[which]][i];
        trinorms[1][i] = onemap3dgeom->ptnormals[nodes[which]][i];
        if (curridx1[0] == which) {
          trinodes[2][i] = currpts[0][i];
          trinodes[3][i] = currpts[1][i];
          trinorms[2][i] = currnormals[0][i];
          trinorms[3][i] = currnormals[1][i];
        }
        else {
          trinodes[2][i] = currpts[1][i];
          trinodes[3][i] = currpts[0][i];
          trinorms[2][i] = currnormals[1][i];
          trinorms[3][i] = currnormals[0][i];
        }
      }
      else {                  /* curridx2[0] == which */
        
        if (curridx1[0] == (which + 2) % 3) {
          trinodes[1][i] = currpts[0][i];
          trinodes[2][i] = currpts[1][i];
          trinorms[1][i] = currnormals[0][i];
          trinorms[2][i] = currnormals[1][i];
        }
        else {
          trinodes[1][i] = currpts[1][i];
          trinodes[2][i] = currpts[0][i];
          trinorms[1][i] = currnormals[1][i];
          trinorms[2][i] = currnormals[0][i];
        }
        trinodes[3][i] = pts[nodes[(which + 1) % 3]][i];
        trinorms[3][i] = onemap3dgeom->ptnormals[nodes[(which + 1) % 3]][i];
      }
trinodes[4][i] = prevpts[1][i];
trinorms[4][i] = prevnormals[1][i];
    }
  }
addOneBand(trinodes, 5, trinorms, onemap3dgeom, onecontourinfo);
}
if (ifSwap)
swapCurrPrev();
}

/*======================================================================*/

/* check the last band in a triangle */
void TriContBandStruct::checkLastBand(float * /*pot[3] */ , float **trinodes, long nodes[3],
                                      float **trinorms, Map3d_Geom * onemap3dgeom, Contour_Info * onecontourinfo)
{
  int i, which;
  float** pts = onemap3dgeom->points[onemap3dgeom->geom_index];
  
  //if (pot) 1;
  
  if (previdx1[0] == previdx1[1]) {
    /* a 4-points polygon */
    which = previdx2[0];
    for (i = 0; i < 3; i++) {
      trinodes[0][i] = prevpts[0][i];
      trinorms[0][i] = prevnormals[0][i];
      if (previdx1[0] == (which + 1) % 3) {
        trinodes[1][i] = prevpts[1][i];
        trinodes[2][i] = pts[nodes[(which + 2) % 3]][i];
        trinodes[3][i] = pts[nodes[which]][i];
        trinorms[1][i] = prevnormals[1][i];
        trinorms[2][i] = onemap3dgeom->ptnormals[nodes[(which + 2) % 3]][i];
        trinorms[3][i] = onemap3dgeom->ptnormals[nodes[which]][i];
      }
      else {                    /* previdx1[0] == (which+2)%3 */
        trinodes[1][i] = pts[nodes[which]][i];
        trinodes[2][i] = pts[nodes[(which + 1) % 3]][i];
        trinodes[3][i] = prevpts[1][i];
        trinorms[1][i] = onemap3dgeom->ptnormals[nodes[which]][i];
        trinorms[2][i] = onemap3dgeom->ptnormals[nodes[(which + 1) % 3]][i];
        trinorms[3][i] = prevnormals[1][i];
      }
    }
    addOneBand(trinodes, 4, trinorms, onemap3dgeom, onecontourinfo);
  }
else {                        /* previdx1[0] != previdx1[1] */

/* a triangle */
which = previdx1[0];
for (i = 0; i < 3; i++) {
  trinodes[0][i] = prevpts[0][i];
  trinorms[0][i] = prevnormals[0][i];
  if (previdx1[1] == (which + 1) % 3) {
    trinodes[1][i] = prevpts[1][i];
    trinodes[2][i] = pts[nodes[(which + 2) % 3]][i];
    trinorms[1][i] = prevnormals[1][i];
    trinorms[2][i] = onemap3dgeom->ptnormals[nodes[(which + 2) % 3]][i];
  }
  else {                    /* previdx1[1] == (which+2)%3 */
    trinodes[1][i] = pts[nodes[(which + 1) % 3]][i];
    trinodes[2][i] = prevpts[1][i];
    trinorms[1][i] = onemap3dgeom->ptnormals[nodes[(which + 1) % 3]][i];
    trinorms[2][i] = prevnormals[1][i];
  }
}
addOneBand(trinodes, 3, trinorms, onemap3dgeom, onecontourinfo);
}
}
