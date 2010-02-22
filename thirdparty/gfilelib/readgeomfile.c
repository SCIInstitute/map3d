 /*** 
   Filename: readgeomfile.c
   Author: Rob MacLeod
 ***/
/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geomlib.h"
#include "cutil.h"
/****************** Prototypes ****************************/
long **LoadElements ( FileInfoPtr thefile,  
		   long numelements, long elementsize,
		   long numscalars, long numvectors, long numtensors,
		   long **surfelements);
long ReadOneSurf( FileInfoPtr luin, char *geomfilename, 
		 long filesurfnum, Surf_Geom *onesurfgeom, 
		  long reportlevel );
/***************** The Code *******************************/

/*======================================================================*/
Surf_Geom *ReadGeomFile (char *infilename, 
			 long ustartsurfnum, long uendsurfnum,
			 long *numsurfsread, long reportlevel)
{
 /*** 
   A routine to read the contents of a .geom file into a surf_geom
   data structure.

   NOTE:  ALL SURFACE NUMBERS BEGIN WITH 0!!!!!!	
   Input:
        infilename	input filename
	ustartsurfnum	first surface number to read -- 0 = First in file
			IF < 0 read all surfaces
	uendsurfnum	last surface number to read
			IF < 0 read all surfaces
	reportlevel	reports back diagnostics if > 0

   Output:
	numsurfsread	the number of surfaces that get read here, and a 
			negative number for an error case. 
   Returns:
        a pointer to surfgeom.
***/
    long error;
    long filesurfnum; /* Different surface numbers */
    long errorlevel, filetype, numfilesurfs, numbsurfs, numtsblocks;
    long startsurfnum, endsurfnum, readsurfnum;
    char filename[100], basefilename[100];
    FileInfoPtr luin;
    Boolean qsettings;
    Surf_Geom *surfgeom = NULL;
/*************************************************************************/

 /*** Open the geometry file. ***/ 

    if ( reportlevel > 1 )
	fprintf(stderr,"In ReadGeomFile\n");
    
 /*** Open the file and get some basic info. ***/

    errorlevel = 1;
    strcpy( basefilename, infilename );
    StripExtension( basefilename );
    strcpy( filename, basefilename );
    strcat( filename, ".geom" );
    if ( ( error = openfile_( filename, 
			       errorlevel, &luin)) < 0 ) {
	*numsurfsread = -1;
	return(NULL);
    }
    getfileinfo_(luin, &filetype, &numfilesurfs, &numbsurfs, &numtsblocks,
		 &qsettings);

    if ( numfilesurfs <= 0 )
    {
	fprintf(stderr,"*** Error in ReadGeomfile: no surfaces in file\n"
		"*** Filename: %s\n", filename);
	return( NULL );
    }
    if ( reportlevel )
	printf(" File %s is open\n"
	       " with %d surfaces\n", filename, numfilesurfs);

 /*** See if there is a specific surface to be read, or we should
      just read all the surfaces in the file. 
 ***/

    startsurfnum = ustartsurfnum;
    endsurfnum = uendsurfnum;
    if ( startsurfnum < 0 ) 
    {
	startsurfnum = 0;
	endsurfnum = numfilesurfs-1;
    } else {
	if ( startsurfnum >= numfilesurfs )
	{
	    startsurfnum = numfilesurfs-1;
	    endsurfnum = numfilesurfs-1;
	}
	if ( endsurfnum < 0 )
	{
	    endsurfnum = numfilesurfs-1;
	} else if ( endsurfnum >= numfilesurfs )
	{
	    endsurfnum = numfilesurfs-1;
	} else if ( endsurfnum < startsurfnum )
	{
	    endsurfnum = startsurfnum;
	}
    }

    if ( reportlevel )
	fprintf(stderr," We will search the geometry file for surfaces"
		" #%d to #%d\n", startsurfnum+1, endsurfnum+1);

 /**********************************************************************

                      Surface Loop

 ***********************************************************************/

    readsurfnum = 0;
    for (filesurfnum=startsurfnum; filesurfnum <= endsurfnum; 
	 filesurfnum++, readsurfnum++)
    {
	if (( surfgeom = AddASurfGeom( surfgeom, 0, numsurfsread ) ) == NULL )
	{
	    fprintf(stderr,
		    "*** Error in ReadGeomfile: could not add a surfgeom\n"
		    "*** Filename: %s\n"
		    "*** At surface  number %d of %d\n",
		    infilename, filesurfnum, endsurfnum);
	    return( NULL );
	}
	if ( reportlevel > 1 )
	    printf(" Added a surface #%d to the surfgeom struct\n",
		   surfgeom[readsurfnum].surfnum+1);

	surfgeom[readsurfnum].surfnum = readsurfnum;
	error = ReadOneSurf( luin, filename, filesurfnum, 
			    &surfgeom[readsurfnum], reportlevel );
	if ( error < 0 ) return( NULL );
	strcpy(surfgeom[readsurfnum].filename, filename);
    }

    closefile_( luin );
    if ( *numsurfsread <= 0 )
    {
	fprintf(stderr,"*** Error in ReadGeomfile: no surfaces read\n"
		"*** Filename: %s\n"
		"*** Desired surface numbers = %d to %d\n",
		infilename, startsurfnum, endsurfnum);
	return( NULL );
    }
    if ( reportlevel )
	fprintf(stderr," Finished reading %d surfaces from geometry file\n",
		*numsurfsread);
    return( surfgeom );
}

