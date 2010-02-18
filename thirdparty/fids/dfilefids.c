 /*** Filename: dfilefids.c
      Author: Rob MacLeod
      Some routines for fids in  data files.
   Last update: Sun Dec  1 20:38:10 1996
     - created
 ***/

#ifdef OSX
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "graphicsio.h"
#include "fids.h"
#include "cutil.h"

/************** Prototypes *********************************/

/*======================================================================*/
long LoadGlobalFids( FileInfoPtr lu_in, long reportlevel, 
		     Lead_Fids *mainfids )
{

 /*** Load the main fids from the data file into a single Lead_Fid 
   structure ***/

    long error, qtime, stime, ttime, pontime, pofftime, rpeaktime = 0;
    long tpeaktime = 0, numfids;
    Boolean qgotxfids=FALSE;
/**********************************************************************/

    error = getqsttimes_( lu_in, &qtime, &stime, &ttime);
    if ( error < 0 ) 
    {
	fprintf(stderr,"+++ No on- or offsets for this data\n");
	return( error );
    } else
    {
	if ( reportlevel )
	    fprintf(stderr," Found on and offsets\n");
    }
    error = checkextendedfiducials_(lu_in, &qgotxfids);
    if ( error < 0 ) 
    {
	fprintf(stderr,"+++ Error checking for extended fiducials\n");
	pontime = 1;
	pofftime = 1;
    } else
    {
	if ( qgotxfids )
	{
	    if ( reportlevel )
		printf(" Extended fiducials found in the file so read them\n");
	    error = getextendedfiducials_(lu_in, &pontime, &pofftime,
					   &rpeaktime, &tpeaktime);
	    if ( error < 0 )
	    {
		fprintf(stderr,"+++ Error reading extended fiducials\n");
		pontime = 1;
		pofftime = 1;
	    }
	}
	else
	{
	    pontime = 1;
	    pofftime = 1;
	    rpeaktime = 1;
	    tpeaktime = 1;
	}
    }
    numfids = 7;
    if (( mainfids->fidtypes = 
	  (short *) calloc( (size_t) numfids, sizeof( short ))) == NULL)
    {
	ReportError( "ReadDataFile", "error getting basic fid mem", 
		     ERR_MEM, "");
	return( ERR_MEM );
    }
    if (( mainfids->fidvals = 
	  (float *) calloc( (size_t) numfids, sizeof( float ))) == NULL)
    {
	ReportError( "ReadDataFile", "error getting basic fid mem", 
		     ERR_MEM, "");
	return( ERR_MEM );
    }
    mainfids->numfids = numfids;
    mainfids->leadnum = -1;
    mainfids->fidtypes[0] = FI_PON;
    mainfids->fidvals[0] = (float) pontime - 1.f;
    mainfids->fidtypes[1] = FI_POFF;
    mainfids->fidvals[1] = (float) pofftime - 1.f;
    mainfids->fidtypes[2] = FI_QON;
    mainfids->fidvals[2]   = (float) qtime - 1.f;
    mainfids->fidtypes[3] = FI_RPEAK;
    mainfids->fidvals[3]    = (float) rpeaktime - 1.f;
    mainfids->fidtypes[4] = FI_SOFF;
    mainfids->fidvals[4]   = (float) stime - 1.f;
    mainfids->fidtypes[5] = FI_TPEAK;
    mainfids->fidvals[5]    = (float) tpeaktime - 1.f;
    mainfids->fidtypes[6] = FI_TOFF;
    mainfids->fidvals[6]   = (float) ttime - 1.f;

    if ( reportlevel > 1 )
    {
	printf(" Some global fidicials from the file\n"
	       " qframe   = %f\n"
	       " ponframe = %f\n"
	       " rpeak    = %f\n",
	       mainfids->fidvals[2],
	       mainfids->fidvals[0],
	       mainfids->fidvals[3]);
    }

    return( error );
}
/*======================================================================*/
Series_Fids *ReadDfileFids( FileInfoPtr lu_in, long timeseriesnum,
			    long reportlevel, Series_Fids *seriesfids,
			    long *numfidseries )
{

 /*** Load all the fids from one time series of a .data file into an 
      array of Series_Fids structures. 
      We assume the seriesfids structure may have data in it already 
      and test for NULL before we calloc a new one.
      We save all the fids sets that are in the data file associated
      with the designated timeseries number.

     Input:
      lu_in	    file handle for the already open input file.
      timeseriesnum desired time series for which we seek fids
		    First value  for this is 0 not 1!!!
      reportlevel   reporting level desired
     In/Output:
      seriesfids    current pointer to the Series_Fids structure array
      numfidseries  current number of fidseries saved in the seriesfids 
		    array. This number is assumed to contain the current
		    number of fid sets upon entry and is updated here.
      
*********************************************************************/
    long i;
    long error, fsnum;
    long numfidleads;
    long fidcount;
    long numfids;
    long numfidvals;
    long leadnum;
    long numfidsets;
    short *fiddesc, *fidtypes;
    float *fidvals;
    Series_Fids *bigfids;
    Lead_Fids *leadfids;
    FidInfo *fidinfos;
/**********************************************************************/

 /*** Select the time series from the already open file. ***/

    error = settimeseriesindex_( lu_in, timeseriesnum+1 );
    if ( error < 0 ) return( NULL );

 /*** See how many fiducial sets we have to read. 
      If there are none, return immediately. ***/
 /*** Add an additional check to make sure we get a sensible value
      for numfidsets. ***/

    /*    error = getleadfiducialcount_( lu_in, &numfidsets ); */
    numfidsets = 0;
    if ( error < 0 ) return( NULL );
    if ( numfidsets < 1 || numfidsets > 10)
    {
	return( seriesfids );
    }

 /*** Get some fid memory. ***/

    if ( seriesfids == NULL )
    {
	if (( bigfids = (Series_Fids *) 
	      calloc( (size_t) numfidsets, 
		      sizeof( Series_Fids ) )) == NULL ) 
	{
	    char errtext[80];
	    sprintf(errtext," Tried to open %d fidsets", numfidsets);
	    ReportError( "ReadDfileFids", "error getting big memory file",
			 ERR_MEM, errtext );
	    return( NULL );
	}
	*numfidseries = 0;
    } else 
    { 
	bigfids = seriesfids;
	if (( bigfids = (Series_Fids *) 
	      realloc( bigfids, (size_t) 
		       (*numfidseries + numfidsets) * sizeof( Series_Fids ) ))
	    == NULL ) 
	{
	    ReportError( "ReadDfileFids", "error expanding big memory file",
			 ERR_MEM, "" );
	    return( NULL );
	}
    }
	
 /*** Set up memory for the fiducial info array.  ***/
	
    if (( fidinfos = (FidInfo *) 
	  calloc( (size_t) numfidsets, sizeof( FidInfo ) )) == NULL )
    {
	ReportError( "ReadDfileFids", "error getting fidinfos memory",
		     ERR_MEM, "" );
	return( NULL );
    }

 /*** Get the fiducial information for all the fid sets.  ***/

    /*    error = getleadfiducialinfo_( lu_in, fidinfos ); */
    if ( error < 0 ) return( NULL );

 /*** Now loop through all the fidsets and load up the fids array. ***/

    for ( fsnum=0; fsnum < numfidsets; fsnum++ )
    {
	numfidleads = fidinfos[fsnum].fidDescSize;
	numfidvals  = fidinfos[fsnum].numberOfFiducials;

 /*** Get memory for the three buffers we read from the file.  ***/

	if (( fiddesc = (short *) 
	      calloc( (size_t) numfidleads, sizeof( short ) )) == NULL )
	{
	    ReportError( "ReadDfileFids", 
			 "error getting fiddesc memory", ERR_MEM,"" );
	    return( NULL );
	}
	if (( fidvals = (float *) 
	      calloc( (size_t) numfidvals, sizeof( float ) )) == NULL )
	{
	    ReportError( "ReadDfileFids", 
			 "error getting fidval memory", ERR_MEM, "" );
	    return( NULL );
	}
	if (( fidtypes = (short *) 
	      calloc( (size_t) numfidvals, sizeof( short ) )) == NULL )
	{
	    ReportError( "ReadDfileFids", 
			 "error getting fidtypes memory", ERR_MEM, "" );
	    return( NULL );
	}

	if (( leadfids = (Lead_Fids *) 
	      calloc( (size_t) numfidvals, sizeof( Lead_Fids ) )) == NULL )
	{
	    ReportError( "ReadDfileFids", 
			 "error getting leadfids memory", ERR_MEM, "" );
	    return( NULL );
	}

 /*** Now get the fiducial values for this fid set. ***/

 /*	error = getleadfiducials_( lu_in, (short) fsnum+1, fiddesc, fidvals, 
				   fidtypes ); */
	if ( error < 0 ) return( NULL );

 /*** Now load up the data structures. ***/

	bigfids[*numfidseries+fsnum].tsnum = timeseriesnum;
	bigfids[*numfidseries+fsnum].pakfilenum = -1;
	bigfids[*numfidseries+fsnum].numfidleads = numfidleads;
	strcpy( bigfids[*numfidseries+fsnum].fidlabel, 
		fidinfos[fsnum].theLabel );

 /*** Now loop through the leads and load a local leadfids structure.  ***/

	fidcount = 0;
	for ( leadnum=0; leadnum < numfidleads; leadnum++ ) 
	{
	    numfids = fiddesc[leadnum];
	    leadfids[leadnum].leadnum = leadnum;
	    leadfids[leadnum].numfids = numfids;

 /*** Get memory for the fid values and types. ***/

	    if (( leadfids[leadnum].fidtypes =
		  (short *) calloc( (size_t) numfids, sizeof( short )) )
		== NULL )
	    {
		ReportError( "ReadDfileFids", "error getting fidtypes memory",
			     ERR_MEM, "" );
		return( NULL );
	    }
	    
	    if (( leadfids[leadnum].fidvals =
		(float *) calloc( (size_t) numfids, sizeof( float )) )
		== NULL )
	    {
		ReportError( "ReadDfileFids", "error getting fidvals memory",
			     ERR_MEM, "" );
		return( NULL );
	    }

 /*** Now loop through all the different fids for this lead. ***/

	    for ( i=0; i<numfids; i++ )
	    {
		leadfids[leadnum].fidvals[i] = fidvals[fidcount];
		leadfids[leadnum].fidtypes[i] = fidtypes[fidcount];
		fidcount++;
	    }
	}

 /*** Assign the local leadfids to the global seriesfids array. ***/

	bigfids[*numfidseries+fsnum].leadfids = leadfids;

 /*** See what fids types we actually used here and update bigfids. ***/

	error = ScanFidTypes( &bigfids[*numfidseries+fsnum] );
	if ( error < 0 ) return( NULL );

	free( fiddesc );
	free( fidvals );
	free( fidtypes );
    }
    *numfidseries += numfidsets;
    if ( reportlevel > 0 )
	printf(" Returning from ReadDfileFids with numfidseries = %d\n",
	       *numfidseries );
    free( fidinfos );
    return( bigfids );
}
	    
