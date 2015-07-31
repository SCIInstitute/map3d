 /*** 
   File: writeelementsfile.c
   Last update: Thu Nov 13 12:41:09 1997
     - created
   Last update: Tue Jan 20 12:52:07 1998 by Rob MacLeod
     - fixed -- it should never have worked as it was???
 ***/
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <string.h>
#else
#include <strings.h>
#endif
#include "geomlib.h"
#include "cutil.h"

/*======================================================================*/
long WriteElementsFile( Surf_Geom *surfgeom )
{
    long i, j, error = 0;
    char instring[200], outfilename[100];
    FILE *luout_p;
/*********************************************************************/    
 /*** Open the file. ***/

    strcpy( outfilename, surfgeom->filename );
    ClobberFile( outfilename );
    strcpy( surfgeom->filename, outfilename );
    if ( error < 0 ) return( error );
    if ( (luout_p = fopen (outfilename, "w")) == NULL )
    {
	sprintf(instring,"Error opening file %s", outfilename); 
	ReportError( "WriteElementsFile", instring, ERR_FILE, "");
	return( ERR_FILE );
    }
    
    for ( i=0; i < surfgeom->numelements; i++ ) 
    {
	for (j=0; j < surfgeom->elementsize; j++ )
	{
	    fprintf( luout_p, "%6d ", 
		surfgeom->elements[i][j]+1 );
	}
	fprintf(luout_p, "\n");
    }
    
    fclose( luout_p );
    return( error );
}
