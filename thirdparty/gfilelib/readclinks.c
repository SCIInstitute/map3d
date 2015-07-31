 /*** 
   Filename: readclinks.c

 ***/
/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "geomlib.h"
#include "cutil.h"
#include "links.h"
#define LEADLABELSIZE 10 /*** Max size of the label we put on leads ***/
/****************** Externals *****************************/

/****************** Prototypes ****************************/
long ChanneltoNode( long chlink, long *channels, long numpts );

/***************** The Code *******************************/

/*================================================================*/
Lead_Link *ReadChannelLinksFile( char *infilename, Surf_Geom *onesurfgeom,
				 long *numclinks )
{
 /*** Read the channellinks file 

   The purpose of the channelinks file is to mark subsets of leads by 
   channel number.  So an entry in this file would be:

   channelnum labelnum 

   where channelnum indicates the channel that should be marked
   with the value "labelnum".

   Once read in, we have to scan the channelnums for their
   associated points and set up the leadlinks array accordingly.

   Return: we return an array of structures that contain the lead link
           information from the file.

   NOTE: TO WORK PROPERLY, WE MUST RUN THIS FUNCTION ONLY AFTER THE CHANNELS
   INFORMATION IS AVAILABLE!!!!
***/

    long i, nclinks, nvalidlinks, noutleads, leadnum;
    long chlink;
    long pointnum;
    long *inleadlinks;
    char instring[100], errstring[100];
    char chlabel[100];
    char **leadlinklabels=NULL;
    FILE *luin_p;
    Lead_Link *leadlinks=NULL;
/******************************************************************/
 /*** Open the file. ***/

    if ((luin_p = fopen(infilename, "r")) == NULL)
    {
	sprintf(errstring,"unable to open glinks file %s", infilename);
	ReportError("ReadChannelLinksFile", errstring, ERR_FILE,"");
	return( NULL );
    }

  /*** Read in the file , setting up the channel links array.
  ***/
  
    fscanf( luin_p, "%ld %s \n", &nclinks, instring);
    fprintf(stderr,"In ReadChannelLinksFile\n");
    fprintf(stderr," First line read as %d %s\n", nclinks, instring);

 /*** Get some local memory for all the links. ***/

    if (( inleadlinks = (long *) 
	 calloc( (size_t) nclinks, sizeof(long) ))
	== NULL )
    {
	fprintf(stderr,"*** ReadChannelLinksFile: error getting memory"
		" for local leadlinks\n");
	return( NULL );
    }
    if (( leadlinklabels = (char **) 
	 calloc( (size_t) nclinks, sizeof(char *) ))
	== NULL )
    {
	fprintf(stderr,"*** ReadChannelLinksFile: error getting local memory"
		" for leadlinklabels\n");
	return( NULL );
    }

 /*** Now do the reading of the lead link file. ***/

    nvalidlinks = 0;
    for( leadnum=0; leadnum<nclinks; leadnum++ )
    {
	if ( fscanf( luin_p, "%ld %s \n",&chlink, chlabel ) != 2 ) 
	{
	    fprintf(stderr,"*** ReadChannelLinksFile: error reading"
		    " link #%d\n",
		    leadnum + 1);
	    return( NULL );
	}

 /*** Load the values into the structures.  ***/

	inleadlinks[leadnum] = ChanneltoNode( chlink, onesurfgeom->channels, 
			   onesurfgeom->numpts );
	if ( inleadlinks[leadnum] >= 0 ) nvalidlinks++;
	if (( leadlinklabels[leadnum] = (char *) 
	 calloc( (size_t) LEADLABELSIZE+1, sizeof(char) ))
	== NULL )
	{
	    fprintf(stderr,"*** ReadChannelLinksFile: error getting memory"
		    " for leadlinklabels lead #%d\n", leadnum+1);
	    return( NULL );
	}
	if ( strlen( chlabel ) <= LEADLABELSIZE ) 
	{
	    strcpy(leadlinklabels[leadnum], chlabel );
	} else {
	    strncpy(leadlinklabels[leadnum], chlabel, 
		   LEADLABELSIZE );
	    leadlinklabels[leadnum][LEADLABELSIZE] = '\0';
	    printf("+++ ReadChannelLinksFile: terminated leadlink label\n");
	}
    }
 /*** Now look for doubles or any other funky stuff. ***/

    for( leadnum=0; leadnum < nclinks; leadnum++)
    { 
	pointnum = inleadlinks[leadnum];
	if ( pointnum >= 0 ) 
	{
	    for( i=leadnum+1; i < nclinks; i++)
	    {
		if ( inleadlinks[i] == pointnum )
		{
		    fprintf(stderr,"+++ Warning in ReadChannelLinksFile\n"
			    "    point #%d is pointed at by lead #%d"
			    " and lead #%d\n", 
			    pointnum + 1, 
			    leadnum  + 1, 
			    i + 1);
		}
	    }
	    strcpy( chlabel, leadlinklabels[leadnum] );
	    for( i=leadnum+1; i < nclinks; i++)
	    {
		if (! strcmp( leadlinklabels[i],chlabel ) )
		{
		    fprintf(stderr,"+++ Warning in ReadChannelLinksFile\n"
			    "+++ leadlabel %s is found in entry #%d"
			    " and entry #%d of channel links file\n", 
			    chlabel, leadnum + 1, i + 1);
		}
	    }
	}
    }

 /*** Now load the subset of links that were actually found in this
      surface and load just those into the data structure. ***/

    if (( leadlinks = (Lead_Link *) 
	 calloc( (size_t) nvalidlinks, sizeof(long) ))
	== NULL )
    {
	fprintf(stderr,"*** ReadChannelLinksFile: error getting memory"
		" for leadlinks\n");
	return( NULL );
    }
    noutleads=0;
    for( leadnum=0; leadnum < nclinks; leadnum++)
    { 
	pointnum = inleadlinks[leadnum];
	if ( pointnum >= 0 ) 
	{
	    leadlinks[noutleads].leadnum = noutleads;
	    leadlinks[noutleads].nodelink = pointnum;
	    if (( leadlinks[noutleads].label = (char *) 
	    calloc( (size_t) strlen(leadlinklabels[leadnum])+1, sizeof(char) ))
		== NULL )
	    {
		fprintf(stderr,"*** ReadChannelLinksFile: error getting memory"
			" for leadlinklabels lead #%d\n", leadnum+1);
		return( NULL );
	    }
	    strcpy( leadlinks[noutleads].label, leadlinklabels[leadnum] );
	    noutleads++;
	}
    }

    free( leadlinklabels );
    free( inleadlinks );

    if ( noutleads != nvalidlinks )
    {
	printf("ReadChannelLinks error because noutleads = %d and"
	       " nvalidlinks = %d\n", noutleads, nvalidlinks);
	return( NULL );
    } else {
	printf(" For this surface there are %d valid channel links\n",
	       nvalidlinks);
    }
    *numclinks = nvalidlinks;
    return( leadlinks );
}


/*================================================================*/
long ChanneltoNode( long chlink, long *channels, long numpts )
{
 /*** Convert a channel number  to its assoicated node number. ***/

    long nodenum;
    chlink--;
    for( nodenum=0; nodenum < numpts; nodenum++ )
    {
	if ( chlink == channels[nodenum] )
	{
	    return( nodenum );
	}
    }
    return(-1);
}

