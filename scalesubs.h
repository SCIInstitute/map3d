/* scalesubs.h */

#ifndef SCALESUBS_H
#define SCALESUBS_H


// it is imperative that these defines are 0-n, as they are array indices
#define LOCAL_SCALE        0 /*** Used for scale_scope ***/
#define GLOBAL_SURFACE     1 /*** Scaling over all frames in a surface. ***/
#define GLOBAL_FRAME       2 /*** Scaling over all surfaces in a frame. ***/
#define GLOBAL_GLOBAL      3 /*** Scaling over all surfaces and frames. ***/
#define GROUP_FRAME        4 /*** Groups scaling ****/
#define GROUP_GLOBAL       5
#define SLAVE_FRAME        6  /*** Slave scaling ****/
#define SLAVE_GLOBAL       7
#define NUM_RANGES         8

// it is imperative that these defines are 0-n, as they are array indices
#define LINEAR     0 /*** Used for scale_model ***/
#define EXP        1 /*** Used for scale_model ***/
#define LOG        2 /*** Used for scale_model ***/
#define LAB7       3 /*** Used for scale_model ***/
#define LAB13      4 /*** Log/13 levels scale model ***/
#define NUM_FUNC   5

// it is imperative that these defines are 0-n, as they are array indices
#define SYMMETRIC  0 /*** For scale_mapping, range +/- MAX(ABS(Max+,Max-) ***/
#define SEPARATE   1 /*** For scale_mapping, range Max- --> 0 & 0 --> Max+ ***/
#define TRUE_MAP   2 /*** For scale_mapping, range Max- --> Max+ ***/
#define MID_MAP    3 /*** For scale_mapping, range Min- --> Midpoint & Midpoint --> Max+ ***/
#define NUM_MAPS   4

class ColorMap;

float *GetContVals(float minval, float maxval, long nconts, float userspacing, long *numconts);
float *AllocContvals(long numconts);
float *ContValsUser(long mapping, float minval, float maxval, float absmax, float userspacing, long *numconts);
void ContValsLinear(float minval, float maxval, const long nconts, float *contvals);
void ContValsExp(float minval, float maxval, const long nconts, float *contvals);
void ContValsLog(float minval, float maxval, const long nconts, float *contvals);
float *ContValsLab(long mapping, float minval, float maxval,
                   const long nconts, float absmax, long nlabconts, float *labvals, long *numconts);
void getContColor(float potval, float min, float max, ColorMap * map, unsigned char *color, bool invert);
float getContNormalizedValue(float potval, float min, float max, bool invert);

//have these functions get called so the scaling dialog and menu can call them here
//  if we have the dialog call the menu accessor, and the menu try update the 
//  dialog, we end up with a big recursive crash

// (although, you could just put surround the code in the menu callback with menulock)
void setScalingRange(int range);
void setScalingFunction(int func);
void setScalingMapping(int map);

#endif
