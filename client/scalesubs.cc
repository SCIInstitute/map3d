/*** A function to compute contour values in one of several different ways.
File: scalesubs.cc
Author: Rob MacLeod
Last update: Mon Jul 15 09:01:25 MDT 1991

  Input:
  type	    type of contouring to be performed:
  1 = linear
  2 = exponential
  3 = logarithmic
  4 = "lab standard", 7-step log coverage of 1 decade
  5 = "lab 13 standard", 13-step log coverage of 1 decade
  mapping     the way the data are scaled mapped to contours
  1 = symmetric, max and min are symmetric about 0
  2 = separate positive and negative mapping
  3 = true mapping, ranges from real min to max
  maxval	    maximuim of the range to be spanned by contours
  minval	    minimum to be spanned by contours
  In/Output:
  nconts	    number of contours desired, is updated to actual value
  when lab type contours are used. If nconts is an odd
  number, then for SYMMETRIC and SEPARATE mapping, a contour
  at 0.0 is returned. In the this manner, for lab scaling, 
  nconts determines only whether the zero contour is 
  returned.
  Output:
  contvals    array of contour values (FLOAT array of at least
  'nlabconts'*2+1 values, 'nlabconts' set below.
  Order of values is always smallest (or most negative)
  to largest (most positive).
  Returns:
  nconts	    Actual number of contours generated is returned
  
    Note: For an even number of contours and symmetric mapping, they will be
    symmetric about 0 if there are both positive and negative values. If the number
    of contours is odd, then the middle one will be set to 0. 
    
      If the mapping is separate, the same number of contours will be used to span
      both the negative and postive values. The number used will be half the total
      specified, with the same type of caveats as above with regards to odd and even
      number of contours. If there are no data values on one or the other side of
      zero, the contours for that regions are returned as all 0.
      
        If the mapping is true, then the actual data range specified by min and max
        will be spanned by the contours, irregardless of whether there are odd or even
        numbers of contours, or positive and/or negative data.
        
          Last update: Wed Feb  6 19:04:50 MST 1991
          Last update: Tue Jul  9 21:04:11 MDT 1991
          - added 'mapping' to variable list to make some more variations
          on the scaling story.
          Last update: Mon Jul 15 09:02:03 MDT 1991 
          - complete rework of the routine.
          
***/

#ifdef _WIN32
#pragma warning(disable:4172 4514)  /* quiet visual c++ */
#endif

#include <stdio.h>
#include <math.h>
#include "scalesubs.h"

#include "colormaps.h"
#include "map3d-struct.h"
#include "WindowManager.h"
#include "GeomWindow.h"
#include "eventdata.h"
extern Map3d_Info map3d_info;

static float labvals7[] = { 1.0f, 1.5f, 2.2f, 3.3f, 4.7f, 6.8f, 10.f };
static float labvals13[] = { 1.0f, 1.2f, 1.5f, 1.8f, 2.2f, 2.7f, 3.3f, 3.9f, 4.7f,
  5.6f, 6.8f, 8.2f, 10.f
};
static long nlabconts7 = 7;     /*** Number of lab contours in 'labvals' ***/
static long nlabconts13 = 13;
static double yexpmax = 10.;   /*** Sets range and spacing of exp contours ***/
//static double maxminfac = 100.; /*** Sets range of contours in log ***/
static double adjust = 1.0001;  /*** Fine tuning in lab contours ***/

#ifndef MAX
#  define MAX(x,y) ((x>y)?x:y)
#endif