/*======================================================================*/
long WriteOneFidsAsTimeSeries( FileInfoPtr lu_out, Series_Fids *onefidseries,
			    long *timeseriesnum )
{

 /*** Load a single fid series into a single data time series.  

   The desired time series number in the output datafile is 'timeseriesnum',
   and this begins with 0 not 1!!!!
   
   The order of loading is the same as that in the onefidseries itself.
   See the fids.h file for the links between fid types and the fidtype
   numbers.
***/

    long error=0, fidnum, j, k=0;
    long numleads, numfidtypes, numframes;
    long ksave ;
    float *databuff;
    char labelstring[80];
/***********************************************************************/    
    numfidtypes = onefidseries->numfidtypes;
    numframes = numfidtypes + 2;
    numleads = onefidseries->numfidleads;
    
 /*** Get the data buffer large enough to hold all the fids
      we might want to store. ***/

    if ( ( databuff = (float *) calloc( (size_t) (numframes * numleads), 
				      sizeof(float) ) ) == NULL ) {
	printf(" In WriteTimesSeries, error getting databuff memory\n");
	return( error );
    }
    for ( fidnum=0; fidnum < numfidtypes; fidnum++ )
    {
	for (j=0; j<numleads; j++) 
	{
	    databuff[k++] = onefidseries->leadfids[j].fidvals[fidnum]+1.0f;
	}
    }

 /*** Output ARI values too. ***/

    ksave = k;
    for (j=0; j<numleads; j++) 
    {
	databuff[k++] = GetARIVal( &onefidseries->leadfids[j] );
	if ( databuff[k-1] < 0 )
	{
	    numframes--;
	    k = ksave;
	    break;
	}
    }

 /*** Output QTI values too. ***/

    ksave = k;
    for (j=0; j<numleads; j++) {
	databuff[k++] = GetQTIVal( &onefidseries->leadfids[j] );
	if ( databuff[k-1] < 0 )
	{
	    numframes--;
	    k = ksave;
	    break;
	}
    }
    sprintf( labelstring, "Fids for input time series %d"
	     "and pak file %d",
	     onefidseries->tsnum, onefidseries->pakfilenum);
    error = WriteOneFidTimeseries( lu_out, *timeseriesnum, numleads,
			   numframes, labelstring, databuff );
    *timeseriesnum += 1;
    free( databuff );
    return( error );
}

