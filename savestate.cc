/* savestate.cxx */

#include "savestate.h"
#include "GeomWindow.h"
#include "Surf_Data.h"
#include "WindowManager.h"
#include "Map3d_Geom.h"
#include "Transforms.h"
#include "pickinfo.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "dialogs.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <gtk/gtk.h>
using namespace std;

extern Map3d_Info map3d_info;

bool ReadMap3drc(int& argc, char**& argv)
{
  // NOTE - here we use iostream to read the file, where most of the
  // rest of map3d uses stdio, as we are not certain 
  // the length of strings in the file, and thus don't want to create any
  // security problems.  

  // look for .map3drc in 2 places - in current dir first, then in home dir
  string filename = ".map3drc";
  ifstream file(".map3drc");
  if (!file.is_open()) {
    char* home = getenv("HOME");
    if (home) {
      filename = home;
      filename += "/.map3drc";
      file.open(filename.c_str());
      if (!file.is_open()) {
        return false;
      }
    }
    else // if could not get "HOME" var
      return false;
  }

  printf("Parsing %s\n", filename.c_str());

  // see if the file has any items
  string buf;
  vector<string> newargv;
  newargv.push_back(argv[0]);

  // read the lines of the .map3drc, and push each argument to a list
  do {
    getline(file, buf);
    istringstream istr(buf);
    string tmp;
    if (buf[0] != '#') { // ignore lines that start with #
      while (!istr.eof()) {
        istr >> tmp;
        if (tmp != "")
          newargv.push_back(tmp);
      }
    }
  } while (!file.eof());

  if (newargv.size() == 1)
    return false;

  for (int i = 1; i < argc; i++)
    newargv.push_back(argv[i]);

  argc = newargv.size();
  argv = new char*[argc];

  // make a new set of argv and append this to the command line options.  We'll need to 
  // delete this later, that's why we return true.
  for (int i = 0; i < argc; i++) {
    argv[i] = new char[newargv[i].length()+1];
    strcpy(argv[i], newargv[i].c_str());
  }
  

  return true;
}

void outputGlobalInfo(FILE* f, char* cont, SaveDialog* sd)
{
  // only output global options if the global box is checked in dialog
  if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sd->settings_global)))
    return;

  // save -b
  if (map3d_info.qnoborders)
    fprintf(f, "-b%s",cont);

  // save scaling range, function, mapping
  fprintf(f, "-sc %d %d %d%s", map3d_info.scale_scope, map3d_info.scale_model, map3d_info.scale_mapping,cont);

  // save general, transform, frame lock
  fprintf(f, "-l %d %d %d%s", 
    map3d_info.lockgeneral, 
    map3d_info.lockrotate, 
    map3d_info.lockframes,cont);

  // report level
  fprintf(f, "-rl %d%s", map3d_info.reportlevel,cont);

  // pick mode?
  fprintf(f, "-pm %d%s", map3d_info.pickmode,cont);
}