float *GetContVals(float minval, float maxval, long nconts, float userspacing, long *numconts)
{
  long reportlevel = map3d_info.reportlevel;
  long type = map3d_info.scale_model;
  long mapping = map3d_info.scale_mapping;

  float *contvals = NULL;
  float absmax;
  float amin, amax;
  long nlabconts = (type == LAB13 ? nlabconts13 : nlabconts7);
  float *labvals = (type == LAB13 ? &labvals13[0] : &labvals7[0]);
  int i;
  /*****************************************************************/

  void (*funcptr) (float, float, const long, float *) = NULL;

  if (reportlevel > 2) {
    fprintf(stderr, "In GetContvals\n");
    fprintf(stderr, " type = %ld, mapping = %ld\n", type, mapping);
  }

  /*** First set up a couple of values from the data which are used 
  throughout the routine. ***/

  /*** Set the maximum absolute extrema for use in SYMMETRIC mapping ***/

  amax = (float)fabs(maxval);
  amin = (float)fabs(minval);
  if (amax >= amin) {
    absmax = amax;
    //    absmin = amin;
  }
  else {
    absmax = amin;
    //    absmin = amax;
  }

  if (reportlevel > 2) {
    fprintf(stderr, "Made it to getcontvals with \n");
    fprintf(stderr, "type =  %ld maxval = %f minval = %f nconts = %ld\n", type, maxval, minval, nconts);
    fprintf(stderr, "user contour spacing set to %f\n", userspacing);
  }

  /*** Now find the contour levels for each of four different models:
  linear, exponential log, or lab standard values. Within each block
  based on type, further differentiate among mapping types: SYMMETRIC, 
  SEPARATE, and TRUE_MAP ***/

  if (type == LINEAR) {
    funcptr = ContValsLinear;
  }
  else if (type == EXP) {
    funcptr = ContValsExp;
  }
  else if (type == LOG) {
    funcptr = ContValsLog;

    /*** And the standard 7/13 setep Log scaling ***/

  }
  else if (type == LAB7 || type == LAB13) {
    contvals = ContValsLab(mapping, minval, maxval, nconts, absmax, nlabconts, labvals, numconts);

      /*** If there was no valid entry for type, then put zeros in the contours
    and print an error message. ***/

  }
  else {
    *numconts = 0;
    fprintf(stderr, "\nError since the type parameter was set to " "%ld \n" "Contours all set to 0.0\n", type);
  }

  if (funcptr) {
    //increment # of contours if there are an even number on symmetric or separate
    if (mapping != TRUE_MAP) {
      if ((nconts % 2) == 0)
        nconts++;
    }
    *numconts = nconts;
    contvals = AllocContvals(nconts);

    float *halfway=0;
    if ((minval < 0 && maxval > 0) || mapping == SYMMETRIC || mapping == MID_MAP)
      if (nconts >= 3)
        halfway = &contvals[nconts / 2 + 1];
      else
        halfway = contvals;
    else if (minval >= 0 && mapping == SEPARATE)
      halfway = contvals;

    if (mapping == TRUE_MAP)
      funcptr(minval, maxval, nconts, contvals);
    else if (mapping == SEPARATE) {
      if (minval < 0) {
        funcptr(minval, 0, nconts / 2, contvals); //fill up from halfway and move to other half
        for (int i = 0, j = nconts/2-1; i <= j; i++, j--) {
          // switch around so scaling happens from 0 to min instead of min to 0
          float temp = minval - contvals[j];
          contvals[j] = minval - contvals[i];
          contvals[i] = temp;
        }
      }
      if (maxval > 0)
        funcptr(0, maxval, nconts / 2, halfway);
      contvals[nconts / 2] = 0;
    }
    else if (mapping == MID_MAP) {
      float midpoint = (maxval + minval)/2;
      funcptr(minval, midpoint, nconts / 2, contvals);
        for (int i = 0, j = nconts/2-1; i <= j; i++, j--) {
          // switch around so scaling happens from mid to min instead of min to mid
          float temp = midpoint - contvals[j];
          contvals[j] = minval + (midpoint - contvals[i]);
          contvals[i] = minval + temp;
        }
      contvals[nconts/2] = midpoint;
      funcptr(midpoint, maxval, nconts/2, halfway);

    }
    else if (mapping == SYMMETRIC) {
      funcptr(0, absmax, nconts / 2, halfway);
      for (i = 0; i < nconts / 2; i++)
        contvals[nconts / 2 - 1 - i] = -contvals[nconts / 2 + i + 1];
      contvals[nconts / 2] = 0;
    }
  }


  if (reportlevel > 2) {
    fprintf(stderr, " After setting up new contours, we have %ld of them\n", *numconts);
  }
  /*** Report the error if we have one. ***/


  if (contvals == NULL) {
    ReportError("GetContVals", "contvals is NULL", 0, "");
    return (NULL);
  }
  else {
    return (contvals);
  }
}

