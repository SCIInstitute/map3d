 /*** 
   Filename: writegeomfile.c
   Author: Rob MacLeod

   Version for gfile/lib not map3d!!!
 ***/
/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geomlib.h"
#include "cutil.h"
/***************** Prototypes *****************************/
long SaveElements( FileInfoPtr luout, long numelements,
		  long elementsize, long elestart, long eleend,
		  long pntstart,
		  long **elements);

/***************** The Code *******************************/
/*======================================================================*/
static time_t thetime = 0;
#include <time.h>
long WriteGeomFile (char *outfilename, Surf_Geom *surfgeom,
		    long startsurfnum, long endsurfnum, 
		    long numsurfs, long reportlevel)
{
 /*** A function to write out a .geom file from the current contents
      of a Surf_Geom structure.
      
   NOTE:  ALL SURFACE NUMBERS BEGIN WITH 0!!!!!!	
     Input:
      outfilename   	filename to be used for the output file.
      	    	    	(not constant because it could get renamed
			 in ClobberFile)
      surfgeom 	    	an array of structures full of surface geometry
	startsurfnum	first surface number to write
			IF < 0 write all surfaces
	endsurfnum	last surface number to write
			IF < 0 write all surfaces
      numsurfs	    	number of surfaces to save 
      reportlevel   	report level for output to stderr during execution

***/
    long error;
    long errorlevel;
    long firstsurf, lastsurf, surfnum, surfcount;
    char cthetime[40], charuser[40], *cuser_p;
    char mainheader[256];
    FileInfoPtr luout;
 /*********************************************************************/

 /*** Open the geometry file. ***/ 

    if ( reportlevel )
	fprintf(stderr,"In WriteGeomFile\n");
    
 /*** Create the file. ***/

    if ( ( error = ClobberFile ( outfilename ) ) < 0 )
    {
	fprintf(stderr, "(writegeomfile.c)\n"
		" +++ Error getting filename so no permanent");
	return(error);
    }

    errorlevel = 1;
    if ( ( error = createfile_( (char*) outfilename, FILETYPE_GEOM,
			       errorlevel, &luout)) < 0 )
    {
	fprintf(stderr, "(writegeomfile.c)\n"
		" +++ Error creating geometry file");
	return(error);
    }

 /*** Get the system time and use it to make the experiment id. 
 ***/

    thetime = time( NULL );
    if ( reportlevel )
	fprintf(stderr," Got the system time as %d\n", thetime);
    sprintf( cthetime, "%d", thetime );
    setexpid_( luout, cthetime );

    strcpy( mainheader," Geometry created by user ");
    cuser_p = charuser;
    cuser_p = getenv( "USER" );
    if (!cuser_p)
      cuser_p = "DefaultUser";
    if ( reportlevel )
	fprintf(stderr," User here is found as %s\n", cuser_p);
    strcat( mainheader, cuser_p );
    strcat( mainheader, " Date/Time: ");
    if ( reportlevel )
	printf(" The charcter time string is %s\n", ctime( &thetime) );
    strcat( mainheader, ctime( &thetime ) );
    if ( reportlevel )
	printf("Main header for the file is %s\n", mainheader);
    if (( error = settext_( luout, mainheader ) )< 0 )
    {
	fprintf(stderr, "(writegeomfile.c)\n"
		" +++ Error creating geometry file");
	return(error);
    }

 /*** OK. now got through the surface(s) we wish to save. ***/

    if ( startsurfnum < 0 )
    {
	firstsurf = 0;
	lastsurf = numsurfs-1;
    } else {
	firstsurf = startsurfnum;
	if ( endsurfnum < 0 )
	    lastsurf = numsurfs-1;
	else
	    lastsurf = endsurfnum;
    }
    surfcount = 0;
    for ( surfnum=firstsurf; surfnum<= lastsurf; surfnum++ )
    { 

 /*** Send out a single surface worth of geometry. ***/

	error = WriteSurfGeomFile( &surfgeom[surfnum], luout, surfcount,
				   reportlevel );
	surfcount++;
	if ( error < 0 ) 
	{
	    fprintf(stderr, "(writegeomfile.c)\n"
		    " +++ Error reurn from WriteSurfGeomFile is %d", error);
	    return( error );
	}
    }

 /*** Now close the file and get outa here. ***/

    closefile_( luout );
    fprintf(stderr," Finished writing output to the file %s\n",
	    outfilename);
    return error;
}