// output info for a mesh.  The first mesh's options will be treated
// as global options if writing a map3drc.
// append the cont string to each set of arguments, which could differ between
// .map3drc/script, unix/windows.
void outputMeshInfo(FILE* f, char* cont, SaveDialog* sd, Mesh_Info* mesh)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sd->settings_meshoptions))) {
    // color
    fprintf(f, "-c %d %d %d%s", (int)(mesh->meshcolor[0]*255), (int)(mesh->meshcolor[1]*255), (int)(mesh->meshcolor[2]*255),cont);
    // shading mode
    fprintf(f, "-sm %d%s", mesh->shadingmodel,cont);
    // mesh render mode
    fprintf(f, "-rm %d%s", mesh->drawmesh,cont);
    // inverted colormap
    if (mesh->invert)
      fprintf(f, "-ic 1%s",cont);
    else
      fprintf(f, "-ic 0%s",cont);
    // contourstep OR number of contours
    // if (1 /* not default contour spacing*/)
    //fprintf(f, "-cs %f%s", mesh->contourspacing,cont);
    //if (mesh->data)
      //fprintf(f, "-nc %d%s", mesh->data->numconts,cont);
    // show legend?
    fprintf(f, "-slw %d%s",mesh->showlegend?1:0, cont);
    // lighting
    fprintf(f, "-el %d%s",mesh->lighting?1:0, cont);
    // depth cue
    fprintf(f, "-ef %d%s",mesh->fogging?1:0, cont);
    // draw contours?
    fprintf(f, "-sco %d%s",mesh->drawcont?1:0, cont);
    // neg conts dashed?
    fprintf(f, "-nc %d%s",mesh->negcontdashed?1:0, cont);
    // marks size and color (all of them)?
    //fprintf(f, "-c %d %d %d", (int)mesh->meshcolor[0]*255
    // axes?
    fprintf(f, "-x %d%s",mesh->axes?1:0, cont);
      // axes color
    if (mesh->axes) {
      fprintf(f, "-xc %d %d %d%s", (int)(mesh->axescolor[0]*255),(int)(mesh->axescolor[1]*255),(int)(mesh->axescolor[2]*255),cont);
    }
    // draw node marks (all)
    fprintf(f, "-nma %d %d %d%s",mesh->mark_all_sphere, mesh->mark_all_number, mesh->mark_all_sphere_value, cont);
    // draw node marks (extrema)
    fprintf(f, "-nme %d %d%s",mesh->mark_extrema_sphere, mesh->mark_extrema_number, cont);
    // draw node marks (pick)
    fprintf(f, "-nmp %d %d%s",mesh->mark_ts_sphere, mesh->mark_ts_number, cont);
    // draw node marks (lead)
    fprintf(f, "-nml %d %d%s",mesh->mark_lead_sphere, mesh->mark_lead_number, cont);
  }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sd->settings_transform))) {
    // rotation matrix
    Quat q = mesh->tran->rotate.qNow;
    fprintf(f, "-rq %f %f %f %f%s", q.w, q.x, q.y, q.z,cont);

    // translation coordinates
    fprintf(f, "-tc %f %f %f%s", mesh->tran->tx, mesh->tran->ty, mesh->tran->tz,cont);
  
    // zoom scale
    fprintf(f, "-zf %f%s", mesh->gpriv->vfov,cont);
  }

}

