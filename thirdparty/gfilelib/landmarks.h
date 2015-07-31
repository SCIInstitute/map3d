#ifndef __LANDMARKS_HEADER__
#define __LANDMARKS_HEADER__
 /*** Defines for the landmarks 
   Last update: Mon Nov 10 15:54:37 1997
     - added two new elements to the the landmark data structure
     segnumval and  segstring.
 ***/

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LM_COR      1 /*** Landmark code for coronary arteries ***/
#define LM_OCCLUS   2 /*** Landmark code for temporary occlusions***/
#define LM_STITCH   3 /*** Landmark code for permanent occlusions***/
#define LM_STIM     4 /*** Landmark code for single stimulation site ***/
#define LM_LEAD     5 /*** Landmark code for recording site ***/
#define LM_PLANE    6 /*** Landmark code for a plane ***/
#define LM_ROD      7 /*** Landmark code for a rod   ***/
#define LM_PACENEEDLE  8 /*** Landmark code for a pacing needle  ***/
#define LM_CATH 9 /*** Landmark code for a catheter ***/
#define LM_FIBER 10 /*** Landmark code for a fiber ***/
#define LM_RECNEEDLE 11 /*** site of recording electrode enty point.  ***/
#define LM_CANNULA 12 /*** site of cannula or infusion needle.  ***/
#define NUMLMARKTYPES 12

 /*** These are defined in landmarks.c ***/
extern char *clmark[];
extern long lmarktypes[];

typedef struct LandMarkSeg
{
    long segnum;       /*** Segment number of this segment ***/
    long type;         /*** Type of landmark (starting at 1) ***/
    long numpts;       /*** Number of points in the segment ***/
    short color[3];    /*** optional color of the segment ***/
                       /*** stored as short, so we can use -1 as unused ***/
    long segnumval; /*** An optional value that reflects some original 
		     number of this segment, like a rod or needle number. ***/
    float **pts;	       /*** Array of points in the segment (nx3)***/
    float *rad;	       /*** Array of radius values for the segment ***/
    char **labels;     /*** Array of optional (40-char) labels 
			    for the segment ***/
} LandMarkSeg;

typedef struct Land_Mark /*** One surfaces worth of landmarks ***/
{
    long  surfnum; /*** Surface number to which this landmark belongs ***/
    long  numsegs; /*** Number of segments in this set  ***/
    float arrowradius; /*** Radius of any arrow heads that get drawn. ***/
    LandMarkSeg *segs; /*** An array of landmark segments. ***/
} Land_Mark;

Land_Mark *DefALandMarkSurf( long surfnum );

LandMarkSeg *AddALandMarkSeg( LandMarkSeg *segs, long *numsegs, 
			    long type, long numpts );
long ReadLandMarkFile (char filename[], Land_Mark *onelandmark, 
		       long reportlevel);
/*long SaveLandMarks( char *outfilename, Land_Mark *landmarks, 
		    long reportlevel ); */
long WriteLandMarks( Land_Mark *landmarks, const long numlmarks, 
		     char *outfilename, long reportlevel );
long WriteOneLandMark( FILE *luout_p, Land_Mark *onelandmark, 
			long reportlevel );

#ifdef __cplusplus
}
#endif

#endif
