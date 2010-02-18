 /*** 
   File: getsurflist.c
   Last update: Sat Nov  1 21:01:43 1997
     - extracted and set up as independent file

 ***/
/****************** Includes *****************************/
#include "geomlib.h"
#include "numseq.h"
#include "links.h"
#include "cutil.h"
/****************** Prototypes ****************************/

/***************** The Code *******************************/

/*======================================================================*/

long *GetSurfList( long surfnum, long numsurfs, long *numlistsurfs )
{

/*** Get a sequence of surface numbers. ***/
    long i, numerror=0;
    long *lsurflist = NULL;
    int *surflist, nlistsurfs;
    char instring[80], surfstring[100]="";
    NumSeq surfseq;
/**********************************************************************/

 /*** If we only have one surface, no need to ask. ***/

    if ( numsurfs <= 1 )
    {
	nlistsurfs = 1;
	lsurflist = (long *) calloc( (size_t) nlistsurfs, sizeof( long ));
	lsurflist[0] = 0;
	*numlistsurfs = nlistsurfs;
	return( lsurflist );
    }

 /*** Now form the list the regular way, using Ted's routines. ***/

    surfseq = NumSeqCreate();
    if ( strlen( surfstring ) < 1 )
	sprintf(surfstring,"%d", surfnum+1);
    printf(" Enter the sequence of surfaces you want (e.g., 1,2,4-5,7)\n"
	   " ranging from (from 1-%d)\n"
	   "  [def=%s] ? ", numsurfs, surfstring );
    fgets(instring, 70,  stdin );
    if ( strlen( instring ) > 0 ) 
	strcpy( surfstring,instring );
    while (NumSeqParse(surfseq, surfstring) && numerror < 10) 
    {
        printf("*** ? Illegal sequence\n"); 
        printf(" Enter the desired range again [def= %s] > ", surfstring);
        scanf("%s",instring);
        if (Trulen((const char*) instring) >= 0) 
	    strcpy( surfstring,instring );
        numerror++;
    }
    if (numerror >= 10) 
    {
	ReportError("GetSurflist", "Blow out trying to get legal string",
		    0, "");
        return ( NULL );
    }
    
 /*** Now convert the string into an array of surface numbers.  ***/

    NumSeqArray( surfseq, &surflist, &nlistsurfs );
    if ( surflist == NULL ) 
    {
	ReportError("GeturfList", "error converting sequence to list",
		    0, "");
	return( NULL );
    }

 /*** Now scan the list and make sure it makes sense, and shift
      values to start at 0. ***/

    lsurflist = (long *) calloc( (size_t) nlistsurfs, sizeof( long ));
    for (i=0; i<nlistsurfs; i++ )
    {
	if ( surflist[i] < 0 )
	    surflist[i] = 1;
	if ( surflist[i] > numsurfs )
	    surflist[i] = numsurfs;
	lsurflist[i] = surflist[i] - 1;
    }
    *numlistsurfs = nlistsurfs;
    free( surflist );
    NumSeqDestroy( surfseq );
    return( lsurflist );
}

