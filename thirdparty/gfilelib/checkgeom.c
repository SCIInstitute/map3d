 /*** 
   Filename: checkgeom.c
   Author: Rob MacLeod
 ***/
/****************** Includes *****************************/
#include "geomlib.h"
#include "cutil.h"
/****************** Externals *****************************/
/****************** Prototypes ****************************/
long CheckElementDoubles( Surf_Geom *onesurfgeom );
void OrderEnodes( long *oneelement, long numnodes, long enodes[] );
long CheckElementPoints( Surf_Geom *onesurfgeom );
long CheckElementValidity( Surf_Geom *onesurfgeom );
long CheckOneSurfgeom( Surf_Geom *onesurfgeom );

/***************** The Code *******************************/
/*================================================================*/
long CheckGeom( Surf_Geom *surfgeom, long numsurfs )
{
 /*** Check all surface of the .geom file for sanity. ***/

    long surfnum;
    long error = 0;
/******************************************************************/

 /*** Loop through each surface. ***/

    for( surfnum=0; surfnum < numsurfs; surfnum++ )
    {
	error =  CheckOneSurfgeom( &surfgeom[surfnum] );
    }
    return( error );
}

/*================================================================*/
long CheckOneSurfgeom( Surf_Geom *onesurfgeom )
{
 /*** Check one surface of geometry for sanity. ***/

    long error1 = 0, error2 = 0, error3 = 0;
/******************************************************************/

 /*** Check elements. ***/

    if (onesurfgeom->numelements > 0)
    {
	error1 = CheckElementPoints( onesurfgeom);
	error2 = CheckElementValidity( onesurfgeom);
	error3 = CheckElementDoubles( onesurfgeom);
    }
    if ( error1 + error2 + error3 < 0 )
	return( error1 + error2 + error3 );
    else 
	return( 0 );
}
/*================================================================*/

long CheckElementDoubles( Surf_Geom *onesurfgeom )
{
    
 /*** See whether there are any doubled elements in the geometry file. 
      Return: error value = ERR_VALID if things are not all OK.
 ***/
    
    long i,j, k, error = 0;
    long nmatches;
    long enodes[4], tenodes[4];
    long surfnum;
    char errstring[100];
 /**********************************************************************/
    if (! onesurfgeom ) return( ERR_VALID );
    surfnum = onesurfgeom->surfnum;
    if (! onesurfgeom->elements ) return( ERR_VALID );
    
    for( i=0; i < onesurfgeom->numelements; i++ )
    {
	OrderEnodes( onesurfgeom->elements[i], onesurfgeom->elementsize, 
		     enodes );
	for ( j=i+1; j < onesurfgeom->numelements; j++)
	{
	    OrderEnodes( onesurfgeom->elements[j], onesurfgeom->elementsize, 
			 tenodes );
	    nmatches = 0;
	    for( k=0; k < onesurfgeom->elementsize; k++ )
	    {
		if ( enodes[k] == tenodes[k] )
		    nmatches++;
	    }
	    if ( nmatches == onesurfgeom->elementsize )
	    {
		sprintf(errstring,"In surface %d, numbers %d (%d, %d, %d)"
			"\n and %d( %d, %d, %d)",
			surfnum+1, i+1, 
			onesurfgeom->elements[i][0],
			onesurfgeom->elements[i][1],
			onesurfgeom->elements[i][2],
			j+1,
			onesurfgeom->elements[j][0],
			onesurfgeom->elements[j][1],
			onesurfgeom->elements[j][2] );
		ReportError("CheckDoubleElements", 
			    "found two matching elements",
			    ERR_VALID, errstring);
		error = ERR_VALID;
	    }
	}
    }
    return( error );
}

/*======================================================================*/

void OrderEnodes( long *oneelement, long numnodes, long enodes[] )
{

 /*** Order the nodes of the element and return then in enodes. ***/

    long i,j, minnode;
/**********************************************************************/
 /*** Find the smallest node ***/
    minnode = 1000000;
    for (i=0; i<numnodes; i++)
    {
	if( oneelement[i] < minnode )
	    minnode = oneelement[i];
    }

 /*** Now load it and look for the rest. ***/

    enodes[0] = minnode;

    for( i=1; i<numnodes; i++ )
    {
	minnode = 1000000;
	for ( j=0; j<numnodes; j++ )
	{
	    if( oneelement[j] > enodes[i-1] && oneelement[j] < minnode )
		minnode = oneelement[j];
	}
	enodes[i] = minnode;
    }
}

/*======================================================================*/

