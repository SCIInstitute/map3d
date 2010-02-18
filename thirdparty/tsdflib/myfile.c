/* FILENAME:  myfile.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  Open a file and take care of byte swapping
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef MYFILE_C
#define MYFILE_C 1

#include "myfile.h"

int	mfByteSwap()
{
	/* Detect byte swapping */
	/* This function loads a 2 byte value into test */
	/* and then tests the values of both bytes to */
	/* determine the order in which they are written */

	short test;
	unsigned char *ptr;
	test = 0x00FF;
	ptr = (unsigned char *) &(test);
	if (ptr[0]) return(1); /* Bytes are swapped, we must be running on some INTEL machine */
	return(0);
}

void	mfSwapBytes(void *buffer,size_t count)
{
	unsigned char 	*ptr;
	int				m,m4,p;
	unsigned char	temp;

	m = count;
	ptr = (unsigned char *)buffer;

	m4 = m-4;
	
	/* unroll loop for swapping bytes */
	/* to increase efficiency */

	for (p=0;p<m4;)
	{
		temp = ptr[p]; ptr[p] = ptr[p+1]; ptr[p+1] = temp; temp = ptr[p+2]; ptr[p+2] = ptr[p+3]; ptr[p+3] = temp; 
		temp = ptr[p+4]; ptr[p+4] = ptr[p+5]; ptr[p+5] = temp; temp = ptr[p+6]; ptr[p+6] = ptr[p+7]; ptr[p+7] = temp; 
		p += 8;	
	}
	for (;p<m;p+=2)	{temp = ptr[p]; ptr[p] = ptr[p+1]; ptr[p+1] = temp;}

	/* end of byte swapping */
}

void	mfSwapBytes4(void *buffer,size_t count)
{
	unsigned char 	*ptr;
	int 		m,m8,p;
	unsigned char	temp;

	m = count;
	ptr = (unsigned char *)buffer;

	m8 = m-8;
	
	for (p=0;p<m8;)
	{
		temp = ptr[p]; ptr[p] = ptr[p+3]; ptr[p+3] = temp; temp = ptr[p+1]; ptr[p+1] = ptr[p+2]; ptr[p+2] = temp;  
		temp = ptr[p+4]; ptr[p+4] = ptr[p+7]; ptr[p+7] = temp; temp = ptr[p+5]; ptr[p+5] = ptr[p+6]; ptr[p+6] = temp; 
		p += 8;	
	}
	for (;p<m;p+=4)	
	{
		temp = ptr[p]; ptr[p] = ptr[p+3]; ptr[p+3] = temp; temp = ptr[p+1]; ptr[p+1] = ptr[p+2]; ptr[p+2] = temp; 
	}
}

void	mfSwapBytes8(void *buffer,size_t count)
{
	unsigned char 	*ptr;
	int 		m,p;
	unsigned char	temp;

	m = count;
	ptr = (unsigned char *)buffer;

	for (p=0;p<m;p+=8)	
	{
		temp = ptr[p]; ptr[p] = ptr[p+7]; ptr[p+7] = temp; temp = ptr[p+1]; ptr[p+1] = ptr[p+6]; ptr[p+6] = temp;
		temp = ptr[p+2]; ptr[p+2] = ptr[p+5]; ptr[p+5] = temp; temp = ptr[p+3]; ptr[p+3] = ptr[p+4]; ptr[p+4] = temp;
	}
}


MFILE 	*mfopen(char *filename,char *mode)
{
	/* Open a file and detect byte swapping */

	MFILE 	*mfile;

	mfile = (MFILE *) calloc(sizeof(MFILE),1);
	if (mfile == NULL) return(NULL);
	mfile->fptr = fopen(filename,mode);
	if (mfile->fptr == NULL) {free(mfile); return(NULL); }
	mfile->byteswap = mfByteSwap();

	return(mfile);
}

void	mfclose(MFILE *mfile)
{
	/* Close the file and release the extra struct we used to store byte swapping */

	if (mfile)
	{
		if(mfile->fptr) fclose(mfile->fptr);
		free(mfile);
	}
}

size_t 	mfread(void *buffer,size_t size,size_t count,MFILE *mfile, int type)
{
	/* Lengths of the different objects. */
	/* If the lengths in bytes do not correspond with the machines sizeof() operator */
	/* a warning is issued */
	/* FUTURE: implementations may correct this by padding extra zeros in between */

	size_t lengths[] = {mfsNONE,mfsCHAR,mfsSHORT,mfsINT,mfsLONGLONG,mfsFLOAT,mfsDOUBLE};
	size_t rcount;

	/* Check whether format is OK */

	if (size == lengths[type])
	{
		rcount = fread(buffer,size,count,mfile->fptr);
                if (mfile->byteswap)
		{
			if (type == mfSHORT)    mfSwapBytes(buffer,size*count);
			if (type == mfINT)      mfSwapBytes4(buffer,size*count);
			if (type == mfFLOAT)    mfSwapBytes4(buffer,size*count);
			if (type == mfLONGLONG) mfSwapBytes8(buffer,size*count);
			if (type == mfDOUBLE)   mfSwapBytes8(buffer,size*count);
		} 
 	}
	else
	{
		printf("Incompatible formats: variable size does not correspond with size of variable in file\n");
	}
	return(rcount);
}

