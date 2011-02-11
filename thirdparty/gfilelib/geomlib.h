#ifndef __GEOM_LIB__
#define __GEOM_LIB__

#if DYNAMIC

#ifdef _WIN32
  #ifdef BUILD_gfile
    #define GFILESHARE __declspec(dllexport)
  #else
    #define GFILESHARE __declspec(dllimport)
  #endif
#else
  #define GFILESHARE
#endif

#else
#define GFILESHARE
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum geomfiletype{ Gfile_geom=1, Gfile_ptsfac=2};

typedef struct /*** Geometry for a single surface. ***/
{
    long surfnum;        /*** Surface number for this surface ***/
    long numpts;         /*** Number of points in the surface geometry. ***/
    long numelements;    /*** Number of elements in the surface. ***/
    long elementsize;    /*** 2, 3, or 4 for segs, tris, and tetras ***/
    float **nodes;         /*** numpts X 3 Node array for whole geometry ***/
    long **elements;     /*** numelements X elementsize Element array  ***/
    long *channels;      /*** Channels for this surface ***/
    char label[100];     /*** A string with the label for this surface ***/
    char filepath[100];  /*** Path name for the geom files ***/
    char filename[100];  /*** Filename of this file ***/
    float xmin,xmax,ymin,ymax,zmin,zmax; /*** extrema of node space ***/
    float xcenter,ycenter,zcenter; /*** middle of node space ***/
    float l2norm;        /*** duh.. ***/
} Surf_Geom;
    
/****************** Prototypes ****************************/
GFILESHARE Surf_Geom *AddASurfGeom( Surf_Geom *surfgeom, 
				    long numpts, long *numsurfs );
GFILESHARE long CheckGeom( Surf_Geom *surfgeom, long numsurfs );
GFILESHARE long CheckOneSurfgeom( Surf_Geom *onesurfgeom );
GFILESHARE long CheckElementDoubles( Surf_Geom *onesurfgeom );
GFILESHARE void OrderEnodes( long *oneelement, long numnodes, long enodes[] );
GFILESHARE long CheckElementPoints( Surf_Geom *onesurfgeom );
GFILESHARE long CheckElementValidity( Surf_Geom *onesurfgeom );
GFILESHARE void FindNodeRange( float **nodes, long numpts, 
			       float maxpoint[3], float minpoint[3] );
GFILESHARE long *GetSurfList( long surfnum, long numsurfs,
				  long *numlistsurfs );
GFILESHARE Surf_Geom *ReadGeomFile (char *infilename, 
				    long startsurfnum, long endsurfnum,
				    long *numsurfsread, long reportlevel);
GFILESHARE long **ReadLinksFile( char *infilename, long *numoflinks );
GFILESHARE long ReadFacFile( Surf_Geom *onesurfgeom, long reportlevel );
GFILESHARE long ReadPtsFile( Surf_Geom *onesurfgeom, long reportlevel );
GFILESHARE long RemoveAnElement( Surf_Geom *onesurfgeom, long elementnum );
GFILESHARE long SetupSurfPoints( Surf_Geom *onesurfgeom, long numpts );
GFILESHARE long WriteChannelsFile( Surf_Geom *surfgeom );
GFILESHARE long WriteElementsFile( Surf_Geom *surfgeom );
GFILESHARE long WriteGeomFile (char *outfilename, Surf_Geom *surfgeom,
		    long startsurfnum, long endsurfnum, 
		    long numsurfs, long reportlevel);
GFILESHARE long WritePtsFile( Surf_Geom *surfgeom );

#ifdef __cplusplus
}
#endif

#endif
