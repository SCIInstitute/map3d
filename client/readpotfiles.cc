/*** 
Filename: readpotfiles.c
Author: Rob MacLeod

  Last update: Sat Dec 31 14:32:16 MST 1994
  - error reading in data when a surface is "skipped", ie., has no data
  Fixed with a new function that converts a point number to a pottential
  number to address the right location in memory.
  Last update: Tue Jun 21 16:58:29 MDT 1994
  - created to read in a series of .pot files.
  
***/
/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "readpotfiles.h"
#include "map3d.h"
#include "Surf_Data.h"

/****************** Externals *****************************/

#ifdef _WIN32
#pragma warning(disable:4172 4514)  /* quiet visual c++ */
#endif

extern Map3d_Info map3d_info;

/****************** Prototypes ****************************/
void ReadOnePotFile(FILE * luin_p, float potscale, Surf_Data * onesurfdata, long lframenum, long *onechannels);

/***************** The Code *******************************/

/*======================================================================*/

Surf_Data *ReadPotFiles(Surf_Input * sinputlist, Surf_Data * surfdata,
                        Map3d_Geom * map3dgeom, long numsurfsread, long insurfnum)
{
/*** Read a set of pot files for a single entry surface in map3d.
The idea is that for each entry surface there can be more than
one display surface, which is all set up so that a geometry
file can have more then one surface in it, i.e. a single entry
surface has info for a single .geom file, which can have more
than one surface in it.  Then we get more display surfaces than
input or entry surfaces.

  Here we read one pot file, or one set of pot files, for each
  display surface.  This is a weird way to use pot files
  but is legal if we either use the same set of pots over and over 
  again, or have channels files that select portions of the pot file
  data.
  
    Input:
    sinputlist   	the structure of information for this input
                        or entry surface. We get lots of stuff from here.
    surfdata	    	the surfdata array for all the surfaces
    numsurfsread 	the number of display surfaces that were read for 
                        this entry surface.  If this is more than 1,
                        we have to re-read the same pot files
    insurfnum	        number of display surfaces at which to start
                        loading new potentials.
    
  ***/
  long surfcount;
  long displaysurfnum; /*** Surface into which we place data. ***/
  long framenum, numpotsurfs;
  long numframes, numleads;
  long filenum, filenum1, filenum2, filestep;
  long *channels_p;  /*** Pointer to one surface worth of channels ***/
  char filename[200], basefilename[200];
  float potscale;
  FILE *luin_p;
 /********************************************************************/

  if (map3d_info.reportlevel > 1)
    fprintf(stderr, "In ReadPotFiles\n");

  filenum1 = sinputlist->ts_start;
  filenum2 = sinputlist->ts_end;
  filestep = sinputlist->ts_step;


  if (filenum2 < filenum1)
    filenum2 = filenum1;
  if (filestep < 0)
    filestep = 1;
  strcpy(basefilename, sinputlist->potfilenames[sinputlist->potfileindex]);
  StripExtension(basefilename);
  if (filenum2 > filenum1)
    basefilename[strlen(basefilename) - 3] = 0;
  printf(" basefilename is %s from %s\n", basefilename, sinputlist->potfilenames[sinputlist->potfileindex]);
  potscale = sinputlist->potscale;
  numpotsurfs = 0;

 /*** Loop through all the surfaces read for this entry surface. 
  ***/

  for (surfcount = 0; surfcount < numsurfsread; surfcount++) {

    displaysurfnum = insurfnum + surfcount;
    if ((displaysurfnum + 1) > map3d_info.numsurfs) {
      fprintf(stderr, "*** ReadPotFiles: we have pots for a surface"
              " with no geometry\n" " Last surface is %ld and we want %ld\n", map3d_info.numsurfs, displaysurfnum + 1);
      return (NULL);
    }
    numleads = map3dgeom[displaysurfnum].numpts;
    channels_p = map3dgeom[displaysurfnum].channels;

 /*** Set up the sequence of files that we want to read in for this
	     surface.
 ***/
    framenum = 0;
    if (map3d_info.reportlevel > 1)
      fprintf(stderr, " Loop of pot files for display surface %ld "
              "from %ld to %ld step %ld\n", displaysurfnum + 1, filenum1 + 1, filenum2 + 1, filestep);
    for (filenum = filenum1; filenum <= filenum2; filenum++) {

 /*** Make the .pot filename and open the file. ***/

      if (filenum2 > filenum1) {
        sprintf(filename, "%s%-3.3ld", basefilename, filenum + 1);
      }
      else {
        strcpy(filename, basefilename);
      }
      strcat(filename, ".pot");

      if ((luin_p = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "*** ReadPotFile: Pot file %s not " "found ***\n", filename);
        return surfdata;

 /*** Now go read the file into this surface's arrays.  ***/

      }
      else {

 /*** Allocate memory for the new surface of data in the surfdata array. ***/

        if (filenum == filenum1) {
          numframes = (long)((filenum2 - filenum1) + 1);
          if (numframes > map3d_info.maxnumframes)
            map3d_info.maxnumframes = numframes;
          if ((surfdata = Surf_Data::AddASurfData(surfdata, displaysurfnum, numframes, numleads)) == NULL)
            return (NULL);
          surfdata->ts_end = filenum2;
          surfdata->ts_start = filenum1;
          surfdata->ts_step = filestep;
          surfdata->numframes = (filenum2 - filenum1) + 1;

        }

        if (map3d_info.reportlevel > 1)
          fprintf(stderr, " Reading potentials from \n" "file: %s\n", filename);
        ReadOnePotFile(luin_p, potscale, &surfdata[displaysurfnum], framenum, channels_p);
        fclose(luin_p);
      }
      framenum++;
    }
    numpotsurfs++;
    if (strlen(sinputlist->datapathname) > 1)
      strcpy(surfdata[displaysurfnum].filepath, sinputlist->datapathname);
    strcpy(surfdata[displaysurfnum].potfilename, sinputlist->potfilenames[sinputlist->potfileindex]);
  }
  return surfdata;
}

