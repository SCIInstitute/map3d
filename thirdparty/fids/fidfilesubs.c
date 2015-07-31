 /*** Filename: fidfilesubs.c
      Author: Rob MacLeod
      All the routines needed to read a fid file.

The format of the fids file looks like this:

 %% text header goes here %%
 num-time-series
 time-series-num  pakfilenum
 nleads fid1 fid2 fid3 .....
 1 10 20 30  ...
 2 10 20 31 ...
 ...
 time-series-num 

and so on for multiple time series.  

The difference between time-series-num and pakfilenum is that 
time-series-num is the running number within the file itself; 
it starts at one and increases for each time series in the file.  The 
values for pakfilenum is only really meaningful to link fid information to 
.pak files on the vax and this is how I anticipate it being used.

The numeric values are in frame numbers relative to the full pack file,
starting at 1.  The "fid1", "fid2", etc. are strings that tell us what type of
fiducials are stored in the file and it what order.  The legal strings for this
must include the following:

ref      reference
pon	 P-onset
poff	 P-offset
qon	 Q-onset
rpeak	 Peak of the R wave
soff	 S-offset == end of QRS
tpeak	 Peak of the T wave
toff	 End of the T wave
act	 Activation time 
actplus	 Activation time Max + slope
actminus Activation time Max - slope
rec	 Recovery time
recplus	 Recovery time Max + slope
recminus Recovery time Max - slope

Not part of the fid file itself but derived from it:
ari      activation-recovery inteval
qti      QT interval

   Last update: Tue Feb 11 22:57:59 1997
     - matched to fit with the new graphicsio library structures
   Last update: Sun Apr 14 20:08:41 1996
     - created from original smap version -- now more general so we can share.
***/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "fids.h"
#include "cutil.h"
/*====================================================================*/
long ReadFidfileSeries( long reportlevel, char *fidfilename,
			long seriesnum, long pakfilenum,
			Series_Fids *oneseriesfids )
{

 /*** 
   Read one time series worth of the contents of a fids file and return 
   the fiducials for the selected 
   time series.  We open and close the file each time we come here, and
   return only a single time series' worth of fiducials.

   Input: 
    reportlevel
    fidfilename	    where to look
    seriesnum	    the specified seriesnumber (starting at 0) 
    pakfilennum	    the desired pakfile number
		    The above to arguments work togehter as follows:
		      - if one is zero, the other is used to find data
		      - if both are used, we assume they agree and point
		        to the same data; if not, we complain.
    oneseriesfids	    pointer to one fidlist structure
   Returns:
    error	    standard graphicsio error values

 ***/

    long error;
    long numseries;
    char instring[100];
    FILE *luin;
    
/**********************************************************************/

 /*** Open the fidfile. ***/

    if (( luin = fopen( fidfilename, "r" )) == NULL )
    {
	printf("*** ReadFidfileSeries error: cannot open fid file %s\n",
	       fidfilename);
	perror( "In ReadFidfileSeries: ");
	return (ERR_FILE);
    }

 /*** Get some basic data. ***/

    ReadComments( luin, instring );
    sscanf( instring, "%d", &numseries);
    if ( reportlevel > 2 )
	printf(" In ReadFidfileSeries the first string is %s"
	       " from which we have numseries = %d\n", instring, numseries);

 /*** Get the file opened to the correct locations.  ***/

    if (( error = FindFidfileSeries( luin, &seriesnum, &pakfilenum ) )
	< 0 )
	return( error );
    oneseriesfids->tsnum = seriesnum;
    oneseriesfids->pakfilenum = pakfilenum;

 /*** Now read the time series we actually want. ***/

    if ( reportlevel > 2 )
	printf(" In ReadFidfileSeries: "
	       "Reading Fid series #%d\n"
	       " from the file %s\n", seriesnum+1, fidfilename);
    
    error = ReadOneFidfileSeries( reportlevel, luin, oneseriesfids );
    
    fclose( luin );
    return( error );
}

