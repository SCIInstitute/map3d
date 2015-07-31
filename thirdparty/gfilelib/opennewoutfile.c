 /*** 
   Filename: opennewoutfile.c
 ***/
/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geomlib.h"
#include "cutil.h"
static time_t thetime = 0;
#include <time.h>
/***************** Prototypes *****************************/
/***************** The Code *******************************/
/*======================================================================*/
long OpenNewOutfile( FileInfoPtr *luout_p, char *outfilename, 
			     long reportlevel )
{
 /*** Open a new output file (and close any existing ones. 

   luout_p	pointer to the graphicsio file, if not zero, then we assume
		we have an open file and close it first
   outfilename	prompted as new filename and updated here
   reportlevel	control level of reporting
***/
    long error=0;
    char cthetime[40], charuser[40], *cuser_p;
    char mainheader[256];
    long errorlevel = 1;
/**********************************************************************/

 /*** If we have a file already open, close it now. ***/

    if ( *luout_p ) closefile_( *luout_p );

 /*** Now get the new filename ***/

    
    ReadFilename( "output .geom", outfilename );
    while ( strlen(outfilename) < 1 )
    {
	printf(" Filename is blank, try again\n");
	ReadFilename( "output .geom", outfilename );
    }
    CheckExtension( outfilename, "geom" );
    if ( ( error = ClobberFile ( outfilename ) ) < 0 )
    {
	ReportError("OpenNewOutfile", "Error getting filename", ERR_FILE, "" );
	return(ERR_FILE);
    }

 /*** Open the output file. ***/

    if ( ( error = createfile_( (char*) outfilename, FILETYPE_GEOM,
			       errorlevel, luout_p)) < 0 )
    {
	ReportError("OpenNewOutfile", "Error open file", error, "" );
	return(error);
    }

 /*** Get the system time and use it to make the experiment id. 
 ***/

    thetime = time( NULL );
    if ( reportlevel )
	printf(" Got the system time as %d\n", thetime);
    sprintf( cthetime, "%d", thetime );
    setexpid_( *luout_p, cthetime );

    strcpy( mainheader," Geometry created by user ");
    cuser_p = charuser;
    cuser_p = getenv( "USER" );
    if ( reportlevel )
	printf(" User here is found as %s\n", cuser_p);
    strcat( mainheader, cuser_p );
    strcat( mainheader, " Date/Time: ");
    if ( reportlevel )
	printf(" The charcter time string is %s\n", ctime( &thetime) );
    strcat( mainheader, ctime( &thetime ) );
    if ( reportlevel )
	printf("Main header for the file is %s\n", mainheader);
    if (( error = settext_( *luout_p, mainheader ) )< 0 )
    {
	ReportError("OpenNewOutfile", "Error setting main header", error, "");
	return(error);
    }
    return( error );
}
