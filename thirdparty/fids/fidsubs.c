 /*** Filename: fidsubs.c
      Author: Rob MacLeod
      Some routines to handle the fids in the fiducial
      data structures.

      These are the core routines that are independent of data files or
      anything but the data structures we have defined to handle fids.
      Consider them the utility routines for managing fids.

   Last update: Wed Dec 18 15:31:24 1996
     - updated fidtype argument to be fid name
   Last update: Sun Dec  1 20:38:10 1996
     - created
 ***/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "fids.h"
#include "cutil.h"
/************** Defines ************************************/
/************** Prototype **********************************/
/************** The Code ***********************************/
/*======================================================================*/
Boolean QNewFidname( short *fidtypelist, long numfidtypes, 
		     const char *fidname )
{

 /*** See if the fid type in 'fidname' is a new one or is already in 
      the list in "fidtypelist". 
***/
    long i;
    Boolean qnew=TRUE;
    short fidtype;
/**********************************************************************/
    
    fidtype = FI_NameToType( fidname );
    for ( i=0; i<numfidtypes; i++)
    {
	if ( fidtype == fidtypelist[i] )
	{
	    qnew = FALSE;
	    break;
	}
    }
    return qnew;
}

/*======================================================================*/
Boolean QNewFidtype( short *fidtypelist, long numfidtypes, short fidtype )
{

 /*** See if the fid type described by 'fidtype' is present in the list
      stored in 'fidtypelist' (with 'numfidtypes' member. 
      Return true if this is, indeed, a new fid type, otherwise return 
      false.
***/
    long i;
    Boolean qnew=TRUE;
/**********************************************************************/
    for (i=0; i<numfidtypes; i++)
    {
	if ( fidtypelist[i] == fidtype )
	{
	    qnew = FALSE;
	    return( qnew );
	}
    }
    return( qnew );
}
    
/*======================================================================*/
long ScanFidTypes( Series_Fids *oneseriesfids )
{
 /*** Look through the fids stored in oneseriesfids and figure out
      how many and what types of fiducial types have been stored there.
      Load up the fiducials structure for fid types and the labels
      that go with them.
***/

    long leadnum, fidnum;
    long numfidtypes=0, maxnumfidtypes;
    short fidtype;
    short *fidtypes=NULL, *fidtypelist=NULL;
    char **fidnames=NULL, **fidnamelist=NULL;
    FiducialInfo fidinfo;
/**********************************************************************/
 /*** Set up some memory for the scan -- we use the maximum possible
      size here because we do not know how many different fid types
      are in the structrue, just how many we could maximally have. ***/

    maxnumfidtypes = FI_GetNumTypes();
    if (( fidtypelist = (short *) calloc( (size_t) maxnumfidtypes,
				       sizeof( short ))) == NULL )
    {
	ReportError("ScanFidTypes", "error getting fidtypelist memory",
		    ERR_MEM, "" );
	return( ERR_MEM );
    }

    if (( fidnamelist = (char **) calloc( (size_t) maxnumfidtypes,
				       sizeof( char * ))) == NULL )
    {
	ReportError("ScanFidTypes", "error getting fidnamelist memory",
		    ERR_MEM, "" );
	return( ERR_MEM );
    }
    for (fidnum=0; fidnum < maxnumfidtypes; fidnum++ )
    {
	if (( fidnamelist[fidnum] = (char *) calloc( (size_t) 40,
				       sizeof( char ))) == NULL )
	{
	    ReportError("ScanFidTypes", "error getting fidnamelist memory",
			ERR_MEM, "" );
	    return( ERR_MEM );
	}
    }

 /*** Now loop through all the leads and see what we have in the 
      way of different fid types over all the leads. ***/

    for (leadnum=0; leadnum < oneseriesfids->numfidleads; leadnum++ )
    {
	for (fidnum=0; fidnum < oneseriesfids->leadfids[leadnum].numfids; 
	     fidnum++ )
	{
	    fidtype = oneseriesfids->leadfids[leadnum].fidtypes[fidnum];
	    if ( QNewFidtype( fidtypelist, numfidtypes, fidtype ) )
	    {
		fidtypelist[numfidtypes] = fidtype;
		fidinfo.type = fidtype;
		FI_GetInfoByType( &fidinfo );
		strcpy( fidnamelist[numfidtypes], fidinfo.name );
		numfidtypes++;
	    }
	}
    }

 /*** Now set up the proper fidtype list and update the Series_Fids 
   structure.  ***/

    if (( fidtypes = (short *) calloc( (size_t) numfidtypes,
				       sizeof( short ))) == NULL )
    {
	ReportError("ScanFidTypes", "error getting fidtypes memory",
		    ERR_MEM, "" );
	return( ERR_MEM );
    }
    if (( fidnames = (char **) calloc( (size_t) numfidtypes,
				       sizeof( char * ))) == NULL )
    {
	ReportError("ScanFidTypes", "error getting fidnames memory",
		    ERR_MEM, "" );
	return( ERR_MEM );
    }
    for (fidnum=0; fidnum<numfidtypes; fidnum++)
    {
	fidtypes[fidnum] = fidtypelist[fidnum];
	if (( fidnames[fidnum] = 
	      (char *) calloc( (size_t) strlen(fidnamelist[fidnum])+1,
					       sizeof( char ))) == NULL )
	{
	    ReportError("ScanFidTypes", "error getting fidnames memory",
			ERR_MEM, "" );
	    return( ERR_MEM );
	}
	strcpy( fidnames[fidnum], fidnamelist[fidnum] );
    }
    oneseriesfids->numfidtypes = numfidtypes;
    oneseriesfids->fidtypes = fidtypes;
    oneseriesfids->fidnames = fidnames;

 /*** Clean up and get out of here. ***/

    free( fidtypelist );
    free( fidnamelist );
    return( 0 );
}