/*======================================================================*/
long FindFidfileSeries( FILE *luin, long *timeseriesnum, long *pakfilenum )
{
 /*** 	
   Find a specified time series of fids, according to either time series
   number or the pakfile number.

   Leave the file pointing to the first line of the time series, that 
   is, at the label line for the fids.

   Input: 
    luin	    pointer to the file
   Input/Output:
    timeseriesnum   the desired/found seriesnumber (first one is 0)
    pakfilenum	    the desired/found pakfile number
		    The above to arguments work togehter as follows:
		      - if one is zero, the other is used to find data
		      - if both are used, we assume they agree and point
		        to the same data; if not, we complain.

   Return a value < 0 if we have an error condition.
 ***/

    long i, val1, val2, error=0;
    long seriesnum, numseries;
    char instring[100];
/**********************************************************************/
    rewind( luin );

 /*** Get some basic data. ***/

    ReadComments( luin, instring );
    sscanf( instring, "%d", &numseries);

 /*** Read the header for the first series, both for the skip function
      we use below and to see if we have a hit already.  ***/

    ReadComments( luin, instring );
    if ( ( sscanf( instring,"%d %d", &val1, &val2) ) == 2 )
	i = 2; /*** Dummy statement ***/
    else if ( ( sscanf( instring,"%d", &val1) ) == 1 )
	val2 = 0;
    else 
    {
	printf(" FindFidFileSeries: error in format with line: %s\n",
	       instring);
	return -99;
    }

 /*    printf(" first line is %s; val1 = %d, val2 = %d\n", 
       instring, val2, val2); 
 */

 /*** Select series by timeseries number  ***/

    if ( *pakfilenum <= 0 && *timeseriesnum >= 0 ) 
    {
	if( *timeseriesnum >= numseries ) 
	{
	    printf(" FindFidFileSeries: error because you requested"
		   " series %d\n"
		   " and the file only has %d series\n",
		   *timeseriesnum+1, numseries);
	    return( -1 );
	}

 /*** See if we have a hit already ***/

	if ( val1-1 == *timeseriesnum ) 
	{
	    *pakfilenum = val2;
	    return( 0 );
	}

 /*** If not, skip over the right number of series, checking along
      the way to see if we are at the right place.   ***/

	for (seriesnum=0; seriesnum <= *timeseriesnum; seriesnum++)
	{
	    if (( error = SkipFidfileSeries( luin, &val1, &val2 ) ) < 0 )
		return( error );
	    if ( val1-1 == *timeseriesnum ) 
	    {
		*pakfilenum = val2;
		break;
	    }
	}
	if ( val1-1 != *timeseriesnum )
	{
	    printf(" FindFidFileSeries: error because val1 = %d and"
		   " timersiesnum = #%d\n", val1, timeseriesnum+1);
	    return( -1 );
	}

 /*** Or by pak file number. ***/

    } else if ( *timeseriesnum < 0 && *pakfilenum > 0 )
    {

 /*** See if we have a hit already ***/

	if ( val2 == *pakfilenum ) 
	{
	    *timeseriesnum = val1-1;
	    return( 0 );
	}

 /*** If not, go looking.  ***/

	for (i=0; i<numseries-1; i++)
	{
	    if (( error = SkipFidfileSeries( luin, &val1, &val2 ) ) < 0 )
		return( error );
	    if ( val2 == *pakfilenum )
	    {
		*timeseriesnum = val1-1;
		break;
	    }
	}

	if ( val2 != *pakfilenum )
	{
	    printf(" FindFidFileSeries: error because val2 = %d and"
		   " pakfilenum = %d\n", val2, pakfilenum);
	    return( -1 );
	}
 /*** Error condition.  ***/

    } else {
	printf(" FindFidFileSeries: error, with seriesnum = %d"
	       " and pakfilenum = %d"
	       "\n we do not know what to do\n", 
	       (*timeseriesnum)+1, *pakfilenum);
	return( -1 );
    }
    return( 0 );
}

