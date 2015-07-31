/*** 
  Filename: mallocsubs.c
  Author: Rob MacLeod (with a little help from Numerical Recipes)
  
  Some subroutines to manage memory in a sensible (I hope) way.

   Last update: Thu Oct 30 13:57:22 1997
     - added character matirix routines
 ***/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "cutil.h"

/*--------------------------------------------------------------------*/
float **Alloc_fmatrix(long nrh, long nch )
{
 /*** 
   Allocate space for a two-dimensional array of size 'nrh' rows
   by 'nch' columns. 
   Returns a double-index pointer to the block of memory.
 ***/
    long i;
    float **m;
/**********************************************************************/
    m = (float **) calloc((size_t) nrh, sizeof(float*));
    if (m == NULL)
    {
	fprintf(stderr," In Alloc_fmatrix, memory allocation failure\n");
	return (NULL);
    }

    for(i=0; i<nrh; i++)
    {
	m[i] = (float *) calloc((size_t) nch, sizeof(float));
	if (m[i] == NULL)
	{
	    fprintf(stderr," In Alloc_fmatrix, memory allocation failure "
		    "looking for %d columns in row %d of matrix\n",
		    nch, i);
	    return(NULL);
	}
    }
    return m;
}

/*--------------------------------------------------------------------*/
float **Grow_fmatrix(float** m, long nrl, long nrh, long ncl, long nch )
{
 /*** 
   Expand the number of rows in the matrix from 'nrl' to 'nrh' and the number
   of colums from 'ncl' to 'nch'.  If either of these pairs are equal, 
   then no expansion of that dimension is performed.
 ***/
    long i;
/**********************************************************************/
    if( nrh > nrl )
    {
/*	fprintf(stderr," Expanding number of rows from %d to %d\n",
		nrl, nrh);
*/
	m = (float **) realloc(m, (size_t) (nrh) * sizeof(float*));
	if (m == NULL)
	{
	    fprintf(stderr," In Grow_fmatrix, memory allocation failure\n");
	    return(NULL);
	}
    }

 /*** 
  If the upper number of columns is larger then the lower, assume the
  number of columns is to be expanded from 'ncl' to 'nch'.  This applies
  only to the rows already defined.  All the new rows will be set immediately
  to 'nch'.
 ***/

    if ( nch > ncl ) 
    {
/*
	fprintf(stderr," Expanding number of columns from %d to %d\n",
		ncl, nch);
*/

 /*** Expand the existing rows to the new column width. ***/

	for(i=0; i<nrl; i++)
	{
	    m[i] = (float *) realloc(m[i], (size_t) nch * sizeof(float));
	    if (m[i] == NULL)
	    {
		fprintf(stderr," In Grow_fmatrix, memory reallocation failure "
			"for row %d of matrix\n", i+1);
		return(NULL);
	    }
	}
    }

 /*** Set the new rows up here. ***/

    if ( nrh > nrl ) 
    {
	for(i=nrl; i<nrh; i++)
	{
	    m[i] = (float *) calloc( (size_t) nch, sizeof(float));
	    if (m[i] == NULL)
	    {
		fprintf(stderr," In Grow_fmatrix, memory allocation failure "
			"looking for %d columns of row %d of matrix\n",
			nch, i);
		return(NULL);
	    }
	}
    }
    return m;
}

/*--------------------------------------------------------------------*/
void Free_fmatrix(float** m, long nrh )
{
 /*** Free up the memory from a floating point matrix.
   Input:
    m	    pointer to the array of pointer
    nrh	    current number of rows in the matrix
 ***/
    long i;
/**********************************************************************/

    for ( i=nrh-1; i>=0; i-- ) free( (float*) m[i] );
    free( (float*) m );
}

/*--------------------------------------------------------------------*/
long **Alloc_lmatrix(long nrh, long nch )
{
 /*** 
   Allocate space for a two-dimensional array of size 'nrh' rows
   by 'nch' columns. 
   Returns a double-index pointer to the block of memory.
   Data type: *** long ***
 ***/
    long i;
    long **m;
/**********************************************************************/
    m = (long **) calloc((size_t) nrh, sizeof(long*));
    if (m == NULL)
    {
	fprintf(stderr," In Alloc_lmatrix, memory allocation failure\n");
	return (NULL);
    }

    for(i=0; i<nrh; i++)
    {
	m[i] = (long *) calloc((size_t) nch, sizeof(long));
	if (m[i] == NULL)
	{
	    fprintf(stderr," In Alloc_lmatrix, memory allocation failure "
		    "looking for %d columns of row %i of matrix\n",
		    nch, i);
	    return(NULL);
	}
    }
    return m;
}