/*======================================================================*/

float *AllocContvals(long numconts)
{
  /*** Return an float array for the contours ***/

  float *contvals = NULL;
  /**********************************************************************/
  if (numconts < 1) {
    ReportError("AllocContvals", "numconts < 1", 0, "");
    return (NULL);
  }
  if ((contvals = (float *)calloc((size_t) numconts, sizeof(float))) == NULL) {
    ReportError("AllocContvals", "getting contval memory", 0, "");
    return (NULL);
  }
  return (contvals);
}


//note - can be called multiple times for symmetric/separate scaling
void ContValsLinear(float minval, float maxval, const long nconts, float *contvals)
{

  /*** Figure out the contours for the LINEAR option. ***/

  long i;
  //  long npconts;
  double contstep;
  contstep = (maxval - minval) / (float)(nconts + 1);
  contvals[0] = (float)(minval + contstep);
  for (i = 1; i < nconts; i++) {
    contvals[i] = (float)(contvals[i - 1] + contstep);
  }
}

/*=======================================================================*/

//note - can be called multiple times for symmetric/separate scaling
void ContValsExp(float minval, float maxval, const long nconts, float *contvals)
{

/*** Figure out the contours for the EXPONENTIAL option. 
This mode involves
mapping the range of data values from 'min' to 'max' onto the range of 
1 to 'yexpmax' using an exponent. From this mapping can be derived 
the spacing for contours, and then the absolute values of those contours.
  ***/

  long i;
  double contstep, beta;
  beta = log(yexpmax) / (maxval - minval);
  contstep = (yexpmax - 1) / (nconts + 1);
  for (i = 0; i < nconts; i++) {
    contvals[i] = (float)(minval + log(1 + (i + 1) * contstep) / beta);
  }
}

/*=======================================================================*/

//note - can be called multiple times for symmetric/separate scaling
void ContValsLog(float minval, float maxval, const long nconts, float *contvals)
{

/*** Figure out the contours for the LOG option. 
There are some assumptions which have
to be made here due to the nature of log scaling, the most important
of which is the range of maximum to minimum values which are to be
displayed, i.e., given a contour value. This ratio of maximum to
minimum is called 'maxminfac' and at the moment, it is fixed in this
routine.  Perhaps in the furure it will be a parameter but time will
tell. 
  ***/

#if 0
  long i;
  double contstep, alpha;
  alpha = (maxminfac) / log10(maxval - minval);
  contstep = (maxminfac) / (nconts + 1);
  for (i = 0; i < nconts; i++) {
    contvals[i] = (float)(minval - 1 + pow(10.0, (1 + (i + 1) * contstep) / alpha));
  }

#else
  double exp = yexpmax;
  long i;
  double contstep, beta;
  beta = log(exp) / (maxval - minval);
  contstep = (exp - 1) / (nconts + 1);
  for (i = 0; i < nconts; i++) {
    contvals[nconts - 1 - i] = (float)(maxval - log(1 + (i + 1) * contstep) / beta);
  }
#endif
}

/*=======================================================================*/