/*======================================================================*/
long WhichFidnum( short *fidtypelist, long numfidtypes, 
		  const short fidtype ) 
{

 /*** Map the fid type to an entry number in the fidtypelist array.
      This is used to put fid values in the proper place in the Lead_Fids
      arrays.
      Input:
      *fidtypelist  list of current fid types
      numfidtypes   number of fids we have = <= size of fidtypelist
      fidtype	    the fid type that we are looking for.

      Returns:
       the number in the fidtypelist where 'fidtype' is hiding
       or -1 if there is no entry of the type 'fidtype' in the list.
 ***/

    long i;
/**********************************************************************/

    for ( i=0; i<numfidtypes; i++ )
    {
	if ( fidtype == fidtypelist[i] )
	{
	    return i;
	}
    }

 /*** If there was no fid found that matched the type, return -1. ***/

    return( -1 );
}

/*======================================================================*/
long AddLeadfidtype( Lead_Fids *oneleadfids, const short fidtype )
{

 /*** Add another fid type to the oneleadfid structure of type 'fidtype'.
      The idea here is to extend the size of the relevant arrays in 
      the Lead_Fids structure and load the type value.

      Check and make sure we do not have an entry for this type already.

      Return the number of the element in the fidvals/fidtypes array
      the points to the 'fidtype' desired 
      or error return.
 ***/
    long numfids;
/**********************************************************************/

    numfids = oneleadfids->numfids+1;
    if ( ! oneleadfids->fidtypes || oneleadfids->numfids <= 0 )
    {
	if ( ( oneleadfids->fidtypes == (short *) 
	       calloc( (size_t) numfids, sizeof( short ) ) ) == (int) NULL )
	{
	    ReportError( "AddLeadfidtype", "error getting new type memory",
			 ERR_MEM, "");
	    return( ERR_MEM );
	}
	if ( ( oneleadfids->fidvals == (float *) 
	       calloc( (size_t) numfids, sizeof( float ) ) ) == NULL )
	{
	    ReportError( "AddLeadfidtype", "error getting new vals memory",
			 ERR_MEM, "");
	    return( ERR_MEM );
	}
    } else {

 /*** Make sure we do not have one of this type already.  ***/

	if( ! QNewFidtype ( oneleadfids->fidtypes, numfids, fidtype ) )
	{
	    printf("\n In AddLeadfidtype: no need to add one\n"
		   " we have one of type %d already!!\n", fidtype );
	    return( WhichFidnum( oneleadfids->fidtypes, numfids, fidtype) );
	}

 /*** Now realloc the memory we need and set things up.  ***/

	if (( oneleadfids->fidtypes == (short *)
	      realloc( oneleadfids->fidtypes, 
		       (size_t) numfids * sizeof( short )) ) == NULL )
	{
	    ReportError( "AddLeadfidtype", "error reallocing type memory",
			 ERR_MEM, "");
	    return( ERR_MEM );
	}
	if (( oneleadfids->fidvals == (float *)
	      realloc( oneleadfids->fidvals, 
		       (size_t) numfids * sizeof( float )) ) == NULL )
	{
	    ReportError( "AddLeadfidtype", "error reallocing vals memory",
			 ERR_MEM, "");
	    return( ERR_MEM );
	}
    }

 /*** Update some important values in the structure and return the element
      number in the array that corresponds to the desired element. ***/

    oneleadfids->fidtypes[numfids-1] = fidtype;
    oneleadfids->numfids = numfids;
    return( numfids-1 );
}

