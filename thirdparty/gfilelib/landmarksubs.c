/*** 
Filename: landmarksubs.c
Author: Rob MacLeod

Some functions to handle landmarks.
   Last update: Wed Aug  6 10:56:50 1997
     - some fixes in normal debugging
   Last update: Tue Jul 22 18:12:55 1997
     - created
***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "geomlib.h"
#include "cutil.h"
#include "landmarks.h"
/**********************************************************************/
/***                 The Code                                       ***/
/**********************************************************************/

LandMarkSeg *AddALandMarkSeg( LandMarkSeg *segs, long *numsegs, 
			    long type, long numpts )

{
 /*** Add the memory necessary for a new segment to an existing array of
      landmark segments. 

      segs	pointer to existing array of segment
      numsegs	current number of segments in the array -- update this
      type	segment type if known (<0 otherwise)
      numpts    number of points in this segment (0 if unknown)

***/

    long lnumsegs, lsegnum;
/**********************************************************************/
    lnumsegs = *numsegs;

 /*** See if we have any memory allocated so far and alloc or realloc
      depending on the result.  ***/

    if ( segs == NULL ) 
    {
	lnumsegs = 1;
	if ( ( segs = (LandMarkSeg *) 
	      calloc( (size_t) (lnumsegs), sizeof(LandMarkSeg)) ) == NULL )
	{
	    ReportError("AddaLandmarkSeg", "seg memory allocation", 0, "");
	    return( NULL );
	}
    } else {
	lnumsegs++;
	if ( ( segs = (LandMarkSeg *) 
	      realloc( segs, (size_t) 
		       lnumsegs * sizeof(LandMarkSeg)) ) == NULL )
	{
	    ReportError("AddaLandmarkSeg", "seg realloc error", 0, "");
	    return( NULL );
	}
    }
    
 /*** Set up some parameters. ***/

    lsegnum = lnumsegs - 1;
    segs[lsegnum].segnum = lsegnum;
    segs[lsegnum].type = type;
    segs[lsegnum].numpts = numpts;
    segs[lsegnum].segnumval = -1;
    segs[lsegnum].color[0] = segs[lsegnum].color[1] = segs[lsegnum].color[2] = -1;
    if ( numpts > 0 ) 
    {
	if (( segs[lsegnum].pts = Alloc_fmatrix( numpts, 3 ) ) == NULL )
	{
	    ReportError("AddaLandmarkSeg", "pts memory allocation", 0, "");
	    return( NULL );
	}
	if (( segs[lsegnum].rad = (float *) calloc( (size_t) numpts,
						 sizeof(float) ) ) == NULL )
	{
	    ReportError("AddaLandmarkSeg", "rad memory allocation", 0, "");
	    return( NULL );
	}
	if (( segs[lsegnum].labels = Alloc_cmatrix( numpts, 40 ) ) == NULL )
	{
	    ReportError("AddaLandmarkSeg", "label memory allocation", 0, "");
	    return( NULL );
	}
    } else {
	segs[lsegnum].pts = NULL;
	segs[lsegnum].rad = NULL;
	segs[lsegnum].labels = NULL;
    }

 /*** Send back updated values ***/

    *numsegs = lnumsegs;
    return( segs );
}

/*====================================================================*/
Land_Mark *DefALandMarkSurf( long surfnum )
{
 /*** Define a Land_Mark structure and return a pointer to it.
   Input:
    surfnum 	    the surface number to which this landmark surface
    	    	    should be associated
   Return:
    *Land_Mark      pointer to a single surface with of landmarks
***/
    Land_Mark *onelandmark;
/**********************************************************************/

 /*** Allocate space for this landmark. ***/

    if ( ( onelandmark = (Land_Mark *) 
	  calloc( (size_t) (1), sizeof(Land_Mark)) ) == NULL )
    {
	fprintf(stderr,"*** DefALandMark: error getting first memory\n");
	return( NULL );
    }

 /*** Set up some parameters. ***/

    onelandmark->surfnum    = surfnum;
    onelandmark->numsegs    = 0;
    onelandmark->arrowradius= 0.0;
    return (onelandmark);
}

