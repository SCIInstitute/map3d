 /*** Filename: dfilefids.c
      Author: Rob MacLeod
      Some routines for fids in  data files.
   Last update: Sun Dec  1 20:38:10 1996
     - created
 ***/

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