/*======================================================================*/
long WriteFidsAsTimeSeries( FileInfoPtr lu_out, Series_Fids *fidseries,
			    long numfidseries, long *timeseriesnum )
{

 /*** Load a set of fid series into data time series, one time
     series for each fid type.  

   The desired time series number in the output datafile is 'timeseriesnum',
    which is updated here.

   We assume we have the same number of fid types for each times element
   of the fidseries.

   The order of loading is the same as that in the onefidseries itself.
   See the fids.h file for the links between fid types and the fidtype
   numbers.
***/

    long error=0, fidnum=0, j, k=0;
    long numleads, numfidtypes;
    long fidtypenum, fidseriesnum=0, fnum;
    short fidtype;
    float *databuff;
    char labelstring[80], estring[100];
/***********************************************************************/    

    numfidtypes = fidseries[0].numfidtypes;
    numleads = fidseries[0].numfidleads;
    
 /*** Get the data buffer large enough to hold all the fids
      we might want to store. ***/

    if ( ( databuff = (float *) calloc( (size_t) (numleads * numfidseries),
				      sizeof(float) ) ) == NULL ) {
	printf(" In WriteTimesSeries, error getting databuff memory\n");
	return( error );
    }
    k = 0;
    for ( fidtypenum=0; fidtypenum < numfidtypes; fidtypenum++ )
    {
	fidtype = fidseries[fidseriesnum].fidtypes[fidtypenum];
	for ( fidseriesnum=0; fidseriesnum < numfidseries; fidseriesnum++ )
	{
	    if ( fidseries[fidseriesnum].numfidtypes != numfidtypes )
	    {
		sprintf(estring,"We expect %d fid types but fid series #%d"
			" has %d so we give up", numfidtypes, fidseriesnum+1,
			fidseries[fidseriesnum].numfidtypes );
		ReportError("WriteFidsAsTimeSeries", estring, ERR_MISC, "");
		return( ERR_MISC );
	    }
	    for (j=0; j<numleads; j++) 
	    {
		fnum = 
		    WhichFidnum( fidseries[fidseriesnum].leadfids[j].fidtypes,
				 fidseries[fidseriesnum].leadfids[j].numfids,
				 fidtype );
		if ( fnum < 0 )
		{
		    sprintf(estring,"In series %d we have no fids of type %d"
			" so we give up", fidseriesnum, fidtype );
		    ReportError("WriteFidsAsTimeSeries", estring, 
				ERR_MISC, "");
		    return( ERR_MISC );
		}
		databuff[k++] = 
		    fidseries[fidseriesnum].leadfids[j].fidvals[fidnum]+1.0f;
	    }
	}
	sprintf( labelstring, "Fids of type %d: %s",
		 fidseries[0].fidtypes[fidtypenum], 
		 fidseries[0].fidnames[fidtypenum]);

	error = WriteOneFidTimeseries( lu_out, *timeseriesnum, 
				       numleads, numfidseries, 
				       labelstring, databuff );
	*timeseriesnum += 1;
	if ( error < 0 ) return (error);
	k = 0;
    }

 /*** Output ARI values too. ***/

    k = 0;
    for (j=0; j<numleads; j++) 
    {
	for ( fidseriesnum=0; fidseriesnum < numfidseries; fidseriesnum++ )
	{
	    databuff[k++] = GetARIVal( &fidseries[fidseriesnum].leadfids[j] );
	    if ( databuff[k-1] < 0 )
	    {
		k =-1;
		break;
	    }
	    if ( k < 0 ) break;
	}
    }
	
    if ( k > 0 ) 
    {
	sprintf( labelstring, "Fids of type activation recovery interval");
	error = WriteOneFidTimeseries( lu_out, *timeseriesnum, 
				       numleads, numfidseries, 
				       labelstring, databuff );
	if ( error < 0 ) return (error);
	*timeseriesnum += 1;
    }
	
 /*** Output	 QTI values too. ***/

    k = 0;
    {
	for ( fidseriesnum=0; fidseriesnum < numfidseries; fidseriesnum++ )
	{
	    databuff[k++] = GetQTIVal( &fidseries[fidseriesnum].leadfids[j] );
	    if ( databuff[k-1] < 0 )
	    {
		k =-1;
		break;
	    }
	    if ( k < 0 ) break;
	}
    }
	
    if ( k > 0 ) 
    {
	sprintf( labelstring, "Fids of type QT interval");
	error = WriteOneFidTimeseries( lu_out, *timeseriesnum, 
				       numleads, numfidseries, 
				       labelstring, databuff );
	if ( error < 0 ) return (error);
	*timeseriesnum += 1;
    }
    free( databuff );
    return( error );
}