/*======================================================================*/
long ReadOneSurf( FileInfoPtr luin, char *geomfilename, 
		 long filesurfnum, Surf_Geom *onesurfgeom, 
		  long reportlevel )
     
{
 /*** Read one surfaces worth of information from a .geom file. 

   Input:
    luin            pointer to open .geom file
    filesurfnum	    surface number in the input file (starting at 0)
    onesurfgeom	    surf_geom structure for this surface
   Returns:
    error   	    < 0 for error situation.

 ***/
    long error;
    long surfnum;
    long numnodes, numnodescalars, numnodevecs, numnodetens;
    long nodenum, index;
    long numelements, elementsize, numelementscalars, numelementvecs;
    long numelementtens;
    long scalarindex, scalartype;
    long nnegchannels;
    float *scalars_p;
    char instring[80];
    Node *tempnodes, *savenodes;
/**********************************************************************/
    surfnum = onesurfgeom->surfnum;
    error = setsurfaceindex_(luin, filesurfnum+1);
    if ( error < 0 ){
	printf("*** In ReadOnsSurf: error setting surfaceindex to %d\n",
	       filesurfnum);
	return(error);
    }

    error = getnodeinfo_(luin, &numnodes, &numnodescalars,
		 &numnodevecs, &numnodetens);
    if ( error < 0 ) return(error);
    if ( reportlevel > 2 )
	fprintf(stderr," In file surface #%d we have %d nodes\n"
		" to load into display surface #%d\n"
		" and the number of scalar sets is %d\n",
		filesurfnum+1, numnodes, surfnum+1, numnodescalars);
    if ( numnodes < 1)
    {
	fprintf(stderr,"*** ReadGeomFile: "
		"*** Something weird here -- you only have %d nodes"
		" in surface %d\n"
		" The file is %s\n", numnodes, filesurfnum+1, geomfilename);
	error = ERR_MISC;
	return(error);
    }
    
 /*** Get the surface text if there is any present. ***/

    getsurfacename_(luin, instring);
    if (strlen( instring) > 2 && strlen(instring) < 100 )
	strcpy( onesurfgeom->label, instring);
    else
	sprintf( onesurfgeom->label,"%s: Surface %d", 
		geomfilename, surfnum+1);
    
 /*********************************************************************

          Points and point links

 *********************************************************************/
 /*** Get some more memory in the surfgeom array for points. ***/

    if ( ( error = SetupSurfPoints( onesurfgeom, numnodes ) ) < 0 )
    {
	fprintf(stderr,"*** ReadGeomFile: "
		"*** Could not get memory for surfpoints"
		" wanted %d nodes for surface %d\n"
		" The file is %s\n", numnodes, filesurfnum+1, geomfilename);
	error = ERR_MEM;
	return(error);
    }

 /*** Now some temporary memory to grab points from the file. ***/

    tempnodes = (NodePtr) calloc( (size_t) numnodes, sizeof(Node) );
    if (tempnodes == NULL)
    {
	fprintf(stderr, " Can't get temp buffer for nodes\n"
		" Number of nodes requested is %d\n", numnodes);
	error = ERR_MEM;
	return(error);
    }
    savenodes = tempnodes;

 /*** Now read in the nodes and add them to the model_pts array.  ***/

    getnodes_(luin, tempnodes);

 /*** Now place them in the surfgeom structure. ***/

    tempnodes = savenodes;
    for (nodenum = 0; nodenum < numnodes;
	 tempnodes++, nodenum++)
    {
	onesurfgeom->nodes[nodenum][CUTIL_X] = tempnodes->x;
	onesurfgeom->nodes[nodenum][CUTIL_Y] = tempnodes->y;
	onesurfgeom->nodes[nodenum][CUTIL_Z] = tempnodes->z;
    }
    
    free(savenodes);    
    
 /*** Load up the associated scalars of the nodes into the channels
       array. 
  ***/
    if (numnodescalars > 0)
    {
	nnegchannels = 0;
	scalars_p = (float *) 
	    calloc((size_t) numnodes, sizeof(float));
	scalarindex = 1;
	getnodescalars_(luin, scalarindex, &scalartype, 
			scalars_p);
	for(index = 0; index < numnodes; index++)
	{
	    if( (long) scalars_p[index] <= 0 )
	    {
		onesurfgeom->channels[index] = -1;
		nnegchannels++;
		if ( nnegchannels < 10 && reportlevel > 2 )
		{
		    fprintf(stderr," In surface #%d, channel #%d is %d\n",
			    surfnum+1, index+1,
			    onesurfgeom->channels[index]);
		}
	    } else
	    {
		onesurfgeom->channels[index] = 
		    (long)scalars_p[index] - 1;
	    }
	}
	if (nnegchannels > 0 )
	{
	    fprintf(stderr,"+++ In ReadGeomFile there were %d negative"
		    " channel numbers in surface #%d\n"
		    "+++ This could cause trouble later !!!\n", 
		    nnegchannels, surfnum+1);
	}
	if ( reportlevel > 2) 
	    fprintf(stderr," Loaded up a set of %d channel numbers"
		    " for surface %d\n", 
		    numnodes, surfnum+1);
	free(scalars_p);
	
 /*** If no scalars were found, load up default channels array. ***/

    } else
    {
	for(index = 0; index < numnodes; index++)
	{
	    onesurfgeom->channels[index] = index;
	}
    }

 /*******************************************************************

                     Read Elements

 *********************************************************************/
 /*** Now get the element (connectivity) information. ***/

    getelementinfo_(luin, &numelements,
		    &elementsize, &numelementscalars,
		    &numelementvecs, &numelementtens);
    
 /*** Load the element info, using the same routine, but with different
      arrays.  ***/

    if ( numelements > 0 )
    {
	if ( reportlevel )
	    printf(" Loading %d elements with LoadElements\n",
		   numelements);
	if (( onesurfgeom->elements = 
	      LoadElements (luin, numelements, elementsize, 
			    numelementscalars, numelementvecs, 
			    numelementtens,
			    onesurfgeom->elements)) == NULL)
	{
	    fprintf(stderr,"*** Error return from LoadElements\n");
	    return(ERR_MISC);
	}
	onesurfgeom->numelements = numelements;
	onesurfgeom->elementsize = elementsize;
    } else
    {
	fprintf(stderr, " No elements found for surface %d\n",
		surfnum+1);
	return( 0 );
    }
    return 0;
}


