/* map3d.cxx */

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif

#include "ParseCommandLineOptions.h"
#include "ProcessCommandLineOptions.h"
#include "colormaps.h"
#include "map3d-struct.h"
#include "regressiontest.h"
#include "savestate.h"

#include <QApplication>

Map3d_Info map3d_info;


const char *units_strings[5] = { "mV", "uV", "ms", "V", "mVms" };

void dump()
{
  FILE* file = fopen("geom/torso/daltorso.channels", "w");
  fprintf(file, "353 leads\n");
  for (int i = 0; i < 353; i++)
    fprintf(file, "%d %d\n", i+1, (i < 50 || (i % 10) == 0 || i > 200) ? -1 : i+1);
  fclose(file);
}

int main(int argc, char **argv)
{
  //dump();
  Global_Input globalInput;
  map3d_info.gi = &globalInput;

  // parse the arguments - it is befroe initting the GUI so it can spit out a usage string without having a display set
  // read .map3drc file and prepend the state to the front of the arguments.  If we do this we will
  // have to delete argv, as we will have dynamically allocated it.
  int old_argc = argc;
  char** old_argv = argv;
  bool loadedState = false; // FIX ReadMap3drc(argc, argv);
  if (!ParseCommandLineOptions(argc, argv, globalInput))
    exit(1);


  QApplication app(argc, argv);

  // process the command line options - build windows, setup meshes, etc.
  if (!ProcessCommandLineOptions(globalInput))
    exit(1);

  //printf("GTK Version: %d.%d.%d\n", gtk_major_version, gtk_minor_version, gtk_micro_version);

  /* start the event system */
#ifdef REGRESSION_TEST
  MenuTester* menutest = new MenuTester();
  menutest->regressionTest();
#endif
  int retval = app.exec();

  // delete the new argv if it was created in readMap3drc
  if (loadedState) {
    for (int i = 0; i < argc; i++)
      delete argv[i];
    delete argv;
  }
  return retval;
}