/*--------------------------------------------------------------------*/
long **Grow_lmatrix(long** m, long nrl, long nrh, long ncl, long nch )
{
 /*** 
   Expand the number of rows in the matrix from 'nrl' to 'nrh' and the number
   of colums from 'ncl' to 'nch'.  If either of these pairs are equal, 
   then no expansion of that dimension is performed.
   Data type: *** long ***
 ***/
    long i;
/**********************************************************************/
    if( nrh > nrl )
    {
	/*
	fprintf(stderr," Expanding number of rows from %d to %d\n",
		nrl, nrh);
		*/
	m = (long **) realloc(m, (size_t) (nrh) * sizeof(long*));
	if (m == NULL)
	{
	    fprintf(stderr," In Grow_lmatrix, memory allocation failure\n");
	    return(NULL);
	}
    }

 /*** 
  If the upper number of columns is larger then the lower, assume the
  number of columns is to be expanded from 'ncl' to 'nch'.  This applies
  only to the rows already defined.  All the new rows will be set immediately
  to 'nch'.
 ***/

    if ( nch > ncl ) 
    {
/*
	fprintf(stderr," Expanding number of columns from %d to %d\n",
		ncl, nch);
*/

 /*** Expand the existing rows to the new column width. ***/

	for(i=0; i<nrl; i++)
	{
	    m[i] = (long *) realloc(m[i], (size_t) nch * sizeof(long));
	    if (m[i] == NULL)
	    {
		fprintf(stderr," In Grow_lmatrix, memory reallocation failure "
			"for row %d of matrix\n", i+1);
		return(NULL);
	    }
	}
    }

 /*** Set the new rows up here. ***/

    if ( nrh > nrl ) 
    {
	for(i=nrl; i<nrh; i++)
	{
	    m[i] = (long *) calloc( (size_t) nch, sizeof(long));
	    if (m[i] == NULL)
	    {
		fprintf(stderr," In Grow_lmatrix, memory allocation failure "
			"looking for %d columns of row %d of matrix\n",
			nch, i);
		return(NULL);
	    }
	}
    }
    return m;
}

/*--------------------------------------------------------------------*/
void Free_lmatrix(long** m, long nrh )
{
 /*** Free up the memory from a long matrix.
   Input:
    m	    pointer to the array of pointer
    nrh	    current number of rows in the matrix
 ***/
    long i;
/**********************************************************************/

    for ( i=nrh-1; i>=0; i-- ) free( (long*) m[i] );
    free( (long*) m );
}

/*--------------------------------------------------------------------*/
char **Alloc_cmatrix(long nrh, long nch )
{
 /*** 
   Allocate space for a two-dimensional array of size 'nrh' rows
   by 'nch' columns. 
   Returns a double-index pointer to the block of memory.
   Data type: *** char ***
 ***/
    long i;
    char **m;
/**********************************************************************/
    m = (char **) calloc((size_t) nrh, sizeof(char*));
    if (m == NULL)
    {
	fprintf(stderr," In Alloc_cmatrix, memory allocation failure\n");
	return (NULL);
    }

    for(i=0; i<nrh; i++)
    {
	m[i] = (char *) calloc((size_t) nch, sizeof(char));
	if (m[i] == NULL)
	{
	    fprintf(stderr," In Alloc_cmatrix, memory allocation failure "
		    "looking for %d columns of row %i of matrix\n",
		    nch, i);
	    return(NULL);
	}
    }
    return m;
}

/*--------------------------------------------------------------------*/
char **Grow_cmatrix(char** m, long nrl, long nrh, long ncl, long nch )
{
 /*** 
   Expand the number of rows in the matrix from 'nrl' to 'nrh' and the number
   of colums from 'ncl' to 'nch'.  If either of these pairs are equal, 
   then no expansion of that dimension is performed.
   Data type: *** char ***
 ***/
    long i;
/**********************************************************************/
    if( nrh > nrl )
    {
	/*
	fprintf(stderr," Expanding number of rows from %d to %d\n",
		nrl, nrh);
		*/
	m = (char **) realloc(m, (size_t) (nrh) * sizeof(char*));
	if (m == NULL)
	{
	    fprintf(stderr," In Grow_cmatrix, memory allocation failure\n");
	    return(NULL);
	}
    }

 /*** 
  If the upper number of columns is larger then the lower, assume the
  number of columns is to be expanded from 'ncl' to 'nch'.  This applies
  only to the rows already defined.  All the new rows will be set immediately
  to 'nch'.
 ***/

    if ( nch > ncl ) 
    {
/*
	fprintf(stderr," Expanding number of columns from %d to %d\n",
		ncl, nch);
*/

 /*** Expand the existing rows to the new column width. ***/

	for(i=0; i<nrl; i++)
	{
	    m[i] = (char *) realloc(m[i], (size_t) nch * sizeof(char));
	    if (m[i] == NULL)
	    {
		fprintf(stderr," In Grow_cmatrix, memory reallocation failure "
			"for row %d of matrix\n",i+1);
		return(NULL);
	    }
	}
    }

 /*** Set the new rows up here. ***/

    if ( nrh > nrl ) 
    {
	for(i=nrl; i<nrh; i++)
	{
	    m[i] = (char *) calloc( (size_t) nch, sizeof(char));
	    if (m[i] == NULL)
	    {
		fprintf(stderr," In Grow_cmatrix, memory allocation failure "
			"looking for %d columns of row %d of matrix\n",
			nch, i);
		return(NULL);
	    }
	}
    }
    return m;
}

