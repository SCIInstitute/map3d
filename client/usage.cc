/* usage.c */

#include <stdio.h>
#include "usage.h"

const char *usage =
  "map3d [FILENAME]\n"
  "map3d [OPTIONS]\n\n"
  "FILENAME:\n"
  " the name of a file containing geometry and/or data.\n\n"
  "OPTIONS:\n\n"
  "  Global:\n"
  "    -bw border_width title_bar_height (to override the default)\n"
  "    -c  colour (default colour for all meshes)\n"
  "    -fg  colour (default text colour for all windows)\n"
  "    -bg  colour (default background colour for all windows)\n"
  "    -bgi <image file> (provides a background image for all Geometry windows)\n"
  "    -bgp xlow ylow zlow xhigh yhigh zhigh\n"
  "         coordinates for bg image in geometry space (not required with -bgi)\n"
  "    -df filename (for default settings file)\n"
  "    -if filename (to set output image basefilename)\n"
  "    -iv (initialize video equipment)\n"
  "    -nv (to *not* check validity of geometry)\n"
  "    -b  (borderless mode - all windows inside of one master window\n"
  "    -nw (for new window for each surface) \n"
  "    -rl level (set report level)\n"
  "    -ss (to have all surfaces with same geometry scale)\n"
  "    -slw 0 (to hide all the legend windows for all surfaces)\n"
  "    -v  (echo the version number)\n"
  "    -vm (for video mode)\n"
  "    -vw xmin ymin (location of video window)\n\n"
  "  Per Surface:\n"
  "    -ac (leave geometry in absolute coordinates)\n"
  "    -al xmin xmax ymin ymax (to set colormap window location)\n"
  "    -as xmin xmax ymin ymax (to set surface window location)\n"
  "    -at xmin xmax ymin ymax (to set scalar window location)\n\n"
  "    -c  colour (for mesh)\n"
  "    -fg  colour (for window text colour)\n"
  "    -bg  colour (for window background colour)\n"
  "    -cf containerfile filename (must also select a data file)\n"
  "    -cg colour (for vectors)\n"
  "    -ch channels_filename (links nodes to channels in data file)\n"
  "    -cl channellinks_flename (links channels to leadlinks)\n"
  "    -cs scale_step_size (user set contour spacing)\n"
  "    -dp datafilepath  (pathname to the pak-data files)\n"
  "    -ds (make this the dominant surface)\n"
  "    -f  geomfilename \n"
  "    -ff fidfilename (.fid file to be read)\n"
  "    -fn fidseriesnum (number of the fid series to take)\n"
  "    -gn num (share scaling with all surfaces in group num (1-n))\n"
  "    -gp geomfilepath  (pathname to the geometry files)\n"
  "    -i  increment (increment between file numbers)\n"
  "    -lh (to set colormap window's orientation to be horizontal)\n"
  "    -ll leadlinks_filename (links electrode numbers to leads)\n"
  "    -lm landmarkfile (to read landmark file (eg. coronaries) in)\n"
  "    -p  pot/grad-filename (basefilename) \n"
  "    -ph maximum_data_value (user set potential maximum)\n"
  "    -pl minimum_data_value (user set potential minimum)\n"
  "    -ps scaleval (scale value applied to all data)\n"
  "    -s  num1 num2 (first and last file numbers)\n"
  "    -sl surfnum (to slave of lock scaling to another surface)\n"
  "    -slw 0 (to hide the legend window for this surface\n"
  "    -t  trace_lead_number (for scalar plot)\n\n"
  "  Misc:\n"
  "    -h | --help | -?: This help message.\n\n"
  "Note: Multiple surfaces, and multiple scalars per surface, are permitted\n";


void PrintUsageStatement(bool versiononly)
{
  printf("\n   Map3D\n");
  printf("   Version " VERSION "\n");
  printf("   %s ", __DATE__ "\n");
  printf("   %s ", __TIME__ "\n");
#ifdef REVISION
  printf("  Build   %d \n", REVISION);
#endif
  if (!versiononly) {
    printf("\nUSAGE:\n%s", usage);
    printf("\nSee also http://software.sci.utah.edu/map3d-6.2_docs/index.html\n\n");
  }
}