/*======================================================================*/
long WriteOneFidTimeseries( FileInfoPtr lu_out, long timeseriesnum, 
			    long numleads, long numframes, 
			    char *labelstring, float *databuff )
{

 /*** Write a single set of fids to a time series (as pseudo pots)

   timeseriesnum = the number of the time series, beginnig from 0 not 1!!
 ***/
    long error=0;
/**********************************************************************/
    if (error) {
			error = settimeseriesindex_( lu_out, timeseriesnum+1 );
			if ( error < 0 ) return (error);
			error = settimeseriesspecs_( lu_out, numleads, numframes );
			if ( error < 0 ) return( error );
			error = settimeserieslabel_( lu_out, labelstring );
			if ( error < 0 ) return( error );
			error = settimeseriesunits_( lu_out, 0 );
			if ( error < 0 ) return( error );
			error = settimeseriesdata_(  lu_out, databuff );
			if ( error < 0 ) return( error );
    }
    return( error );
}

/*======================================================================*/
long WriteDFileFids( Series_Fids *fidseries,  long timeseriesnum,
		     FileInfoPtr lu_out, long numfidseries )
{

 /*** Save one or more fid sets in an already open data file.

   Input:
     fidseries	    The data structure with all the fids in it for this 
		    .data file
     timeseriesnum  the number of the time series whose associated fiducials
		    we want to save; there can be more than one fid set for
		    any time series number so a single value here does
		    not mean we save a single fid set.
		    The first times series is 0 (not 1).
     lu_out	    output file handle
     numfidseries  number of set of fiducials we have to save

     We assume that the proper time series numbers are already in the 
     fidseries structure.
 ***/

    long error = 0, tsnum, leadnum, i;
    long fidcount, numfidvals;
    long numleads, numleadfidvals;
    short *fiddesc, *fidtypes;
    float *fidvals;
/**********************************************************************/

 /*** Loop through all of the fiducial sets, looking for those (there
      can be more than one) that belong to the selected time series.  ***/

    for ( tsnum=0; tsnum < numfidseries; tsnum++ )
    {
	if ( fidseries[tsnum].tsnum != timeseriesnum )
	    continue;

	numleads = fidseries[tsnum].numfidleads;

 /*** Set the output file index according to what the fids header says. ***/

	error = settimeseriesindex_( lu_out, timeseriesnum+1 );
	if ( error < 0 ) return( error );

 /*** Set up memory for the filedesc array.  ***/
	
	if (( fiddesc = (short *) 
	      calloc( (size_t) numleads, sizeof( short ) )) == NULL )
	{
	    ReportError( "WriteDFileFids", "error getting fiddesc memory",
			 ERR_MEM, "" );
	    return( ERR_MEM );
	}
	
 /*** See how many fiducial values there are in this fid set
      and then set up memory for them. ***/

	numfidvals = FindNumSeriesFidvals( &fidseries[tsnum] );
	
	if (( fidvals = (float *) 
	      calloc( (size_t) numfidvals, sizeof( float ) )) == NULL )
	{
	    ReportError( "WriteDFileFids", "error getting fidvals memory",
			 ERR_MEM, "" );
	    return( ERR_MEM );
	}
	
	if (( fidtypes = (short *) 
	      calloc( (size_t) numfidvals, sizeof( short ) )) == NULL )
	{
	    ReportError( "WriteFidSeries", "error getting fidtypes memory",
			 ERR_MEM, "" );
	    return( ERR_MEM );
	}

 /*** Loop through each lead and load up the fidvals and fidtypes for 
      that lead. ***/

	fidcount = 0;
	for ( leadnum=0; leadnum < numleads; leadnum++ )
	{
	    numleadfidvals = fidseries[tsnum].leadfids[leadnum].numfids;
	    fiddesc[leadnum] = (short) numleadfidvals;
    
	    for (i=0; i < numleadfidvals; i++ )
	    {
		fidvals[fidcount] = 
		    fidseries[tsnum].leadfids[leadnum].fidvals[i];
		fidtypes[fidcount] = 
		    fidseries[tsnum].leadfids[leadnum].fidtypes[i];
		fidcount++;
	    }
	}
    
 /*** Write the result to the file.  ***/

 /* 
      error = setleadfiducials_( lu_out, fidseries[tsnum].fidlabel, 
	   (short) numleads, fiddesc, fidvals, fidtypes );
 */
	if ( error < 0 ) return( error );
	free( fidvals );
	free( fidtypes );
	free( fiddesc );
    }
    return( error );
}

/*======================================================================*/
long FindNumSeriesFidvals( Series_Fids *oneseriesfids )
{
 /*** Find out the total number of fid values in this 
      one set of fiducials. ***/

    long leadnum, numfids;
/**********************************************************************/

    numfids = 0;
    for (leadnum=0; leadnum<oneseriesfids->numfidleads; leadnum++ )
    {
	numfids += oneseriesfids->leadfids[leadnum].numfids;
    }
    return numfids;
}

