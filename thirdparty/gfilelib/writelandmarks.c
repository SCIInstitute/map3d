/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cutil.h"
#include "landmarks.h"
/****************** Externals *****************************/
/****************** Prototypes ****************************/
/***************** The Code *******************************/
/*======================================================================*/

long WriteLandMarks( Land_Mark *landmarks, const long numlmarks, 
		     char *outfilename, long reportlevel )
{
/*** Write out a whole set of landmarks ***/

    long lmarknum;
    long error=0;
    FILE *luout;
/**********************************************************************/

    error = ClobberFile( outfilename );
    if ((luout = fopen(outfilename, "w") ) == NULL)
    {
	ReportError("WriteLandMarks", "error opening the file", ERR_FILE, "");
	return( error );
    }

    for ( lmarknum=0; lmarknum<numlmarks; lmarknum++ )
    {
	error = WriteOneLandMark( luout, &landmarks[lmarknum], reportlevel );
	if ( error < 0 ) return( error );
    }
    fclose( luout );
    printf(" Writelandmarks: wrote %d landmark sets\n"
	   " To the file: %s\n", numlmarks, outfilename );

    return( error );
}

/*======================================================================*/

long WriteOneLandMark( FILE *luout_p, Land_Mark *onelandmark, 
			long reportlevel )
{

 /*** Write out one landmark set to an already open file.  ***/

    long error = 0, i, segnum;
    char outstring[256], numstring[20];
/***********************************************************************/

    if ( reportlevel > 3 ) 
	printf(" Made if to WriterLandMark\n");

    if ( onelandmark->numsegs <= 0 ) 
    {
	ReportError( "WriteOneLandMark", "No segments in this landmark",
		     ERR_MISC, "");
	return( ERR_MISC );
    }

 /*** The opening line: how many landmarks are there. ***/

    fprintf(luout_p, "%d\n", onelandmark->numsegs);

 /*** Now write the segments out. ***/

    for ( segnum=0; segnum < onelandmark->numsegs; segnum++ )
    {
	if ( onelandmark->segs[segnum].type > 0 )
	{
	    sprintf(outstring, "%3d %s %d",
		    segnum+1, clmark[onelandmark->segs[segnum].type-1], 
		    onelandmark->segs[segnum].numpts);

	    if ( onelandmark->segs[segnum].segnumval >= 0 )
	    {
		sprintf(numstring," %d ", 
			onelandmark->segs[segnum].segnumval );
		strcat( outstring, numstring );
	    }
            if (onelandmark->segs[segnum].color[0] != -1) {
                sprintf(numstring," %d %d %d", 
			onelandmark->segs[segnum].color[0],
			onelandmark->segs[segnum].color[1],
			onelandmark->segs[segnum].color[2]);
                strcat( outstring, numstring );
            }

 /*** Send the completed string for this line out now. ***/

	    fprintf( luout_p, "%s\n", outstring );

 /*** Now write all the points for thsi segment.  ***/

	    for (i=0; i<onelandmark->segs[segnum].numpts; i++)
	    {
		error = fprintf( luout_p, " %8.2f %8.2f %8.2f %6.2f %s\n",
				 onelandmark->segs[segnum].pts[i][CUTIL_X],
				 onelandmark->segs[segnum].pts[i][CUTIL_Y],
				 onelandmark->segs[segnum].pts[i][CUTIL_Z],
				 onelandmark->segs[segnum].rad[i],
                                 onelandmark->segs[segnum].labels[i]);
		if ( error < 0 ) 
		{
		    fprintf(stderr," In WriteLandMarkFile: \n"
			    " Error writing segment #%d point #%d\n",
			    segnum+1, i+1);
		    return( error );
		}
	    }
	} else {
	    printf(" Writelandmarks: landmark segment %d has type = %d\n"
		   " So we do not write it out\n",
		   segnum+1, onelandmark->segs[segnum].type );
	}
    }
    return( error );
}