/*======================================================================*/
long SkipFidfileSeries( FILE *luin, long *seriesval, long *paknum )
{
 /*** Skip over a complete time series worth of fiducials.
   We assume that as we get here, the file reader is pointed at 
   the firstline of the time series data, the line that contains the 
   fid labels.

   On return, we are pointed at the same line of the next series.

   Return a value < 0 if we have an error condition.
   Return the pak file number if there is one in the file.
 ***/

    char instring[100];
    long val1, val2, error=0, numleads, leadnum;
/**********************************************************************/

 /*** Read the line that has number of leads and labels. ***/
    if ( (ReadComments( luin, instring ) ) == NULL ) 
    {
	printf("SkipFidSeries:  Error getting line with numleads\n");
	return (-99);
    }
    sscanf( instring,"%d", &numleads);
    for (leadnum=0; leadnum < numleads; leadnum++)
    {
	if ( ( ReadComments( luin, instring ) ) == NULL ) 
	{
	    printf("SkipFidSeries:  Error getting line for lead %d"
		   " of %d total\n",
		   leadnum+1, numleads);
	    return (-99);
	}
    }

 /*** Now read the first line of the next time series, the one with the
      series and pak file number
 ***/
    if ( ( ReadComments( luin, instring ) ) == NULL ) 
    {
	printf("SkipFidSeries:  Error getting line with series"
	       " and pak numbers\n");
	return (-99);
    }
    if ( sscanf( instring,"%d %d", &val1, &val2) )
	error= 0; /*** Dummy statement ***/
    else if ( ( sscanf( instring,"%d", &val1) ) == 1 )
	val2 = 0;
    else 
    {
	printf(" SkipFidSeries: error in format with line: %s\n",
	       instring);
	return -99;
    }
    *seriesval = val1;
    *paknum = val2;
    return error;
}

