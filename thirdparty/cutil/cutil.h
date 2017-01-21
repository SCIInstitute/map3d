#ifndef __CUTIL_HEADER__
#define __CUTIL_HEADER__

/*** 
  File: cutil.h
  Author: Rob MacLeod

  A file of header stuff for C utility routines
 ***/

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#else
#  ifdef _WIN32
//  windows C doesn't have 'inline'
#    define inline __inline
#  endif
#endif

#ifndef FALSE
  #define FALSE 0
#endif
#ifndef TRUE
  #define TRUE ( !FALSE )
#endif
#define SUCCESS 0
#ifndef CUTIL_X
  #define CUTIL_X 0
#endif
#ifndef CUTIL_Y
  #define CUTIL_Y 1
#endif
#ifndef CUTIL_Z
  #define CUTIL_Z 2
#endif
#if !defined(__GL_GL_H__) && !defined(__GRAPHICSIO_H__) && !defined(Boolean)
typedef unsigned char Boolean; 
#endif
#ifndef  __GRAPHICSIO_H__
typedef struct Node
{
    float x;
    float y;
    float z;
}Node, *NodePtr;

typedef struct Vector
{
    float x;
    float y;
    float z;
}Vector, *VectorPtr;
#endif
/***************** Error values ***********************************/
enum ErrorNums { ERR_FILE=-99, ERR_MISC, ERR_ENTRY, ERR_MEM, ERR_VALID };
/**************** Macros ******************************************/
#ifndef VECMAG3
#define VECMAG3(x1,y1,z1) ( sqrt( (double) \
				 ( (x1)*(x1) + (y1)*(y1) + (z1)*(z1) ) ) )
#endif
#ifndef DOTPROD3
#define DOTPROD3(vec1,vec2) ( vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] )
#endif

#ifndef LENGTH3
#define LENGTH3(vec1) sqrt( vec1[0] * vec1[0] + vec1[1] * vec1[1] + vec1[2] * vec1[2] )
#endif

static const double PI=3.14159265;
static inline long MinLong( long a, long b ) 
{ return( ( a  <  b ) ? ( a ) : ( b ) ); }
static inline long MaxLong( long a, long b ) 
{ return( ( a  <  b ) ? ( b ) : ( a ) ); }
static inline float MinFloat( float a, float b ) 
{ return( ( a  <  b ) ? ( a ) : ( b ) ); }
static inline float MaxFloat( float a, float b ) 
{ return( ( a  <  b ) ? ( b ) : ( a ) ); }
static inline float DegtoRadF( float deg ){ return( ((float)PI)/180.f * deg ); }

/***************** Some universal data structures *****************/
typedef struct Point3D {
    float point[3];
} Point3D;
typedef struct DPoint3D {
    double point[3];
} DPoint3D;
/******************************* Prototypes ***********************/
float  **Alloc_fmatrix(long nrh, long nch );
long   **Alloc_lmatrix(long nrh, long nch );
char   **Alloc_cmatrix(long nrh, long nch );
float  *Alloc_farray(long nrows, float *array, char *name );
long   *Alloc_larray(long nrows, long *array, char *name );
short  *Alloc_sarray(long nrows, short *array, char *name );
Boolean   CheckExist( char *filename );
long   CheckExtension( char *filename, const char *extension );
Boolean CheckWrite( char *dirname );
int    ClobberFile (char *filename);
long   ConfirmFile( char *filename, char *prompt );
double Dist3d( float node1[3], float node2[3] );
void   Free_fmatrix( float** m, long nrh );
void   Free_lmatrix( long** m, long nrh );
void   Free_cmatrix( char** m, long nrh );
long   GetFileLength ( FILE *filePtr );
int    GetFilename(char *prompt, char *filename);
float  **Grow_fmatrix(float** m, long nrl, long nrh, long ncl, long nch );
long   **Grow_lmatrix(long** m, long nrl, long nrh, long ncl, long nch );
char   **Grow_cmatrix(char** m, long nrl, long nrh, long ncl, long nch );
 Boolean   IsBlank( char *string );
long   KillFile (char *filename);
char   *Lowercase( char *string );
long   Nint( double val );
double NodeDistance( Node node1, Node node2 );
void	ParseFilename (char *filename, char *dirname, 
		     char *basefilename, char *extension );
double PointDistance( float *node1, float *node2 );
Boolean QComment( char *astring );
char   *ReadComments( FILE *luin, char astring[] );
double ReadDouble( const double defaultval );
void   ReadFilename(const char *prompt, char *filename);
float  ReadFloat( const float defaultval );
int    ReadInt( const int defaultval );
char   *ReadLine(char *instring, FILE *luin_p );
char   *ReadNextLine( FILE *luin );
long   ReadLong( const long defaultval );
char  *ReadString( const char *defaultval );
void   ReportError(const char *function, const char *message, long returnvalue,
            const char *optionalstring);
void   StripExtension (char *filename );
int    Trulen (const char *string);
char   *Uppercase( char *string );

#ifdef __cplusplus
}
#endif

#endif