/*====================================================================*/
long **LoadElements ( FileInfoPtr thefile,  
		   long numelements, long elementsize,
		   long numscalars, long numvectors, long numtensors,
		   long **surfelements)

/*** 
  A routine to load the elements from the file to the appropriate
  buffer.  The function is set up to handle all different sizes of
  element so the user can call it with segments, triangle, or tetrahedra.

Input: 
 thefile    	pointer to an open .data file
 surfacenum 	current surface number to load data into
 numelements	number of elements to load
 elementsize	number of points in each element (2--4)
 numscalars 	number of sets of scalars associated with the elements
 numvectors 	number of sets of vectors associated with the elements
 numtensors 	number of sets of tensors associated with the elements

In/Output:
 surfelements   pointer to the place for surface elements to be stored 
                in the surf_geom structure

Returns:
 error code = 0 for no error, < 0 for error, > 0 contains number of elements
              stored in the buffer
 ***/
/************************************************************************/
{
    long i=0,j,k;
    long index, jump;
    long error;
    long *tempele, *saveele;
/************************************************************************/
 /*** Dummy statement to keep compiler happy for the moment ***/
    if ( numscalars == numvectors == numtensors) i=0;
    i++;

    if ( numelements < 1 ) 
    {
	fprintf(stderr,"*** Error in LoadElements since numelements = %d\n",
		numelements);
	return (NULL);
    }

 /*** Get temp memory for the elements buffer. ***/

    tempele =  (long *) calloc((size_t) (numelements * elementsize), 
			       sizeof(long));
    if (tempele == NULL)
    {
	fprintf(stderr, "*** Error in LoadElements\n"
		    "*** Can't get temp buffer for tempele\n");
	return(NULL);
    }
    saveele = tempele;
    if ( ( error = getelements_(thefile, tempele) ) < 0 )
    {
	printf("*** Error in LoadElement reading elements from file is %d\n",
	       error);
	return(NULL);
    }
    jump = 0;

 /*** Set up memory for the structure version of the elements. ***/

    if ( surfelements != NULL ) free( surfelements );
    if ( ( surfelements = Alloc_lmatrix( numelements, elementsize ) ) 
	== NULL )
    {
	fprintf(stderr,"*** Error in LoadElements\n"
		"*** Can't get memory for surfelements\n");
	return(NULL);
    }
    jump = 0;
    for ( index=0; index < numelements; index++, jump += elementsize )
    {
	for (j=0; j<elementsize; j++)
	{
	    k = jump + j;
	    surfelements[index][j] = tempele[k] - 1;
	}
    }

    free( saveele );
    return surfelements;
}