/*======================================================================*/
#define SEPCHARS " ,"
long ReadOneFidfileSeries( long reportlevel, FILE *luin, 
		       Series_Fids *onefidseries )
{
 /*** Read in a series of fiducial values from a fid file.
      We assume we are pointing at the line at which the time series starts
      that is, with the number of lead and list of fid labels in it.
      
      We load the onefidseries structure with fid values, based on the 
      set of fids the file tells us it contains.  This is coded into the 
      file, in the same line as the number of leads.

      NOTE1: The values returned for fid values are adjusted for use as
            pointers in C; that is they have had one subtracted from
	    the values in the file. 
      NOTE2: the fids part of onefidseries gets allocated here and passed
            to the calling routine.  SO do not set this up beforehand.

      Input:
       reportlevel
       luin		File lu we are reading from
       onefidseries		fids we load up and return
***/

    long numleads, leadnum, testleadnum;
    long numfidtypes, oldnumfidtypes, maxnumfidtypes, fnum;
    short *fidtypelist=NULL;
    float fidval;
    char instring[200];
    char errstring[100];
    char *word_p;
    Lead_Fids *leadfids = NULL;
    FiducialInfo fidinfo;
/**********************************************************************/

 /*** Allocate space for the fidtypelist. ***/
    maxnumfidtypes =  (long) FI_GetNumTypes();
    if ( maxnumfidtypes < 1 || maxnumfidtypes > 100 )
    {
	sprintf(errstring," FI_GetNumTypes returns %d ", 
		maxnumfidtypes );
	ReportError("ReadOneFidFileSeries", "too many fid types",
		    ERR_MEM, errstring);
	return( ERR_MEM );
    }
    if ( ( fidtypelist = (short *) calloc( (size_t) maxnumfidtypes, 
					   sizeof( short )) ) == NULL )
    {
	sprintf(errstring,"Trying to get memory for %d entries", 
		maxnumfidtypes );
	ReportError("ReadOneFidFileSeries", "error getting fidtypelist memory",
		    ERR_MEM, errstring);
	return( ERR_MEM );
    }

 /*** Get the line with the number of leads and the fid names. 
      Then parse it for sensible values. ***/

    if ( ( ReadComments( luin, instring ) ) == NULL ) 
	return (ERR_FILE);

    word_p = strtok( instring, SEPCHARS );
    numleads = atoi( word_p );
    onefidseries->numfidleads = numleads;
    if ( ( leadfids = (Lead_Fids *) 
	   calloc ((size_t) numleads, sizeof(Lead_Fids))) == NULL ) {
	printf(" Error in ReadFidSeries getting onefidseries->leadfids\n");
	exit(1);
    }

    numfidtypes = 0;
    while ( word_p != NULL )
    {
	word_p = strtok( NULL, SEPCHARS );
	if ( word_p == NULL || strcmp( word_p, "\n") == 0 ) break;
	oldnumfidtypes = numfidtypes;

	if ( reportlevel > 0 )
	    printf("\n Fid string found as %s\n", word_p);

 /*** See which string from the fidinfo structure matches this
      string from the file. 
 ***/
	FI_First( &fidinfo );
	do
	{
	    if ( reportlevel > 2 )
		printf(" Comparing %s to %s\n",
		       word_p, fidinfo.name);

	    if ( strstr( word_p, fidinfo.name ) != NULL )
	    {
		fidtypelist[numfidtypes] = fidinfo.type;
		numfidtypes++;
		break;
	    }
	} while ( FI_Next( &fidinfo ) );

	if ( oldnumfidtypes == numfidtypes )
	{
	    printf("*** In ReadOneFidSeries: \n"
		   "*** could not interpret a fid type %s\n"
		   "*** so will set it to -1\n",
		   word_p);
	    fidtypelist[numfidtypes] = -1;
	    numfidtypes++;
	}
    }

 /*** Set up the fid types and names for the Series_Fids.  ***/

    onefidseries->numfidtypes = numfidtypes;
    if ( ( onefidseries->fidtypes = (short *) 
	   calloc( (size_t) numfidtypes, sizeof( short ) ) ) == NULL )
    {
	ReportError( "ReadOneFidSeries", "error getting new type memory",
		     ERR_MEM, "");
	return( ERR_MEM );
    }
    if ( ( onefidseries->fidnames = (char **) 
	   calloc( (size_t) numfidtypes, sizeof( char * ) ) ) == NULL )
    {
	ReportError( "ReadOneFidSeries", "error getting new names memory",
		     ERR_MEM, "");
	return( ERR_MEM );
    }

 /*** Load up the name and type information into the structures. ***/

    for( fnum = 0; fnum < numfidtypes; fnum++ )
    {
	fidinfo.type = fidtypelist[fnum];
	FI_GetInfoByType( &fidinfo );
	onefidseries->fidtypes[fnum] = fidtypelist[fnum];
	if ( ( onefidseries->fidnames[fnum] = (char *) 
	   calloc( (size_t) strlen( fidinfo.name ), sizeof( char ))) == NULL )
	{
	    ReportError( "ReadOneFidSeries", "error getting new names memory",
			 ERR_MEM, "");
	    return( ERR_MEM );
	}
	
	strcpy( onefidseries->fidnames[fnum], 
		fidinfo.name );
    } 
    
 /*** Set up the memory for the different fid types. ***/

    for ( leadnum = 0; leadnum < numleads; leadnum++ )
    {
	if ( ( leadfids[leadnum].fidtypes = (short *) 
	       calloc( (size_t) numfidtypes, sizeof( short ) ) ) == NULL )
	{
	    ReportError( "ReadOneFidSeries", "error getting new type memory",
			 ERR_MEM, "");
	    return( ERR_MEM );
	}
	if ( ( leadfids[leadnum].fidvals = (float *) 
	       calloc( (size_t) numfidtypes, sizeof( float ) ) ) == NULL )
	{
	    ReportError( "ReadOneFidSeries", "error getting new vals memory",
			 ERR_MEM, "");
	    return( ERR_MEM );
	}
    }

 /*** Now read through the list of fids and extract each value. ***/

    for (leadnum=0; leadnum < numleads; leadnum++)
    {
	if ( ( ReadComments( luin, instring ) ) == NULL ) 
	    return (ERR_FILE);
	word_p = strtok( instring, SEPCHARS );
	testleadnum = atoi( word_p );
	if ( leadnum+1 != testleadnum )
	{
	    printf("*** Error in ReadOneFidSeries:\n"
		   " File says we are on lead #%d but we should be on #%d\n",
		   testleadnum, leadnum+1);
	    return( ERR_FILE );
	}
	leadfids[leadnum].leadnum = leadnum;
	leadfids[leadnum].numfids = numfidtypes;
	for ( fnum=0; fnum < numfidtypes; fnum++ )
	{
	    word_p = strtok( NULL, SEPCHARS );
	    fidval = (float)atof( word_p ) - 1;
	    leadfids[leadnum].fidtypes[fnum] = fidtypelist[fnum];
	    leadfids[leadnum].fidvals[fnum] = fidval;
	}

    }
    onefidseries->leadfids = leadfids;

    if ( reportlevel > 0 )
    {
	DisplayLeadFids( &leadfids[0] );
    }

    return 0;
}