/*--------------------------------------------------------------------*/
void Free_cmatrix(char** m, long nrh )
{
 /*** Free up the memory from a char matrix.
   Input:
    m	    pointer to the array of pointer
    nrh	    current number of rows in the matrix
 ***/
    long i;
/**********************************************************************/

    for ( i=nrh-1; i>=0; i-- ) free( (char*) m[i] );
    free( (char*) m );
}

/*--------------------------------------------------------------------*/
float *Alloc_farray(long nrows, float *array, char *name )
{
 /*** 
   Allocate space for a real vector array of size 'nrows' rows.

   Input:
   nrows    	size of desired array
   *array   	current value of array pointer
   *name    	character array containing type of array to be used in 
                 error messages
   Output:
                updated value of array
   Returns:
                pointer to the block of memory, or NULL if error.
 ***********************************************************************/

 /*** If array is empty now, then declare it afresh. ***/

    if ( array == NULL )
    {
	if (( array = (float*)
	     calloc((size_t) nrows,
		    sizeof(float))) == NULL)
	{
	    fprintf(stderr,"Can't get initial memory for %s\n",
		    name);
	    return NULL;
	}
    }

 /*** Or else expand an existing array. ***/
    else
    {
	if (( array = (float*)
	     realloc(array, (size_t) (nrows) *
		     sizeof(float))) == NULL)
	{
	    fprintf(stderr,"Can't get memory to grow %s array\n",
		    name);
	    return NULL;
	}
    }
    return array;
}
/*--------------------------------------------------------------------*/
long *Alloc_larray(long nrows, long *array, char *name )
{
 /*** 
   Allocate space for a long vector array of size 'nrows' rows.

   Input:
   nrows    	size of desired array
   *array   	current value of array pointer
   *name    	character array containing type of array to be used in 
                 error messages
   Output:
                updated value of array
   Returns:
                pointer to the block of memory, or NULL if error.
 ***********************************************************************/

 /*** If array is empty now, then declare it afresh. ***/

    if ( array == NULL )
    {
	if (( array = (long*)
	     calloc( (size_t) nrows,
		    sizeof(long) )) == NULL)
	{
	    fprintf(stderr,"Can't get initial memory for %s\n",
		    name);
	    return NULL;
	}
    }

 /*** Or else expand an existing array. ***/
    else
    {
	if (( array = (long*)
	     realloc(array, (size_t) (nrows) *
		     sizeof(long))) == NULL)
	{
	    fprintf(stderr,"Can't get memory to grow %s array\n",
		    name);
	    return NULL;
	}
    }
    return array;
}
/*--------------------------------------------------------------------*/
short *Alloc_sarray(long nrows, short *array, char *name )
{
 /*** 
   Allocate space for a short vector array of size 'nrows' rows.

   Input:
   nrows    	size of desired array
   *array   	current value of array pointer
   *name    	character array containing type of array to be used in 
                 error messages
   Output:
                updated value of array
   Returns:
                pointer to the block of memory, or NULL if error.
 ***********************************************************************/

 /*** If array is empty now, then declare it afresh. ***/

    if ( array == NULL )
    {
	if (( array = (short*)
	     calloc((size_t) nrows,
		    sizeof(short))) == NULL)
	{
	    fprintf(stderr,"Can't get initial memory for %s\n",
		    name);
	    return NULL;
	}
    }

 /*** Or else expand an existing array. ***/
    else
    {
	if (( array = (short*)
	     realloc(array, (size_t) (nrows) *
		     sizeof(short))) == NULL)
	{
	    fprintf(stderr,"Can't get memory to grow %s array\n",
		    name);
	    return NULL;
	}
    }
    return array;
}