size_t	mfwrite(void *buffer,size_t size,size_t count,MFILE *mfile, int type)
{
	size_t lengths[] = {mfsNONE,mfsCHAR,mfsSHORT,mfsINT,mfsLONGLONG,mfsFLOAT,mfsDOUBLE};
	size_t rcount;

	/* Check whether format is OK */

	if (size == lengths[type])
	{
                if (mfile->byteswap)
		{
			if (type == mfSHORT)    mfSwapBytes(buffer,size*count);
			if (type == mfINT)      mfSwapBytes4(buffer,size*count);
			if (type == mfFLOAT)    mfSwapBytes4(buffer,size*count);
			if (type == mfLONGLONG) mfSwapBytes8(buffer,size*count);
			if (type == mfDOUBLE)   mfSwapBytes8(buffer,size*count);
		} 
                
                rcount = fwrite(buffer,size,count,mfile->fptr);

                if (mfile->byteswap)
		{
			if (type == mfSHORT)    mfSwapBytes(buffer,size*count);
			if (type == mfINT)      mfSwapBytes4(buffer,size*count);
			if (type == mfFLOAT)    mfSwapBytes4(buffer,size*count);
			if (type == mfLONGLONG) mfSwapBytes8(buffer,size*count);
			if (type == mfDOUBLE)   mfSwapBytes8(buffer,size*count);
		} 
	
	}
	else
	{
		printf("Incompatible formats: variable size does not correspond with size of variable in file\n");
	}
	return(rcount);
}

int	mfseek(MFILE *mfile,long offset,int origin)
{
	return(fseek(mfile->fptr,offset,origin));
}	


void   *mfmemread(void *buffer,size_t size,size_t count,void *datablock,int type)
{
	/* Lengths of the different objects. */
	/* If the lengths in bytes do not correspond with the machines sizeof() operator */
	/* a warning is issued */
	/* FUTURE: implementations may correct this by padding extra zeros in between */

	size_t lengths[] = {mfsNONE,mfsCHAR,mfsSHORT,mfsINT,mfsLONGLONG,mfsFLOAT,mfsDOUBLE};
	size_t rcount;
        int    byteswap;
	char   *newptr;
 
        byteswap = mfByteSwap();

	/* Check whether format is OK */

	if (size == lengths[type])
	{
		memcpy(buffer,datablock,(long)size*count);
                rcount = size*count;
                if (byteswap)
		{
			if (type == mfSHORT)    mfSwapBytes(buffer,size*count);
			if (type == mfINT)      mfSwapBytes4(buffer,size*count);
			if (type == mfFLOAT)    mfSwapBytes4(buffer,size*count);
			if (type == mfLONGLONG) mfSwapBytes8(buffer,size*count);
			if (type == mfDOUBLE)   mfSwapBytes8(buffer,size*count);
		} 
                
	}
	else
	{	/* IN FUTURE MAY ADD CONVERSION IF I KNOW HOW TO DO THEM */
		printf("Incompatible formats: variable size does not correspond with size of variable in memory block\n");
	}
        
	newptr = (char *) datablock;
        newptr +=  rcount;
	return((void *)newptr);
}

void	*mfmemwrite(void *buffer,size_t size,size_t count,void *datablock, int type)
{
	size_t lengths[] = {mfsNONE,mfsCHAR,mfsSHORT,mfsINT,mfsLONGLONG,mfsFLOAT,mfsDOUBLE};
	size_t rcount;
        int byteswap;
	char *newptr;

	/* Check whether format is OK */

        byteswap = mfByteSwap();

	if (size == lengths[type])
	{
		memcpy(datablock,buffer,size*count);
                rcount = size*count;
                if (byteswap)
		{
			if (type == mfSHORT)    mfSwapBytes(datablock,size*count);
			if (type == mfINT)      mfSwapBytes4(datablock,size*count);
			if (type == mfFLOAT)    mfSwapBytes4(datablock,size*count);
			if (type == mfLONGLONG) mfSwapBytes8(datablock,size*count);
			if (type == mfDOUBLE)   mfSwapBytes8(datablock,size*count);
		} 
                
	}
	else
	{
		printf("Incompatible formats: variable size does not correspond with size of variable in memory block\n");
	}

	newptr = (char *)datablock;
        newptr += rcount;
	return(newptr);
}

#endif


