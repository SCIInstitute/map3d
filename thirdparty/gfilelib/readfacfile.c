#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geomlib.h"
#include "cutil.h"

/*======================================================================*/
long ReadFacFile( Surf_Geom *onesurfgeom, long reportlevel )
{	
 /*** Read .pts file and return the contents in the nodes part of the
      Surf_Geom structure. 
 ***/
    long numelements, count, numread, i, j, error=0;
    FILE *luin_p;
    char *infilename, errstring[100];
    long **elements;
    int base = 1;
/**********************************************************************/   
    
    infilename = onesurfgeom->filename;
    StripExtension(infilename);
    strcat(infilename,".fac");

 /*** Open the file. ***/

    if ((luin_p = fopen(infilename, "r")) == NULL)
    {
	sprintf(errstring,"unable to open fac file %s", infilename);
	ReportError("ReadFacFile", errstring, ERR_FILE,"");
	return( ERR_FILE );
    }

 /*** Get the file length and the size of the memory we need. ***/

    numelements = GetFileLength( luin_p );
    rewind( luin_p );
    if ( numelements < 1 ) 
    {
	sprintf(errstring,"file %s has length = %d", infilename, numelements);
	ReportError("ReadFacFile", errstring, ERR_FILE,"");
	return( ERR_FILE );
    }
    if (( elements = Alloc_lmatrix( numelements, 3 ))
	== NULL )
    {
	ReportError("ReadFacFile", "error getting elements memory", 
		    ERR_MEM,"");
	return( ERR_MEM );
    }

 /*** Read the points from the file. ***/

    for (count = 0; fscanf(luin_p, "%d%d%d",
			   &elements[count][0], 
			   &elements[count][1],
			   &elements[count][2]) != EOF; count++)
      if (elements[count][0] == 0 || elements[count][1] == 0 || elements[count][2] == 0)
        base = 0;

 /*** Set up some new memory just in case we had fewer read than 
      there were lines in the file.  ***/

    numread = count;
    if (( onesurfgeom->elements = Alloc_lmatrix( numread, 3)) == NULL )
    {
	ReportError( "ReadFacFile", "error getting memory", ERR_MEM, "" );
	return( ERR_MEM );
    }
    /* allow user to specify 0-based files.  Default is 1-based, but 
       if there is a 0 in the file, make the whole file 0-based. 
       If there is a 0, don't subtract 1 */

    if (base == 1) {
      for (i=0; i<numread; i++)
      {
        for (j=0; j<3; j++) {
          onesurfgeom->elements[i][j] = elements[i][j]-1;
        }
      }
    }
    onesurfgeom->numelements = numread;
    onesurfgeom->elementsize = 3;
    free( elements );
    fclose( luin_p );
    if ( reportlevel ) 
	printf(" Read %d triangles from %s\n", count, infilename );
    return( error );
}
