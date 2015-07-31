#ifndef __LINK_STRUCTS__
#define __LINK_STRUCTS__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Lead_Link	/*** Link infomation for a node ***/
{
    long leadnum; /*** Number of the lead to which this applies ***/
    long surflink; /*** Surface to which the link belongs ***/
    long nodelink; /*** Node to which the link points ***/
    char *label; /*** Label associated with this link ***/
    long channel; /*** channel number of this link ***/
    long colnum; /*** Column number (where appropriate) ***/
    long rownum; /*** Row number (where appropriate) ***/
    long electrodenum; /*** Electrode number (where appropriate) ***/
    long localchannel; /*** A second channel number for local context ***/
} Lead_Link;

typedef struct Surf_Link
{
    long surfnum; /*** Surface number for these links ***/
    long numlinks; /*** Number of links for this surface ***/
    Lead_Link *links; /*** Links for this surface ***/
} Surf_Link;

/************* Prototypes ******************************************/
Lead_Link *ReadChannelLinksFile( char *infilename, Surf_Geom *onesurfgeom,
                                 long *numclinks);
Lead_Link *ReadGeomLinksFile( char *infilename, long *numglinks );

#ifdef __cplusplus
}
#endif

#endif