float *ContValsLab(long mapping, float minval, float maxval, const long /*nconts */ , float absmax,
                   long nlabconts, float *labvals, long *numconts)
{

/*** 
Now the case of a lab standard contour layout: seven (or thirteen)
contours positive and negative from a set of set values.  
Note the "adjustment" factor which is a number just slightly different 
from one. It is multiplied by the lab contour values to try and avoid 
the problem of data values being exactly equal to data values which 
have been rounded to whole number values (often the case for 
map data files).  
  ***/

  long i, k;
  long lcontnum;
  int power;
  float tens;
  float *contvals = NULL;
  /**********************************************************************/
  if (map3d_info.reportlevel > 2) {
    fprintf(stderr, " labvals now set to \n");
    for (i = 0; i < nlabconts; i++) {
      fprintf(stderr, " val %ld is %f units\n", i, labvals[i]);
    }
  }
  /*** Now the number of contours to be used on each side of 0.0 for
  the symmetric and separate case, and a logical depending on whether
  the number of contours requested is odd or even. 
  Adjust the number of contours based on whether an odd or even
  number was passed to the routine. 
  ***/


  /*** First the case of SYMMETRIC mapping. Find the nearest decade to
  the absolute maximum.  Then use it to set the power of ten with 
  which we multiply the standard contour values. ***/
  if (mapping == SYMMETRIC || (minval < 0 && maxval > 0))
    *numconts = 2 * nlabconts + 1;
  else
    *numconts = nlabconts;
  contvals = AllocContvals(*numconts);


  if (mapping == SYMMETRIC) {
    power = 6;
    tens = (float)pow(10., power);
    while (tens >= absmax) {
      power--;
      tens = (float)pow(10., power);
    }

    /*** Now home in on the nearest lab value just below the maximum. ***/

    lcontnum = nlabconts - 1;
    while (labvals[lcontnum] * tens >= absmax) {
      lcontnum--;
    }

    /***	    
    printf( "absmax = %f tens = %f power = %ld\n", 
    absmax, tens, power);
    printf( "nconts = %i npcont = %i\n", nconts, npconts);
    ***/

    /*** Now we have the outermost contour, so run up through the
    rest and set them up. Note the adjust factor so as not to have too round
    values (necessary for some contouring schemes. ***/

    for (k = 0; k < nlabconts; k++) {
      contvals[k] = (float)(-labvals[lcontnum] * tens * adjust);
      contvals[*numconts - 1 - k] = -contvals[k];
      lcontnum--;
      if (lcontnum < 0) {
        lcontnum = nlabconts - 2;
        power--;
        tens = (float)pow(10., power);
      }
    }
  }

  /*** Now the case of SEPARATE mapping. Use the same basic strategy
  as above, but treat positive and negative sides separately. There
  will be 'nlabconts' contours on each side of zero but the values
  will be different if the extrema have different magnitudes. 
  We use the same setup for TRUE_MAP mapping fo the moment.
  ***/

  if (mapping != SYMMETRIC) {
    if (minval < 0.0) {
      power = 6;
      tens = (float)(-pow(10., power));
      while (tens <= minval) {
        power--;
        tens = (float)(-pow(10., power));
      }

      /*** Now home in on the nearest lab value just above the minimum. ***/

      lcontnum = nlabconts - 1;
      while (labvals[lcontnum] * tens <= minval) {
        lcontnum--;
      }

      /***	
      printf( "minval = %f tens = %f lcontnum = %i\n", 
      minval, tens, power, lcontnum);
      printf( "nconts = %i npcont = %i\n", nconts, npconts);
      ***/

      /*** Now we have the first contour, so run up through the
      rest of the negative values and set them up. ***/

      for (k = 0; k < nlabconts; k++) {
        contvals[k] = (float)(labvals[lcontnum] * tens * adjust);
        lcontnum--;
        if (lcontnum < 0) {
          lcontnum = nlabconts - 2;
          power--;
          tens = (float)(-pow(10., power));
        }
      }

      /*** Now the positive side of zero. ***/

      if (maxval > 0) {
        power = 6;
        tens = (float)pow(10., power);
        while (tens >= maxval) {
          power--;
          tens = (float)pow(10., power);
        }

        /*** Now home in on the nearest lab value just below the maximum. ***/

        lcontnum = nlabconts - 1;
        while (labvals[lcontnum] * tens >= maxval) {
          lcontnum--;
        }

        /***
        printf( "maxval = %f tens = %f lcontnum = %i\n", 
        maxval, tens, lcontnum);
        printf( "nconts = %i npcont = %i\n", nconts, npconts);
        ***/

        i = 1;
        for (k = nlabconts; k > 0; k--) {
          contvals[*numconts - i] = (float)(labvals[lcontnum] * tens * adjust);
          i++;
          lcontnum--;
          if (lcontnum < 0) {
            lcontnum = nlabconts - 2;
            power--;
            tens = (float)pow(10., power);
          }
        }
      }
    }
  }
  return (contvals);
}

