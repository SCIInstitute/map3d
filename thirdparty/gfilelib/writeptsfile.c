/*** 
  filename: writeptsfile.c
  author: Rob MacLeod
  Write a points file.

   Last update: Tue Mar  4 23:48:32 1997
     - made to fit new data structures for surface geom
   Last update: Thu Jun 15 12:36:37 MDT 1995
     - changes arguments to give one less level of indirection
   Last update: Thu May 25 15:43:46 MDT 1995
   - created
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
long WritePtsFile( Surf_Geom *surfgeom )
{
    long i, error = 0;
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
	ReportError( "WritePtsFile", instring, ERR_FILE, "");
	return( ERR_FILE );
    }
    
    for ( i=0; i < surfgeom->numpts; i++ ) {
	fprintf( luout_p, "%8.2f %8.2f %8.2f \n", 
		surfgeom->nodes[i][CUTIL_X],
		surfgeom->nodes[i][CUTIL_Y],
		surfgeom->nodes[i][CUTIL_Z]);
    }
    
    fclose( luout_p );
    return( error );
}
    