/*======================================================================*/
long GetNumFidfileSeries( char *fidfilename )
{
    
 /*** Return the number of fid series in the file.  ***/

    long numseries;
    char instring[100];
    FILE *luin;
/**********************************************************************/
 /*** Open the fidfile. ***/

    if (( luin = fopen( fidfilename, "r" )) == NULL )
    {
	printf("*** GetNumFidSeries error: cannot open fid file %s\n",
	       fidfilename);
	perror( "In GetNumFidSeries: ");
	return (ERR_FILE);
    }

 /*** Get some basic data. ***/

    ReadComments( luin, instring );
    sscanf( instring, "%d", &numseries);
    fclose( luin );
    return( numseries );
    
}

/*======================================================================*/
long WriteFidfileSeries( long reportlevel, FILE *luout, 
		     Series_Fids *onefidseries )
{
 /*** 
   Write a single fid series to a fid file that we assume is already open.

 ***/
    
    long i;
    long error;
    long leadnum, fidnum, fnum;
    long numfidtypes=0;
    short fidtypelist[NUMFIDTYPES], fidtype=0;
    char fstring[100];
    FiducialInfo fidinfo;
/**********************************************************************/
    if ( reportlevel > 2 ) {
	printf(" Writing all the stored fids to a file\n");
    }

    error= 0;
    fprintf(luout,"%d %d\n %d ", 
	    onefidseries->tsnum+1, onefidseries->pakfilenum+1, 
	    onefidseries->numfidleads);
    strcpy( fstring, " %4d ");

    for (leadnum = 0; leadnum < onefidseries->numfidleads; leadnum++ )
    {
	for (fnum=0; fnum< onefidseries->leadfids[leadnum].numfids; fnum++)
	{
	    fidinfo.type = onefidseries->leadfids[leadnum].fidtypes[fnum];
	    FI_GetInfoByType( &fidinfo );
	    if ( QNewFidname( fidtypelist, numfidtypes, fidinfo.name ) )
	    {
		fidtypelist[numfidtypes] = fidtype;
		numfidtypes++;
	    }
	}
    }

    if ( numfidtypes != onefidseries->numfidtypes ) 
    {
	printf("*** In WriteFirdfileSeries: number of fid types "
	       " do not agree\n"
	       " In the Series_Fids we have %d\n"
	       " but by scanning all the leadfids we get %d\n",
	       onefidseries->numfidtypes, numfidtypes);
    }
 
 /*** Print out the header with the list of fid types. ***/

    for (i=0; i<numfidtypes; i++) {
	fidinfo.type = fidtypelist[i];
	FI_GetInfoByType( &fidinfo );
	fprintf(luout," %s ", fidinfo.name );
    }
    fprintf(luout, " \n");

 /*** Now the fid values themselves 
   Note that we only save the first occurrence of a particular type -
   the fid file format is just not flexible enough to handle mutiple
   fids.
 ***/

    for (leadnum=0; leadnum < onefidseries->numfidleads; leadnum++)
    {
	fprintf(luout, "%4d ", leadnum+1);
	for (fnum=0; fnum<numfidtypes; fnum++) {
	    if ( (fidnum = WhichFidnum( fidtypelist, numfidtypes, 
			       fidtypelist[fnum])) > 0 )
		fprintf(luout, "%8.2f ", 
			onefidseries->leadfids[leadnum].fidvals[fidnum]);
	    else 
		fprintf(luout, "%8.2f ", 0.0 );
	}
	fprintf(luout, " \n");
    }
    return( error );
}

