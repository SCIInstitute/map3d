#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geomlib.h"
#include "cutil.h"

/*======================================================================*/
long ReadPtsFile( Surf_Geom *onesurfgeom, long reportlevel )
{	
 /*** Read .pts file and return the contents in the nodes part of the
      Surf_Geom structure. 
      We assume that onesurfgeom is already set up and only alter the
      node and channels components.
 ***/
    long numnodes, numargs, count, i, j, error=0;
    FILE *luin_p;
    char *infilename, errstring[100], instring[256];
    char *char_p=NULL;
    float **nodes;
/**********************************************************************/   
    
    infilename = onesurfgeom->filename;

 /*** Open the file. ***/

    if ((luin_p = fopen(infilename, "r")) == NULL)
    {
	sprintf(errstring,"unable to open pts file %s", infilename);
	ReportError("ReadPtsFile", errstring, ERR_FILE,"");
	return( ERR_FILE );
    }

 /*** Get the first line and parse it to see what format we have.
      Is is possible that the firstline contains the number of points
      in the file. 
 ***/

    fgets(instring, 100, luin_p);
    char_p = strtok( instring, " \t" );
    numnodes = strtol( char_p, NULL, 10 );
    numargs=1;
    while ( (char_p = strtok( NULL, " \t" ) ) != NULL )
    {
	numargs++;
    }

 /*** See if we have one argument in the line; if not, then
      we assume a "normal" pst file and have to scan it to get
      the number of lines it contains. 
 ***/

    if ( numargs == 1 )
    {
	printf(" Funky .pts file format--first line contains 1 value = %d\n"
	       " so we assume it is the number of nodes\n",
	       numnodes );
    } else {
	rewind( luin_p );
	numnodes = GetFileLength( luin_p );
	rewind( luin_p );
    }

    if ( numnodes < 1 ) 
    {
	sprintf(errstring,"file %s has length = %d", infilename, numnodes);
	ReportError("ReadPtsFile", errstring, ERR_FILE,"");
	return( ERR_FILE );
    }
    if (( nodes = Alloc_fmatrix( numnodes, 3 ))
	== NULL )
    {
	ReportError("ReadPtsFile", "error getting nodes memory", ERR_MEM,"");
	return( ERR_MEM );
    }

 /*** Read the points from the file. ***/

    for (count = 0; fscanf(luin_p, "%f%f%f",
			   &nodes[count][CUTIL_X], 
			   &nodes[count][CUTIL_Y],
			   &nodes[count][CUTIL_Z]) != EOF; count++);


 /*** Now set up just enough memory and transfer points. ***/

    if ( count < 1 ) 
    {
	sprintf(errstring,"file %s had %d points", infilename, count);
        ReportError("ReadPtsFile", errstring, ERR_FILE,"");
        return( ERR_FILE );
    }

    error = SetupSurfPoints( onesurfgeom, count );
    if ( error < 0 ) return( error );

    for (i=0; i<count; i++)
    {
	for(j=0; j<3; j++)
	{
	    onesurfgeom->nodes[i][j] = nodes[i][j];
	}
    }
    free( nodes );

 /*** Set up default channels arrays.  These may be
      overwritten later but this way, there is always a valid set

 ***/

    for ( i=0; i<count; i++ )
    {
	onesurfgeom->channels[i] = i;
    }

    fclose( luin_p );
    if ( reportlevel )
	printf(" Read %d points from %s\n", count, infilename );
    return( error );
}

