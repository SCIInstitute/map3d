/*** 
  filename: writechannelsfile.c
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
long WriteChannelsFile( Surf_Geom *surfgeom )
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
	ReportError( "WriteChannelsFile", instring, ERR_FILE, "");
	return( ERR_FILE );
    }
    
    fprintf( luout_p," %d channels\n", surfgeom->numpts );
    for ( i=0; i < surfgeom->numpts; i++ ) {
	fprintf( luout_p, "%5d %5d \n", 
		i+1, surfgeom->channels[i]+1);
    }
    
    fclose( luout_p );
    return( error );
}
    
