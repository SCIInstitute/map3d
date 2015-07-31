#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cutil.h"

/*================================================================*/

long **ReadLinksFile( char *infilename, long *numoflinks )
{
 /*** Read a links file with two columns of numbers 
   Return a N * 2 array of values.
***/

    long i, numlinks, leadnum, linknum;
    long pointnum, link1, link2;
    long **links;
    char instring[100], errstring[100];
    FILE *luin_p;
/******************************************************************/
  
 /*** Open the file. ***/

    if ((luin_p = fopen(infilename, "r")) == NULL)
    {
	sprintf(errstring,"unable to open pts file %s", infilename);
	ReportError("ReadLinksFile", errstring, 0,"");
	return( NULL );
    }

 /*** Get the number of links for the memory we need. ***/

    fscanf( luin_p, "%ld %s \n", &numlinks, instring);
    fprintf(stderr,"In ReadLinksFile\n");
    fprintf(stderr," First line read as %d %s\n", numlinks, instring);

 /*** Get some memory for these links. ***/

    if ( numlinks < 1 ) 
    {
	sprintf(errstring,"file %s has %d links", infilename, numlinks);
	ReportError("ReadLinksFile", errstring, 0,"");
	return( NULL );
    }
    if ( ( links = Alloc_lmatrix( numlinks, 2 ) ) == NULL )
    {
	ReportError("ReadLinksFile", "error getting links memory", 0,"");
	return( NULL );
    }
    
 /*** Now do the reading of the lead link file. ***/

    for( leadnum=0; leadnum<numlinks; leadnum++ )
    {
	if ( ReadLine( instring, luin_p ) == NULL )
	{
	    fprintf(stderr,"*** ReadLinksFile: error reading link #%d\n",
		    leadnum + 1);
	    return( NULL );
	}

 /*** Scan the line and check how many values are on each.  We expect
      two values for each line: ***/

	if ( sscanf( instring, "%d %d \n", &link1, &link2 ) != 2 ) 
	{
	    fprintf(stderr,"*** ReadLinksFile: error scanning line\n %s\n",
		    instring);
	    return( NULL );
	}
	
 /*** Load the values into the structures.  ***/

	links[leadnum][0] = link1 - 1;
	links[leadnum][1] = link2 - 1;

  /*** If there is a zero entry in the leadlink file, flag it by a -1
   for internal checking in the program.
   ***/

	if ( links[leadnum] < 0 )
	{
	    sprintf(errstring,"Leadlink #%d points to a zero point number",
		    leadnum+1);
	    ReportError("ReadLinksFile", errstring, 0, "");
	    return( NULL );
	}
    }

 /*** Now look for doubles or any other funky stuff. ***/

    for( linknum=0; linknum<2; linknum++ )
    {
	for( leadnum=0; leadnum < numlinks; leadnum++)
	{ 
	    pointnum = links[leadnum][linknum];
	    if ( pointnum >= 0 ) 
	    {
		for( i=leadnum+1; i < numlinks; i++)
		{
		    if ( links[i][linknum] == pointnum )
		    {
			fprintf(stderr,"+++ Warning in ReadLinksFile\n"
				"    point #%d is pointed at by link #%d"
				" and link #%d\n", 
				pointnum + 1, 
				leadnum  + 1, 
				i + 1);
		    }
		}
	    }
	}
    }
    *numoflinks = numlinks;
    return( links );
}

