/* FILENAME:  myfile.h
   AUTHOR:    JG STINSTRA
   CONTENTS:  Open a file and take care of byte swapping
   LAST UPDATE: 4 JULY 2003 
*/


#ifndef MYFILE_H 
#define MYFILE_H	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	FILE	*fptr;
	int	byteswap; } MFILE;

#define		mfNONE		0
#define 	mfCHAR		1
#define		mfSHORT		2
#define		mfINT		3
#define		mfINTEGER	3
#define		mfLONG		3
#define		mfLONGLONG	4
#define		mfFLOAT		5
#define		mfDOUBLE	6

#define		mfsNONE		1
#define		mfsCHAR		1
#define		mfsSHORT	2
#define		mfsINT		4
#define		mfsINTEGER	4
#define		mfsLONG		4
#define		mfsLONGLONG	8
#define		mfsFLOAT	4
#define		mfsDOUBLE	8

#define		mfNOBYTESWAP	0
#define		mfBYTESWAP	1

/*
 These functions are for simple byte swapping issues
 mf = myfile

 INTERNAL FUNCTIONS
 mfByteSwap     Detect byte swapping of the machine
 mfSwapBytes    For swapping the byte in a buffer, size_t is the size in bytes

 FILE FUNCTIONS
 mfopen        Detect a swapping machine, if so setup a swapping procedure for input and output
 mfclose       Same but for closing the file

 mfread/mfwrite Functions for writing and reading data to the file, these functions have an additional
                argument that specifies what the type is and hence determine whether any additional conversion
                is needed
 mfseek       Same as fseek only uses the *MFILE instead of *FILE

 mfmemread     After reading a block of data which may or not may be swapped, it can be converted using these functions
               Here byteswap is the state of the byteswapping in the original file and datablock is the data read.
 All functions for conversion need a sizeof(datatype) and the mfDATATYPE, to check lengths and see whether the architcture
 is OK for doing the conversions.  */
 
int	mfByteSwap();
void	mfSwapBytes(void *buffer,size_t size);
void	mfSwapBytes4(void *buffer,size_t size);
void	mfSwapBytes8(void *buffer,size_t size);

MFILE 	*mfopen(char *filename,char *mode);
void	mfclose(MFILE *mptr);

size_t 	mfread(void *buffer,size_t size, size_t count,MFILE *mptr, int type);
size_t	mfwrite(void *buffer,size_t size,size_t count,MFILE *mptr, int type);
int	mfseek(MFILE *mptr,long offset,int origin);

void   *mfmemread(void *buffer,size_t size,size_t count,void *datablock, int type);
void   *mfmemwrite(void *buffer,size_t size,size_t count,void *datablock, int type);


#endif