void SaveSettings(SaveDialog* sd)
{
  // loop through meshes

  //   if script, output surfaces
  //   output mesh options
  //   if mesh == 0 && rc break

  int map3drc = gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(sd->settings_map3drc)->check_button.toggle_button);
  char* filename;
  FILE *f;

  // the characters to output between settings.  For state, go ahead and output newlines.
  // For scripts, the shell needs to execute the entire file, so the command needs to be on one line.
  // However, on UNIX-type systems, we can continue a line with " \\\n"
  char cont[4];

  if (map3drc) {
    // save a state file
    filename = ".map3drc";
    strcpy(cont, "\n");
    printf("State saved in file %s\n", filename);
  }
  else {
    // save a script file
    filename = (char*) gtk_entry_get_text(GTK_ENTRY(sd->settings_filename));
    if (gtk_toggle_button_get_active(&GTK_RADIO_BUTTON(sd->settings_batch)->check_button.toggle_button))
      strcpy(cont, " ");
    else
      strcpy(cont, " \\\n");
    printf("Script saved in file %s\n", filename);
  }

  FILE* test = fopen(filename, "r");
  if (test) {
    char message[300];
    sprintf(message, "Overwrite %s?", filename);
    fclose(test);
    GtkWidget* overwrite = gtk_message_dialog_new(GTK_WINDOW(sd->window), GTK_DIALOG_DESTROY_WITH_PARENT, 
                              GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, message);
    int response = gtk_dialog_run(GTK_DIALOG(overwrite));
    gtk_widget_destroy(overwrite);
    if (response != GTK_RESPONSE_OK)
      return;
  }
  f = fopen(filename, "w");

  // if script output executable name
  if (!map3drc) {
    // convert executable name to have '/' instead of '\'
    char exec[256];
    unsigned i;
    for (i = 0; i < strlen(map3d_info.map3d_executable); i++) {
      if (map3d_info.map3d_executable[i] == '\\')
        exec[i] = '/';
      else
        exec[i] = map3d_info.map3d_executable[i];
    }
    exec[i] = 0;
    fprintf(f, "%s ", exec);
  }
  else {
    // .map3drc - save -nw - we don't want to save -nw in the global options because 
    //  we might want to specify -w in a script for specific surfaces
    if (!map3d_info.singlewin)
      fprintf(f, "-nw%s", cont);
  }

  // - GLOBAL OPTIONS - 
  outputGlobalInfo(f, cont, sd);

  // loop through each mesh
  bool firstmesh = true;
  for (unsigned win = 0; win < numGeomWindows(); win++) {
    GeomWindow* gpriv = GetGeomWindow(win);
    bool newWindow = true;
    for (unsigned i = 0; i < gpriv->meshes.size(); i++) {
      Mesh_Info* mesh = gpriv->meshes[i];
      if (i > 0 && mesh->mysurf == gpriv->meshes[i-1]->mysurf)
        continue;
      int x=0, y=0, width=0, height=0;
      if (!map3drc) {
        // save surface info
        fprintf(f, "-f %s@%d ", mesh->geom->basefilename, mesh->mysurf->geomsurfnum);
        if (mesh->data) {
          fprintf(f, "-p %s@%d ", mesh->data->potfilename, mesh->mysurf->timeseries);
          fprintf(f, "-s %d %d -i %d ", mesh->data->ts_start+1, mesh->data->ts_end+1, mesh->data->ts_sample_step);
        }
        if (mesh->mysurf->llfilename && strlen(mesh->mysurf->llfilename) > 0)
          fprintf(f, "-ll %s ", mesh->mysurf->llfilename);
        if (mesh->mysurf->lmfilename && strlen(mesh->mysurf->lmfilename) > 0)
          fprintf(f, "-lm %s ", mesh->mysurf->lmfilename);
        if (mesh->mysurf->chfilename && strlen(mesh->mysurf->chfilename) > 0)
          fprintf(f, "-ch %s ", mesh->mysurf->chfilename);
        if (newWindow) {
          fprintf(f, "-w ");
          // geom window coordinates
          gpriv->getCommandLineCoordinates(width, height, x, y);
          fprintf(f, "-as %d %d %d %d ", x, x+width, y, y+height);
        }
        for (int i = 0; i <= mesh->pickstacktop; i++) {
          fprintf(f, "-t %d ", mesh->pickstack[i]->node+1);
          mesh->pickstack[i]->pickwin->getCommandLineCoordinates(width, height, x, y);
          fprintf(f, "-at %d %d %d %d ", x, x+width, y, y+height);
          // pick windows and coordinates
        }
        // legend window coordinates
        if (mesh->legendwin)
          mesh->legendwin->getCommandLineCoordinates(width, height, x, y);
        fprintf(f, "-al %d %d %d %d ", x, x+width, y, y+height);
      }
      // output per-window info, that might also be useful in the .map3drc
      if (firstmesh || (!map3drc && newWindow)) {
        fprintf(f, "-fg %d %d %d -bg %d %d %d ", (int)gpriv->fgcolor[0]*255,(int)gpriv->fgcolor[1]*255,(int)gpriv->fgcolor[2]*255,
          (int)gpriv->bgcolor[0]*255,(int)gpriv->bgcolor[1]*255,(int)gpriv->bgcolor[2]*255);
        fprintf(f, "-lf %d -mf %d -sf %d ", (int)gpriv->large_font, (int)gpriv->med_font, (int)gpriv->small_font); 
      }
      // output info on first mesh for map3drc, or all meshes otherwise
      if (firstmesh || !map3drc)
        outputMeshInfo(f, cont, sd, mesh);
      firstmesh = false;
      newWindow = false;
    }
  }
  fclose(f);
#ifndef _WIN32
  //set execute permissions
  char cmd[256];
  sprintf(cmd, "chmod a+x %s", filename);
  system(cmd);
#endif
}