void getContColor(float potval, float min, float max, ColorMap * cmap, unsigned char *color, bool invert)
{
  int length = cmap->max;
  unsigned char *map = cmap->map;
  int index = (int)(length*getContNormalizedValue(potval, min, max, invert));

  color[0] = map[index*3];
  color[1] = map[index*3 + 1];
  color[2] = map[index*3 + 2];
}

float getContNormalizedValue(float potval, float min, float max, bool invert)
{
  // we want to return a number between 0 and 1
  // we will adjust these for symmetric and separate
  float length = 1;
  float base = 0;
  float retval = 0;

  //cut scale in half if separate or symetric
  //if it is inverted, then you want the other half
  if (map3d_info.scale_mapping == SYMMETRIC)
    if (potval < 0)
      return 1-getContNormalizedValue(-potval, 0, MAX(max, -min), invert);
    else {
      max = MAX(max, -min);
      min = 0;
    }
  else if (map3d_info.scale_mapping == SEPARATE) {
    if (potval < 0)
      return 1-getContNormalizedValue(-potval, 0, -min, invert);
    else
      min = 0;
  }
  else if (map3d_info.scale_mapping == MID_MAP)  {
    float midpoint = (max+min)/2;

    if (potval < midpoint)
      return 1-getContNormalizedValue(midpoint+midpoint-potval,min,max, invert);
    else
      min = midpoint;
  }

  if (map3d_info.scale_mapping != TRUE_MAP) { 
    // we can assert that potval is greater than the midpoint
    length /= 2;
    base = .5;
  }

  if (max - min != 0) {
//  Don't set the color to black if out of range - just clamp it instead -BJW
    if (potval > max)
      potval = max;
    if (potval < min)
      potval = min;

#if 0
    // this section is if we want to scale the colors for an exp/log model
    if (map3d_info.scale_model == EXP)
      // kind of do the inverse of the exp contour model:
      // e^((val-min)*(log(base)/(max-min))) and get a number between 1 and base,
      // map it linearly between 0 and 1 (subtract 1 and divide by (base-1)
      retval = base + length*(exp((potval-min)*(log(yexpmax))/(max-min)) - 1) / (yexpmax-1);
    else if (map3d_info.scale_model == LOG) {
      retval = 1-length*(exp((max-potval)*(log(yexpmax))/(max-min)) - 1) / (yexpmax-1);
    }
    else
#endif
      retval = base + length * (potval - min) / (max - min);

    if (invert) 
      retval = 1 - retval;

  }
  // clamp it just in case
  if (retval < 0)
    retval = 0;
  if (retval > 1)
    retval = 1;
  return retval;
}

void setScalingRange(int range)
{
  map3d_info.scale_scope = range;
  map3d_info.scale_frame_set = 0; // reset the scaling for frame-specific ranges
  Broadcast(MAP3D_UPDATE);
}

void setScalingFunction(int func)
{
  map3d_info.scale_model = func;
  Broadcast(MAP3D_UPDATE);
}

void setScalingMapping(int map)
{
  map3d_info.scale_mapping = map;
  Broadcast(MAP3D_UPDATE);
}

