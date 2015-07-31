/*** 
Filename: readlmarkfile.c
Author: Rob MacLeod
   Last update: Mon Nov 10 15:54:37 1997
     - added two new elements to the the landmark data structure
   Last update: Mon Jun 20 13:09:22 MDT 1994
     - added the plane as a valid landmark to mark a slice through
       some geometry.
   Last update: Sat Apr  3 09:58:07 MST 1993
     - generalized the concept to that of landmarks and set up 
       a new scheme to read the files.
   Last update: Mon Nov 23 09:08:00 MST 1992
     - updated to use dynamic memory allocation
   Last update: Sun Oct  4 18:00:10 MDT 1992
     - still tuning, found error in multiple surface reads
   Last update: Thu Oct  1 22:04:56 MDT 1992
     - added option to read both types of coronary data files
   Last update: Wed Sep 30 23:04:41 MDT 1992
     - updated to reflect new type of coronary file format. Now we
       have a set of points, each with its own radius.
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "landmarks.h"
#include "cutil.h"
static long npts=0;
/**********************************************************************/
/***                 The Code                                       ***/
/**********************************************************************/
/*======================================================================*/

long ReadLandMarkFile (char filename[], Land_Mark *onelandmark, 
		       long reportlevel)
{
/*** 

  Read in a set of points that define landmarks.  These are not just
  coronaries, but can also be occlusions, stimulus needles or other
  goodies.

 Input:
 	filename    name of the coronary artery file to open.
	surfnum	    current surface number
	onelandmark pointer to the land mark structure for this surface
         
 Return:
        error       =1 for successful reading of an old format coronary file.
	    	    =2 for success with a new format file.
	    	    otherwise, error of some sort.
 ***/

    long i, j_start, pnum, firstpnum;
    long error, ncorpts, nnewcorpts, segnum, segnumval;
    long pntnum;
    double distance, dist_threshold=0;
    char instring[80], lmstring[20], *instring_p, *atoken_p;
    char newfilename[100];
    char sepchars[] = " ,\n";
    FILE *luin_p;
/***********************************************************************/

    if( reportlevel )
	fprintf(stderr,"In ReadLMarkFile\n ");

    error = 2;

 /* First, open the file. */
    
    if ((luin_p = fopen(filename, "r")) == NULL) 
    {
	strcpy( newfilename, filename );
	StripExtension( newfilename );
	strcat( newfilename, ".lmarks");
	if ((luin_p = fopen(newfilename, "r")) == NULL) 
	{
	    fprintf(stderr, "In readlmarkfile.c: error opening landmark file\n"
		    " Could not open either %s or %s \n", 
		    filename, newfilename);
	    return ERR_FILE;
	}
    }
    
 /*** Now, read in the contents of the file. ***/

 /*** First line: the number of landmark segments to read ***/

    if ( ReadLine( instring, luin_p ) == NULL )
    {
	fprintf(stderr,"**** Readlmarkfile Error: end of file hit "
		" at start of file"
		" ***\n");
	return -5;
    }
	
    if (sscanf(instring, "%i", &onelandmark->numsegs) != 1) 
    {
	fprintf(stderr,"*** In readlmarkfile.c: error in first "
		"line of read ***\n");
	error = -10;
	return error;
    }

 /*** Set up the memory to hold the segments of the landmark  ***/

    if (( onelandmark->segs = (LandMarkSeg *)
	 calloc( (size_t) onelandmark->numsegs, sizeof(LandMarkSeg) )) 
	== NULL)
    {
	printf("*** ReadLandMarkfile: error getting memory\n");
	return( ERR_MEM );
    }

 /*** Now the loop that reads all the Landmarks. ***/
    
    for( segnum=0; segnum < onelandmark->numsegs; segnum++ )
    {
      long optional[4] = {-1, -1, -1, -1};
      int optionalIndex = 0;
	if ( reportlevel > 2 )
	    printf(" segnum = %d resetting npts to 0\n", segnum);
	onelandmark->segs[segnum].segnum = segnum;
	
	npts = 0;
	if( ReadLine( instring, luin_p ) == NULL )
	{
	    fprintf(stderr,"+++ (readlmarkfile.c)\n"
		    "+++ End of file hit before expected\n"
		    "+++ Will take the %d landmarks we have already\n",
		    segnum);
	    onelandmark->numsegs = segnum;
	    return error;
	}

 /*** Read through the line token by token and permit some flexibility
      of the contents of this line, allowing the segnumval and setstring
      parameters to be optional. ***/

	instring_p = instring;
	atoken_p = strtok( instring_p, sepchars );
	i = strtol( atoken_p, NULL, 10 );
	strcpy(lmstring, strtok( NULL, sepchars ) );
	if ( lmstring == NULL )
	{
	    ReportError( "ReadLMarkFile", "reading landmark type string",
			 ERR_FILE, instring);
	    return( ERR_FILE );
	}
	if ( ( atoken_p = strtok( NULL, sepchars ) ) == NULL )
	{
	    ReportError( "ReadLMarkFile", "reading number of landmark points",
			 ERR_FILE, instring);
	    return( ERR_FILE );
	}
	ncorpts = strtol( atoken_p, NULL, 10 );
        // allow for either segnumval, a color triple, or both
        while ( (atoken_p = strtok( NULL, sepchars)) != NULL && 
		optionalIndex < 4) {
	    optional[optionalIndex] = (short) strtol( atoken_p, NULL, 10 );
            optionalIndex++;
        }

        if (optionalIndex == 1 || optionalIndex == 4)
          segnumval = optional[0];
        else
          segnumval = -1;

        {
          int c;
          for (c = 0; c < 3; c++) {
            onelandmark->segs[segnum].color[c] =
          (short) optional[c + (optionalIndex != 3?1:0)];
          }
        }
	
 /*
	if (sscanf(instring, "%d %s %d %d %s", &i, instring, &ncorpts) != 3) 
	{
	    fprintf(stderr,"*** In readlmarkfile.c: error reading number of"
		    " points and landmark type in coronary #%d ***\n", 
		    segnum+1);
	    fprintf(stderr,"*** Each landmark must be preceeded by a line"
		    " that tells the segment number,"
		    " the type of landmark it is\n"
		    " and number of points\n");
	    error = -10;
	    return error;
	}
 */
	if ( i != segnum+1 )
	    fprintf(stderr,"+++ (readlmarkfile.c): the file says this is"
		    " segment %d\n"
		    "+++ But I have it as number %d\n", i, segnum+1);

	onelandmark->segs[segnum].numpts = ncorpts;
	onelandmark->segs[segnum].segnumval = segnumval;
	
 /*** Now some pointer gymnastics to get rid of leading blanks
      and terminate the string properly, and convert it to uppercase.***/

	/*	lmstring_p = &instring[0];
	lmstring_p += strspn( lmstring_p, " ");
	len = strcspn( lmstring_p, "\n#" );
	*(lmstring_p+len) = '\0';
	strncpy( lmstring, lmstring_p, (size_t) len+1);
	*/
	Uppercase( lmstring );

 /*** Now see if the string matches the legal values. ***/
	    
	if( strncmp( lmstring, "CORONAR", 6 ) == 0 ||
	    strncmp( lmstring, "ARTER", 5) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_COR;
	}
	else if ( strncmp( lmstring, "OCCLUS", 3 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_OCCLUS;
	}
	else if ( strncmp( lmstring, "STITCH", 5 ) == 0  ||
		  strncmp( lmstring, "CLOS", 4 ) == 0 ) 
	{
	    onelandmark->segs[segnum].type = LM_STITCH;
	}
	else if ( strncmp( lmstring, "STIM", 3 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_STIM;
	}
	else if ( strncmp( lmstring, "LEAD", 4 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_LEAD;
	}
	else if ( strncmp( lmstring, "PLANE", 5 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_PLANE;
	}
	else if ( strncmp( lmstring, "ROD", 3 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_ROD;
	}
	else if ( strncmp( lmstring, "PACENEEDLE", 6 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_PACENEEDLE;
	}
	else if ( strncmp( lmstring, "CATH", 4 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_CATH;
	}
	else if ( strncmp( lmstring, "FIBER", 4 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_FIBER;
	}
	else if ( strncmp( lmstring, "RECNEEDLE", 6 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_RECNEEDLE;
	}
	else if ( strncmp( lmstring, "CANNULA", 6 ) == 0 )
	{
	    onelandmark->segs[segnum].type = LM_CANNULA;
	} else 
	{
	    printf("** ReadLMarkfile:  With landmark string: %s\n"
		   "**      we cannot set the type\n", lmstring);
	    onelandmark->segs[segnum].type = -1;
	}
	
 /*** Set aside some memory for the coronary points. ***/

	if( reportlevel > 2 )
	    fprintf(stderr," Allocating space for points "
		    "in landmarks\n"
		    " We want space for %d points\n",
		    ncorpts );

	if( ( onelandmark->segs[segnum].pts = Alloc_fmatrix( ncorpts, 3 )
	      ) == NULL )
	{
	    printf("*** ReadLandmarkFile: error getting memory for %d pts\n",
		   ncorpts);
	    return (ERR_MEM);
	}
	if( ( onelandmark->segs[segnum].rad = (float *)
	     calloc((size_t) ncorpts, sizeof(float) ) ) == NULL )
	{
	    fprintf(stderr,"*** Readlmarkfile, error allocating memory\n"
		    " for rads");
	    return (ERR_MEM);
	}
        if ( (onelandmark->segs[segnum].labels = Alloc_cmatrix( ncorpts, 40)
              ) == NULL )
	{
	    printf("*** ReadLandmarkFile: error getting memory for labels\n");
	    return (ERR_MEM);
	}
	
	
 /*** Check the first line of the points and see if we have a new
	  or old coronary file format. ***/
	
	if ( segnum == 0 )
	{
	    if( ReadLine( instring, luin_p ) == NULL )
	    {
		fprintf(stderr,"*** (readlmarkfile.c)\n"
			"*** End of file hit while reading landmark %d\n",
			segnum+1);
		return( ERR_FILE );
	    }
	    if (sscanf( instring, "%f %f %f %f %39s", 
		       &onelandmark->segs[segnum].pts[npts][CUTIL_X], 
		       &onelandmark->segs[segnum].pts[npts][CUTIL_Y],
		       &onelandmark->segs[segnum].pts[npts][CUTIL_Z], 
		       &onelandmark->segs[segnum].rad[npts],
                       onelandmark->segs[segnum].labels[npts]) >= 4)
	    {
		if (reportlevel > 2) 
		    fprintf(stderr," Landmark file seems to be in "
			    "proper format\n");
	    }
	    else if (sscanf( instring, "%f %f", 
			    &onelandmark->segs[segnum].pts[npts][CUTIL_X],
			    &onelandmark->segs[segnum].pts[npts][CUTIL_Y]) == 2)
	    {
		printf(" This is old format Coronary file\n"
		       " Redo the file !!!\n");
		onelandmark->numsegs = 0;
		return( ERR_FILE );
	    } else
	    {
		fprintf(stderr,"*** Error in Readlmarkfile since file does not"
			" fit a known format ***\n");
		fprintf(stderr," Line reads %s", instring);
		fprintf(stderr," So problems here\n");
		return(-1);
	    }
	    j_start = 1;
	    npts++;
	} else
	    j_start = 0;

 /*** Now get on with reading the file. ***/

	firstpnum = npts;
	nnewcorpts = ncorpts;

	for( pntnum=j_start; pntnum<ncorpts; pntnum++ )
	{
	    if( ReadLine( instring, luin_p ) == NULL )
	    {
		fprintf(stderr,"**** (readlmarkfile.c)\n"
			"**** End of file hit while reading landmark %d\n"
			"**** and point %d\n", segnum+1, pntnum+1);
		return( ERR_FILE );
	    }
	    if (sscanf( instring, "%f %f %f %f %39s", 
		       &onelandmark->segs[segnum].pts[npts][CUTIL_X], 
		       &onelandmark->segs[segnum].pts[npts][CUTIL_Y],
		       &onelandmark->segs[segnum].pts[npts][CUTIL_Z], 
		       &onelandmark->segs[segnum].rad[npts],
                       onelandmark->segs[segnum].labels[npts])
		< 4)
	    {
		fprintf(stderr,"*** In readlmarkfile: error reading points"
			" at coronary #%d and point #%d ***\n",
			segnum+1, pntnum+1);
		fprintf(stderr,"*** instring reads %s", instring);
		error = -20;
		return (error);
	    }
	    if ( pntnum == j_start )
		dist_threshold = ( onelandmark->segs[segnum].pts[npts][CUTIL_X] *
				   onelandmark->segs[segnum].pts[npts][CUTIL_X] +
				   onelandmark->segs[segnum].pts[npts][CUTIL_Y] *
				   onelandmark->segs[segnum].pts[npts][CUTIL_Y] +
				   onelandmark->segs[segnum].pts[npts][CUTIL_Z] *
				   onelandmark->segs[segnum].pts[npts][CUTIL_Z] ) /
		10.E8;
	    npts++;
	    
 /*** Check for doubles. ***/

	    for (pnum=firstpnum; pnum<npts-1; pnum++)
	    {
		if ( ( distance =
		      VECMAG3((onelandmark->segs[segnum].pts[pnum][CUTIL_X] - 
			       onelandmark->segs[segnum].pts[npts-1][CUTIL_X] ),
			      (onelandmark->segs[segnum].pts[pnum][CUTIL_Y] - 
			       onelandmark->segs[segnum].pts[npts-1][CUTIL_Y] ),
			      (onelandmark->segs[segnum].pts[pnum][CUTIL_Z] - 
			       onelandmark->segs[segnum].pts[npts-1][CUTIL_Z] ) ) )
		    < dist_threshold ) {
		    fprintf(stderr,"+++ ReadLmarkfile in segment %d"
			    ": You have doubled"
			    " points dude!\n"
			    " Point %d has a double"
			    " %f, %f, %f \n"
			    " And distance between them is %f\n",
			    segnum+1, pntnum+1, 
			    onelandmark->segs[segnum].pts[npts-1][CUTIL_X],
			    onelandmark->segs[segnum].pts[npts-1][CUTIL_Y],
			    onelandmark->segs[segnum].pts[npts-1][CUTIL_Z],
			    distance);
		    npts--;
		    nnewcorpts--;
		}
	    }
	    if ( reportlevel > 3 )
	    {
		if ( npts < 50 )
		{
		    fprintf(stderr," Line %d reads %s\n"
			    " and returns %f %f %f and %f\n",
			    npts, instring, 
			    onelandmark->segs[segnum].pts[npts-1][CUTIL_X],
			    onelandmark->segs[segnum].pts[npts-1][CUTIL_Y],
			    onelandmark->segs[segnum].pts[npts-1][CUTIL_Z],
			    onelandmark->segs[segnum].rad[npts-1]);
		}
	    }
	}
	if ( nnewcorpts != ncorpts ) {
	    onelandmark->segs[segnum].numpts = nnewcorpts;
	}

	if ( reportlevel > 3 )
	{
	    fprintf(stderr," Coronary points and radii from segment %d\n",
		    segnum);
	    for( i=0; i < onelandmark->segs[segnum].numpts; i++ )
	    {
		fprintf(stderr," %d %f %f %f %f \n",
			i, 
			onelandmark->segs[segnum].pts[i][CUTIL_X], 
			onelandmark->segs[segnum].pts[i][CUTIL_Y],
			onelandmark->segs[segnum].pts[i][CUTIL_Z], 
			onelandmark->segs[segnum].rad[i]);
	    }
	}
    }
    
    fclose(luin_p);
    
    return (error);
}
