
 /*** 
   Filename: geomlib.c
   The eventual home of some of the routines that manage geometry.

 ***/
/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geomlib.h"
#include "cutil.h"
#include "landmarks.h"
/****************** Externals *****************************/
/****************** Prototypes ****************************/
Land_Mark *AddALandmarkSurf( long displaysurfnum );
void CopyOneSurfGeom( Surf_Geom *destsurfgeom, Surf_Geom *sourcesurfgeom );
long SetupSurfPoints( Surf_Geom *onesurfgeom, long numpts );

/***************** The Code *******************************/
Surf_Geom *AddASurfGeom( Surf_Geom *surfgeom, long numpts, long *numsurfs )
{

 /*** Add a new member to the list of surface geometries.
   Input:
     surfgeom	    surf_geom structure for the whole geometry
		    If numpts > 0, we allocate space for nodes and channels
     numpts 	    number of points that will go in the geometry
                    if <= 0, do nothing with this info
   In/Output:
     numsurfs	    at input, the current number of surfaces, at output, 
		    the new number of surfaces
   Return:
     	    	    number of surfaces in the array of structures
                     	or value < 0 for error
  ***/
    long newsurfnum;
    long error = 0;
    long numgeomsurfs; 
/**********************************************************************/
    numgeomsurfs = *numsurfs;

 /*** If this is the first surface, allocate. ***/
    
    if ( surfgeom == NULL )
    {
	numgeomsurfs = 1;
	if ( ( surfgeom = (Surf_Geom *) 
	      calloc( (size_t) (numgeomsurfs), sizeof(Surf_Geom)) ) == NULL )
	{
	    fprintf(stderr,"*** AddAGeomSurf: error getting first memory\n");
	    return( NULL );
	}
	
	
 /*** If not, reallocate. ***/

    } else
    {
	numgeomsurfs++;
	if ( ( surfgeom = 
	      (Surf_Geom *) realloc( surfgeom, (size_t)
				    (numgeomsurfs) * sizeof(Surf_Geom))
	      ) == NULL )
	{
	    fprintf(stderr,"*** AddAGeomSurf: error reallocating memory\n");
	    return( NULL );
	}
    }
    newsurfnum = numgeomsurfs-1;

    surfgeom[newsurfnum].nodes = NULL;
    surfgeom[newsurfnum].elements = NULL;
    surfgeom[newsurfnum].channels = NULL;
    strcpy(surfgeom[newsurfnum].label,"");
    strcpy(surfgeom[newsurfnum].filepath,"");

 /*** Now allocate some memory for the points in the surface. ***/

    if ( numpts > 0 )
    {
	error = SetupSurfPoints( &surfgeom[newsurfnum], numpts );
	if ( error < 0 )
	{
	    ReportError("AddAGeomSurf", "Error return from SetupSurfPoints",
			0, "");
	    return( NULL );
	}
    }

 /*** Set up some parameters. ***/

    surfgeom[newsurfnum].surfnum = newsurfnum;
    surfgeom[newsurfnum].numpts = numpts;
    surfgeom[newsurfnum].numelements = 0;
    surfgeom[newsurfnum].elementsize = 0;
    *numsurfs = numgeomsurfs;
    return surfgeom;
}

/*======================================================================*/

long SetupSurfPoints( Surf_Geom *onesurfgeom, long numpts )
{

 /*** Set up the buffers that are related to points in a surf_geom 
   structure. ***/

/**********************************************************************/

    if ( numpts < 1 ) 
    {
	ReportError("SetupSurfPoints", "numpts < 1", ERR_MISC, "" );
	return( ERR_MISC );
    }

    if (( onesurfgeom->nodes = Alloc_fmatrix( numpts, 3 ))
	== NULL )
    {
	fprintf(stderr,"*** SetupSurfPoints: error getting memory"
		" for points\n");
	return( ERR_MEM );
    }
    if (( onesurfgeom->channels = (long *) 
	 calloc( (size_t) numpts, sizeof(long) ))
	== NULL )
    {
	fprintf(stderr,"*** SetupSurfPoints: error getting memory"
		" for channels\n");
	return( ERR_MEM );
    }
    onesurfgeom->numpts = numpts;
    return 0;
}
/*====================================================================*/

Surf_Geom *DeleteASurfGeom( Surf_Geom *surfgeom, long dsurfnum, 
			    long *numsurfs )
{

 /*** Delete a new member from the list of surface geometries.
      and free all memory when the last surface is govne
   Input:
     surfgeom	    surf_geom structure for the whole geometry
     dsurfnum 	    number of the surface to delete
                    if <= 0, delete all surfaces
   In/Output:
     numsurfs	    at input, the current number of surfaces, at output, 
		    the new number of surfaces
   Return:
     	    	    number of surfaces in the array of structures
                     	or value < 0 for error
  ***/
    long surfnum, firstsurf, lastsurf;
    long numgeomsurfs; 
/**********************************************************************/
    numgeomsurfs = *numsurfs;

 /*** Deallocate the surface(s) we want to delete.  ***/

    if ( dsurfnum < 0 )
    {
	firstsurf = 0;
	lastsurf = numgeomsurfs-1;
    } else
    {
	firstsurf = lastsurf = dsurfnum;
    }

    for (surfnum = firstsurf; surfnum <= lastsurf; surfnum++ )
    {
	free( surfgeom[surfnum].nodes );
	if ( surfgeom[surfnum].elements != NULL ) 
	    free(surfgeom[surfnum].elements);
	if ( surfgeom[surfnum].channels != NULL ) 
	    free(surfgeom[surfnum].channels);
	numgeomsurfs--;
    }

 /*** If we have no more surfaces left, deallocate the whole thing ***/

    if ( numgeomsurfs <= 0 ) 
    {
	free( surfgeom );
	*numsurfs = 0;
	return( NULL );
    }

 /*** Slide all the other surfaces down a notch if we have deleted
      a middle surface and there are more than one surfaces left. ***/

    if( dsurfnum < numgeomsurfs-1 && numgeomsurfs > 1 )
    {
	for (surfnum = dsurfnum; surfnum < numgeomsurfs; surfnum++ )
	{
	    CopyOneSurfGeom( &surfgeom[surfnum], &surfgeom[surfnum+1] );
	}
    }
    *numsurfs = numgeomsurfs;
    return( surfgeom );
   
}
/*====================================================================*/
void CopyOneSurfGeom( Surf_Geom *destsurfgeom, Surf_Geom *sourcesurfgeom )
{
 /*** Copy the contents of one surface geom into another. ***/

    destsurfgeom->surfnum = sourcesurfgeom->surfnum;
    destsurfgeom->numpts = sourcesurfgeom->numpts;
    destsurfgeom->numelements = sourcesurfgeom->numelements;
    destsurfgeom->elementsize = sourcesurfgeom->elementsize;
    destsurfgeom->nodes = sourcesurfgeom->nodes;
    destsurfgeom->elements = sourcesurfgeom->elements;
    destsurfgeom->channels = sourcesurfgeom->channels;
    strcpy(destsurfgeom->label, sourcesurfgeom->label);
    strcpy(destsurfgeom->filepath, sourcesurfgeom->filepath);
    strcpy(destsurfgeom->filename, sourcesurfgeom->filename);
    return;
}