/*======================================================================*/
void DisplayLeadFids ( Lead_Fids *oneleadfids )
{

 /*** Display the current fids from this lead_fid set.  ***/

    long fnum;
    FiducialInfo fidinfo;
/**********************************************************************/
    printf(" For lead %d we have %d fiducials\n", oneleadfids->leadnum+1,
	   oneleadfids->numfids);
    for ( fnum = 0; fnum < oneleadfids->numfids; fnum++ )
    {
	fidinfo.type = oneleadfids->fidtypes[fnum];
	if ( FI_GetInfoByType( &fidinfo ) )
	{
	    printf(" Fid #%d is %f of type %d (%s)\n",
		   fnum+1, oneleadfids->fidvals[fnum],
		   oneleadfids->fidtypes[fnum],
		   fidinfo.label );
	} else {
	    printf(" Fid #%d is %f of type %d (type unknown)\n",
		   fnum+1, oneleadfids->fidvals[fnum],
		   oneleadfids->fidtypes[fnum]);
	}

    }
}

/*======================================================================*/
float GetARIVal( Lead_Fids *oneleadfids )
{

 /*** Return the activation recovery time for this lead, based on
      the values for activation and recovery.

      If there are not activation or recovery times in the leadfids, then
      return a value of -1. 
 ***/
    long actfnum, recfnum;
/**********************************************************************/
    if ( ( actfnum = WhichFidnum( oneleadfids->fidtypes, oneleadfids->numfids,
				  FI_ACT ) ) < 0 )
    {
	printf(" No activation time in this fid set so we cannot save"
	       " ARIs\n");
	return( -1.0 );
    }
    if ( ( recfnum = WhichFidnum( oneleadfids->fidtypes, oneleadfids->numfids,
				  FI_REC ) ) < 0 )
    {
	printf(" No recovery time in this fid set so we cannot save"
	       " ARIs\n");
	return ( -1.0 );
    }
    return( oneleadfids->fidvals[recfnum] - oneleadfids->fidvals[actfnum] );
}

/*======================================================================*/
float GetQTIVal( Lead_Fids *oneleadfids )
{

 /*** Return the QT interval time for this lead, based on
      the values for qon and toff.

      If there are not q and t times in the leadfids, then
      return a value of -1. 
 ***/
    long qonfnum, toffnum;
/**********************************************************************/
    if ( ( qonfnum = WhichFidnum( oneleadfids->fidtypes, oneleadfids->numfids,
				  FI_QON ) ) < 0 )
    {
	printf(" No qonset time in this fid set so we cannot save"
	       " QTIs\n");
	return( -1.0 );
    }
    if ( ( toffnum = WhichFidnum( oneleadfids->fidtypes, oneleadfids->numfids,
				  FI_TOFF ) ) < 0 )
    {
	printf(" No toff time in this fid set so we cannot save"
	       " QTIs\n");
	return ( -1.0 );
    }
    return( oneleadfids->fidvals[toffnum] - oneleadfids->fidvals[qonfnum] );
}