/*=====================================================================*/
long WriteSurfGeomFile( Surf_Geom *onesurfgeom, FileInfoPtr luout, 
		       long outsurfnum, long reportlevel )
{

 /*** Save one surfaces worth of the geometry informatio to an 
   already open file 

   NOTE:  ALL SURFACE NUMBERS BEGIN WITH 0!!!!!!	

   Input:
    onesurfgeom	    pointer to a single surface surf_geom structure
    luout   	    FileInfoPtr of the already open .geom file
    outsurfnum	    The surface number in the output context
		    (often different from surfnum in onesurfgeom)
    reportlevel	    > 0 for reporting of progress and diagnostics.

***/
    long i, error;
    long surfnum;
    long numpts;
    long scalartype;
    long printmax, numtoprint;
    float *channels_p;
    float *save_p;
    char surfaceheader[80];
    Boolean qsavechannels;
    NodePtr nodes_p, savenodes_p;
/**********************************************************************/

    surfnum = onesurfgeom->surfnum;

    if ( reportlevel )
	fprintf(stderr,"WriteGeomFile: surface #%d\n", outsurfnum+1);

 /*** Set the index. ***/

    error = setsurfaceindex_( luout, outsurfnum+1 );
    if ( error < 0 ) 
    {
	fprintf(stderr, "(writegeomfile.c)\n"
		" +++ Error setting index to %d is %d", 
		outsurfnum+1, error);
	return( error );
    }

 /*** Write out a header text. ***/

    strcpy( surfaceheader, onesurfgeom->label );
    if ( strlen( surfaceheader ) < 1 )
	sprintf(surfaceheader," Output surface %d from input surface %d", 
		outsurfnum+1, surfnum+1);
    error = setsurfacename_( luout, surfaceheader );
    if ( reportlevel )
	printf(" Saving surface header as\n"
	       "[%s]\n", surfaceheader);

 /*** Set up some parameters and set up the points in a temp buffer
      to ship out to the file.  ***/

    numpts = onesurfgeom->numpts;
    if ( numpts < 1 ) 
    {
	fprintf(stderr,"**** In WriteSurfGeomFile: numpts too small = %d\n",
		numpts);
	
	return( ERR_MISC);
    }
    nodes_p = (NodePtr) calloc( (size_t) numpts, sizeof(Node) );
    if (nodes_p == NULL)
    {
	fprintf(stderr, "**** Can't get temp buffer for nodes\n"
		" Number of nodes requested is %d\n", numpts);
	return(ERR_MEM);
    }
    savenodes_p = nodes_p;

    if ( ! onesurfgeom->nodes )
    {
	fprintf(stderr,"**** In WriteSurfGeomFile: nodes is empty\n" );
	return( ERR_MISC);
    }
	
    for (i=0; i < numpts; nodes_p++, i++)
    {
	nodes_p->x = onesurfgeom->nodes[i][CUTIL_X] ;
	nodes_p->y = onesurfgeom->nodes[i][CUTIL_Y] ;
	nodes_p->z = onesurfgeom->nodes[i][CUTIL_Z] ;
    }
    nodes_p = savenodes_p;
    error = setnodes_( luout, numpts, nodes_p );
    if ( error < 0 ) 
    {
	fprintf(stderr,"*** WriteSurfGeomFile:"
		" Error return from setnodes is %d\n", error);
	return( error );
    }
    free( nodes_p );
    if ( reportlevel )
	fprintf(stderr," Saved %d nodes\n", 
		numpts);

 /*** Now store the elements that connect the nodes.  ***/

    if ( onesurfgeom->numelements > 0 )
    {
	error = SaveElements( luout,  onesurfgeom->numelements,
			      onesurfgeom->elementsize,
			      0, onesurfgeom->numelements-1, 0,
			      onesurfgeom->elements);

	if ( error < 0 ) 
	{
	    fprintf(stderr,"*** WriteSurfGeomFile:"
		    " Error return from SaveElements is %d\n", error);
	    return( error );
	}
	if ( reportlevel )
	    fprintf(stderr," Saved %d elements\n", 
		    onesurfgeom->numelements);
    } else
	if ( reportlevel )
	    fprintf(stderr," No elements\n");
    
    
 /*** Now save any channels geom with the nodes. ***/

    if ( ! onesurfgeom->channels )
    {
	if ( reportlevel )
	    fprintf(stderr," No channels\n");
	return( error );
    }

 /*** Make sure we have channels that tell us something. ***/

    qsavechannels = FALSE;
    for (i=0; i<numpts; i++)
    {
	if ( onesurfgeom->channels[i] != i )
	{
	    qsavechannels = TRUE;
	    break;
	}
    }
    if ( qsavechannels )
    {
	channels_p = (float *) 
	    calloc((size_t) numpts, sizeof(float));
	if (channels_p == NULL)
	{
	    fprintf(stderr, " Can't get temp buffer for channels\n"
		    " Number of nodes requested is %d\n", numpts);
	    return(ERR_MEM);
	}
	scalartype = 1;
	save_p = channels_p;
	for (i=0; i < numpts; channels_p++, i++)
	{
	    *channels_p = (float) onesurfgeom->channels[i]+1;
	    printf(" channel %d is %d or %f\n", i, 
		   onesurfgeom->channels[i]+1,  *channels_p);
	}
	channels_p = save_p;
	error = setnodescalars_( luout, scalartype, numpts, channels_p);
	if ( error < 0 ) 
	{
	    fprintf(stderr,"*** WriteSurfGeomFile:"
		    " Error return from SaveElements is %d\n", error);
	    return( error );
	}
	if ( reportlevel )
	{
	    printmax = 12;
	    fprintf(stderr," Saved %d channels as scalar\n", numpts);
	}
	free( channels_p );
    } else
    {
	printf(" In surface #%d, channels were the same as node numbers\n"
	       " so not saved\n", surfnum+1 );
    }
    return 0;
}
/*======================================================================*/
long SaveElements( FileInfoPtr luout, long numelements,
		  long elementsize, long elestart, long eleend,
		  long pntstart,
		  long **elements)
{
    
    long i,j,k, jump;
    long error;
    long *elements_p, *isave_p; 
    
/************************************************************************/
    error = 0;
    if ( eleend < elestart || elementsize < 1 )
    {
	fprintf(stderr,"*** SaveElements: illegal values\n"
		" eleend = %d, elestart = %d, elementsize = %d\n",
		eleend, elestart, elementsize );
	return( ERR_MISC );
    }
    elements_p =  (long *) 
	calloc((size_t) (numelements * elementsize), 
	       sizeof(long));
    if (elements_p == NULL)
    {
	fprintf(stderr, " (SaveElements)"
		" Can't get temp buffer for elements\n"
		" Number of elements requested is %d\n", 
		numelements);
	return( ERR_MEM );
    }
    isave_p = elements_p;
    jump = 0;
    for (i = elestart; i <= eleend;
	 i++, jump += elementsize)
    {
	for (j=0; j<elementsize; j++)
	{
	    k = jump + j;
	    elements_p[k] = elements[i][j] - pntstart + 1;
	}
    }
    elements_p = isave_p;
    error = setelements_( luout, elementsize, 
			 numelements, elements_p );
    if ( error < 0 ) 
    {
	fprintf(stderr,"*** SaveElements: error from setelements is %d\n",
		error);
	return( error );
    }
    free( isave_p );
    return 0;
}
