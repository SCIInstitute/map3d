 /*** 
   Filename: readglinks.c

 ***/
/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "geomlib.h"
#include "cutil.h"
#include "links.h"
/****************** Externals *****************************/

/****************** Prototypes ****************************/
void ConvertRCEString( char *rcestring, long *colnum, 
		       long *rownum, long *electrodenum );
/***************** The Code *******************************/

/*================================================================*/
Lead_Link *ReadGeomLinksFile( char *infilename, long *numglinks )
{
 /*** Read a geometry (.glinks) file 

   The purpose of the geometry links file is to connect channels in 
   one geometry with new channel numbers in a second geometry.
   There are two possible  formats for the glinks file:

# Comments and headings
n link_type_text
old-chnlnum1  new-chnlnum1  mux-chnlnum1 col-row-electrode1 local-chnlnum1
old-chnlnum2  new-chnlnum2  mux-chnlnum2 col-row-electrode2 local-chnlnum2
   .
   .

And 

# Comments and headings
n link_type_text
old-chnlnum1  new-chnlnum1  mux-chnlnum1 
old-chnlnum2  new-chnlnum2  mux-chnlnum2 
   .
   .

   where n is the number of links in the file.
   and link_type_text is just a text field for identification
   The order of the links determines the order of the nodes in the new geomtry.

   Return: we return an array of structures that contain the lead link
           information from the file.
	   We use the Lead_Link data structure by placing the 
	   old channel number in .nodelink and
	   new channel number in .channel
	   mux channel number in .lead
	   column in colnum
	   row in rownum
	   electrode in electrodenum
	   local-chnlnum in localchannel

***/

    long i, nglinks, nvalidlinks, noutleads, leadnum;
    long oldchan, newchan, muxchan;
    long reportlevel = 0;
    long colnum=-1, rownum=-1, electrodenum=-1, localchannel=-1;
    long *oldchans, *newchans, *muxchans;
    long *colnums, *rownums, *electrodenums, *localchannels;
    char instring[100], errstring[100], leadname[20];
    char rcestring[80];
    FILE *luin_p;
    Lead_Link *leadlinks=NULL;
/******************************************************************/

 /*** Open the file. ***/

    if ((luin_p = fopen(infilename, "r")) == NULL)
    {
	sprintf(errstring,"unable to open glinks file %s", infilename);
	ReportError("ReadGeomLinksFile", errstring, ERR_FILE,"");
	return( NULL );
    }

 /*** Look for comment lines. ***/
    
    ReadComments( luin_p, instring );
    
  /*** Read in the file, setting up the links arrays.
  ***/
  
    sscanf( instring, "%ld %s", &nglinks, leadname);
    if ( reportlevel ) 
    {
	fprintf(stderr,"In ReadGeomLinksFile\n");
	fprintf(stderr," First line read as %d %s\n", nglinks, leadname);
    }
 /*** Get some local memory for all the links. ***/

    if (( oldchans = (long *) 
	 calloc( (size_t) nglinks, sizeof(long) ))
	== NULL )
    {
	fprintf(stderr,"*** ReadGeomLinksFile: error getting memory"
		" for local oldchans\n");
	return( NULL );
    }
    if (( newchans = (long *) 
	 calloc( (size_t) nglinks, sizeof(long) ))
	== NULL )
    {
	fprintf(stderr,"*** ReadGeomLinksFile: error getting memory"
		" for local newchans\n");
	return( NULL );
    }

    if (( muxchans = (long *) 
	 calloc( (size_t) nglinks, sizeof(long) ))
	== NULL )
    {
	fprintf(stderr,"*** ReadGeomLinksFile: error getting memory"
		" for local muxchans\n");
	return( NULL );
    }
    colnums = (long *) calloc( (size_t) nglinks, sizeof(long) );
    rownums = (long *) calloc( (size_t) nglinks, sizeof(long) );
    electrodenums = (long *) calloc( (size_t) nglinks, sizeof(long) );
    localchannels = (long *) calloc( (size_t) nglinks, sizeof(long) );

 /*** Now do the reading of the geometry link link file. 
      We don't need a lot of what is in the file so ignore it.
 ***/

    nvalidlinks = 0;
    for( leadnum=0; leadnum<nglinks; leadnum++ )
    {
	strcpy(instring, ReadNextLine( luin_p ) );

 /*** Long format. ***/

	if ( sscanf( instring, "%d %d %d %s %d", 
		     &oldchan, &newchan, &muxchan, rcestring, &localchannel )
	     == 5 ) 
	{
	    ConvertRCEString( rcestring, &colnum, &rownum, &electrodenum );

 /*** Short format. ***/

	} else if ( sscanf( instring, "%d %d %d", 
		     &oldchan, &newchan, &muxchan )
	     == 3 ) 
	{
	    rownum = colnum = electrodenum = localchannel=0;
	} else {
	    sprintf(errstring,"error reading"
		    " link #%d\n", leadnum + 1);
	    ReportError("ReadGeomLinksFile",errstring, 0, "");
	    return( NULL );
	}

 /*** Load the values into the structures.  ***/

	oldchans[leadnum] = oldchan-1;
	newchans[leadnum] = newchan-1;
	muxchans[leadnum] = muxchan-1;
	rownums[leadnum] = rownum - 1;
	colnums[leadnum] = colnum - 1;
	electrodenums[leadnum] = electrodenum - 1;
	localchannels[leadnum] = localchannel-1;
	if ( oldchans[leadnum] >= 0 ) nvalidlinks++;
    }

 /*** Now look for doubles or any other funky stuff. ***/

    for( leadnum=0; leadnum < nglinks-1; leadnum++)
    { 
	oldchan = oldchans[leadnum];
	newchan = newchans[leadnum];
	muxchan = muxchans[leadnum];
	if ( oldchan >= 0 ) 
	{
	    for( i=leadnum+1; i < nglinks; i++)
	    {
		if ( oldchans[i] == oldchan )
		{
		    fprintf(stderr,"+++ Warning in ReadGeomLinksFile\n"
			    "    channel #%d is pointed at by lead #%d"
			    " and lead #%d\n", 
			    oldchan + 1, 
			    leadnum  + 1, 
			    i + 1);
		}
		if ( newchans[i] == newchan )
		{
		    fprintf(stderr,"+++ Warning in ReadGeomLinksFile\n"
			    "    channel #%d is pointed at by lead #%d"
			    " and lead #%d\n", 
			    newchan + 1, 
			    leadnum  + 1, 
			    i + 1);
		}
		if ( muxchans[i] == muxchan )
		{
		    fprintf(stderr,"+++ Warning in ReadGeomLinksFile\n"
			    "    channel #%d is pointed at by lead #%d"
			    " and lead #%d\n", 
			    muxchan + 1, 
			    leadnum  + 1, 
			    i + 1);
		}
	    }
	}
    }

 /*** Now load the subset of links that were actually found in this
      surface and load just those into the data structure. ***/

    if (( leadlinks = (Lead_Link *) 
	 calloc( (size_t) nvalidlinks, sizeof(Lead_Link) ))
	== NULL )
    {
	fprintf(stderr,"*** ReadGeomLinksFile: error getting memory"
		" for leadlinks\n");
	return( NULL );
    }
    noutleads=0;
    for( leadnum=0; leadnum < nglinks; leadnum++)
    { 
	oldchan = oldchans[leadnum];
	if ( oldchan >= 0 ) 
	{
	    leadlinks[noutleads].leadnum = muxchans[leadnum];
	    leadlinks[noutleads].nodelink = oldchan;
	    leadlinks[noutleads].channel = newchans[leadnum];
	    leadlinks[noutleads].rownum = rownums[leadnum];
	    leadlinks[noutleads].colnum = colnums[leadnum];
	    leadlinks[noutleads].electrodenum = electrodenums[leadnum];
	    leadlinks[noutleads].localchannel = localchannels[leadnum];
	    noutleads++;
	}
    }

    free( newchans );
    free( oldchans );
    free( muxchans );

    if ( noutleads != nvalidlinks )
    {
	printf("ReadChannelLinks error because noutleads = %d and"
	       " nvalidlinks = %d\n", noutleads, nvalidlinks);
	return( NULL );
    } else {
	printf(" For this surface there are %d valid channel links\n",
	       nvalidlinks);
    }
    *numglinks = nvalidlinks;
    return( leadlinks );
}

/*======================================================================*/

void ConvertRCEString( char *rcestring, long *colnum, 
		       long *rownum, long *electrodenum )
{
 /*** Convert a column-row-electrode string to constituent values. ***/

    char *tok_p;
/**********************************************************************/

    tok_p = strtok( rcestring,"-" );
    if ( tok_p != NULL )
    {
	*colnum = strtol( tok_p, NULL, 10 );
	tok_p = strtok( NULL,"-" );
	if ( tok_p != NULL )
	{
	    *rownum = strtol( tok_p, NULL, 10 );
	    tok_p = strtok( NULL,"-" );
	    if ( tok_p != NULL )
	    {
		*electrodenum = strtol( tok_p, NULL, 10 );
	    } else
		*electrodenum = -1;
	} else 
	    *rownum = *electrodenum = -1;
    } else 
	*colnum = *rownum = *electrodenum = -1;
    return;
}