/*================================================================*/
void ReadOnePotFile(FILE * luin_p, float potscale, Surf_Data * onesurfdata, long lframenum, long *onechannels)
 /*** Read the potentials values from the already open file 
      Dynamic allocation version.
                    
    Input:
      potscale	      float value multiplied by each value in the file
                      as it is read in.
      onesurfdata     one structure to load up with data
      lsurfnum        local surface number
      onechannels     pointer to channels for this surface.
                      
 ***/
{
  long i;
  long npotread, ninpots, index;
  long numleads, nfields;
  float *potbuff;   /*** Buffer to hold a set of potentials ***/
 /******************************************************************/

  numleads = onesurfdata->numleads;

    /*** Set up the buffer used to store one set of potentials. ***/

  ninpots = GetFileLength(luin_p);
  rewind(luin_p);

  if ((potbuff = (float *)calloc((size_t) ninpots, sizeof(float))) == NULL) {
    fprintf(stderr, "*** In ReadOnePotFile error getting memory"
            " for pot data\n" "    in frame #%ld\n", lframenum + 1);
    fprintf(stderr, "   num_values = %ld\n", ninpots);
    exit(ERR_MEM);
  }

 /*** Read the data finally, first into a buffer.  ***/

  npotread = 0;
  for (i = 0; i < ninpots; i++) {
    if ((nfields = fscanf(luin_p, "%f\n", &potbuff[i])) != 1) {
      fprintf(stderr, "*** In ReadOnePotFile problems because the file\n"
              "     yielded wrong number of fields or less"
              " data than expected\n"
              "     We expect %ld values but are only at %ld\n"
              "     and have a line with %ld fields\n", ninpots, npotread + 1, nfields);
      exit(ERR_FILE);
    }
    npotread++;
  }

 /*** Now transfer the data to the final location, using the channels
  buffer to redirect the effort. ***/

  for (i = 0; i < numleads; i++) {
    index = onechannels[i];
 /*** Assume we have a bad channel here and so put zeros in the lead ***/
    if (index < 0) {
      onesurfdata->potvals[lframenum][i] = 0;

 /*** Check if we have valid index pointer. ***/

    }
    else if (index > ninpots - 1) {
      fprintf(stderr, "*** In ReadOnePotFile  we have an invalid"
              " redirection\n"
              "    The value of the channel index is %ld but"
              " we have %ld values\n"
              "    The index must always be less than the number" " of values in the file\n", index, ninpots);
      exit(ERR_MEM);
    }
    else {
      onesurfdata->potvals[lframenum][i] = potbuff[index] * potscale;
    }
  }

    /*** Free up the space needed for the buffer. ***/

  free((float *)potbuff);
  potbuff = NULL;

  /*** Since pot files don't have units, set a 0 in theunits. ***/

  onesurfdata->units = 0;

  if (map3d_info.reportlevel > 3) {
    fprintf(stderr, " Potentials for frame %ld read\n", lframenum);
    fprintf(stderr, " First values are\n");
    for (i = 0; i < cjmin(ninpots, 10); i++)
      fprintf(stderr, " %ld  %f\n", i, onesurfdata->potvals[lframenum][i]);
  }
}
