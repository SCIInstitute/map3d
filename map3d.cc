/* map3d.cxx */

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include "ParseCommandLineOptions.h"
#include "ProcessCommandLineOptions.h"
#include "colormaps.h"
#include "map3d-struct.h"
#include "regressiontest.h"
#include "savestate.h"

Map3d_Info map3d_info;
GdkGLConfig *glconfig;


char *units_strings[5] = { "mV", "uV", "ms", "V", "mVms" };

void initGTK(int argc, char **argv) {
  gtk_init(&argc, &argv);
  gtk_gl_init(&argc, &argv);

  glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode) (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE));
  if (glconfig == NULL) {
    g_print("*** Cannot find the double-buffered visual.\n");
    g_print("*** Trying single-buffered visual.\n");

    /* Try single-buffered visual */
    glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode) (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH));
    if (glconfig == NULL) {
      g_print("*** No appropriate OpenGL-capable visual found.\n");
      exit(1);
    }
  }
}

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

  // parse the arguments - it is befroe initGTK so it can spit out a usage string without having a display set
  // read .map3drc file and prepend the state to the front of the arguments.  If we do this we will
  // have to delete argv, as we will have dynamically allocated it.
  int old_argc = argc;
  char** old_argv = argv;
  bool loadedState = ReadMap3drc(argc, argv);
  if (!ParseCommandLineOptions(argc, argv, globalInput))
    exit(1);

  // give gtk the old args in case we changed them with readMap3drc,
  // gtk (at least windows optimized version) does not like changing the args.
  initGTK(old_argc, old_argv);

  // process the command line options - build windows, setup meshes, etc.
  if (!ProcessCommandLineOptions(globalInput))
    exit(1);

  //printf("GTK Version: %d.%d.%d\n", gtk_major_version, gtk_minor_version, gtk_micro_version);

  /* start the event system */
#ifdef REGRESSION_TEST
  int idle_id = 0;
  idle_id = gtk_idle_add_priority(GTK_PRIORITY_REDRAW, (GtkFunction) regressionTest, NULL);
#endif
  gtk_main();

  // delete the new argv if it was created in readMap3drc
  if (loadedState) {
    for (int i = 0; i < argc; i++)
      delete argv[i];
    delete argv;
  }
  return 0;
}