long CheckElementPoints( Surf_Geom *onesurfgeom )
{
 /*** Check all the points pointed at by the elements and make
      sure they are all valid point addresses. 

      Return: error value = ERR_VALID if things are not all OK.

 ***/

    long i,j;
    long numpts, error = 0;
/**********************************************************************/
    if (! onesurfgeom ) return( ERR_VALID );
    numpts = onesurfgeom->numpts;
    if (! onesurfgeom->elements ) return( ERR_VALID );
    for( i=0; i < onesurfgeom->numelements; i++ )
    {
	for(j=0; j < onesurfgeom->elementsize; j++)
	{
	    if ( onesurfgeom->elements[i][j] >= numpts )
	    {
		fprintf(stderr,"*** In CheckPointValidity"
			" error in elements #%d of length %d\n"
			"    The point %d is beyond the last"
			" one in the dataset at %d\n",
			i+1, onesurfgeom->elementsize,
			onesurfgeom->elements[i][j]+1, numpts);
		error =  ERR_VALID;
	    } else if ( onesurfgeom->elements[i][j] < 0 )
	    {
		fprintf(stderr,"*** In CheckPointValidity"
			" error in element #%d of length %d\n"
			"    The point #%d = %d is zero or less\n",
			i+1, onesurfgeom->elementsize, j+1, 
			onesurfgeom->elements[i][j]+1 );
		error = ERR_VALID;
	    }
	}
    }
    return( error );
}

/*================================================================*/

long CheckElementValidity( Surf_Geom *onesurfgeom )
{
    
 /*** See whether all connectivites in the elements are valid 

      Return: error value = ERR_VALID if things are not all OK.

***/
    
    long i,j, k, l, error = 0;
    long surfnum;
 /**********************************************************************/
    if (! onesurfgeom ) return( ERR_VALID );
    surfnum = onesurfgeom->surfnum;
    if (! onesurfgeom->elements ) return( ERR_VALID );
    
    for( i=0; i < onesurfgeom->numelements; i++ )
    {
	j = 0;
	for ( k=j+1; k < onesurfgeom->elementsize; k++)
	{
	    if ( onesurfgeom->elements[i][j] == onesurfgeom->elements[i][k] )
	    {
		fprintf(stderr,"+++ In CheckElementValidity in surface"
			"#%d\n"
			"    Element #%d of size %d "
			" has pointers to the same node\n"
			"    Node numbers are = %d, %d",
			surfnum+1, i+1, onesurfgeom->elementsize, 
			onesurfgeom->elements[i][0]+1, 
			onesurfgeom->elements[i][1]+1);
		for (l=2; l < onesurfgeom->elementsize; l++)
		    fprintf(stderr,", %d", onesurfgeom->elements[i][l]+1);
		fprintf(stderr,"\n");
		error = ERR_VALID;
	    }
	}
    }
    return( error );
}

/*======================================================================*/

long *CheckPointLinks( Surf_Geom *onesurfgeom, long *error, 
		       long *numunlinked )
{
 /*** Check all the points and locate any that are not part of an element.

      Return: 
      
      pointer        to list of unlinked point numbers
      error         = ERR_VALID if things are not all OK.
      *numunlinked  = number of unlinked points in pointer

 ***/

    long i,j;
    long numpts;
    long *plist=NULL;
/**********************************************************************/
    *error = 0;
    if (! onesurfgeom ) 
    {
	*error = ERR_VALID;
	return( NULL );
    }
    numpts = onesurfgeom->numpts;
    plist = (long *) calloc( (size_t) numpts, sizeof(long));
    if ( plist == NULL )
    {
	ReportError("CheckPointLinks", "error getting plist memory",
		    ERR_MEM, "");
	*error = ERR_MEM;
	return( NULL );
    }
    if (! onesurfgeom->elements )
    {
	*error = ERR_VALID;
	return( NULL );
    }
    for( i=0; i < onesurfgeom->numelements; i++ )
    {
	for(j=0; j < onesurfgeom->elementsize; j++)
	{
	    plist[onesurfgeom->elements[i][j]] = 1;
	}
    }
    return( NULL );
}


/*======================================================================*/

long RemoveAnElement( Surf_Geom *onesurfgeom, long elementnum )
{
 /*** Remove element number 'elementnum' from the current list 
      of elements ion onesurfgeom.  Slide all the rest down in memory
      so that we have no gaps. 

      Return an error if we have no elements or the elementnum is out of
          bounds.
 ***/

    long error = 0, elenum, nodenum;
/**********************************************************************/
    
    if ( onesurfgeom->numelements < 1 )
    {
	ReportError("RemoveAnElement", "No elements in the surface",
		    ERR_MISC, "");
	return( ERR_MISC );
    }
    if ( elementnum < 0 || elementnum > onesurfgeom->numelements - 1 )
    {
	ReportError("RemoveAnElement", "illegal elementnum value",
		    ERR_MISC, "");
	return( ERR_MISC );
    }

    for ( elenum = elementnum; elenum < onesurfgeom->numelements-1; elenum++)
    {
	for( nodenum=0; nodenum < onesurfgeom->elementsize; nodenum++ )
	{
	    onesurfgeom->elements[elenum][nodenum] = 
		onesurfgeom->elements[elenum+1][nodenum];
	}
    }
    onesurfgeom->numelements--;
    return( error );
}


