/* savestate.h */

#ifndef _SAVESTATE_H
#define _SAVESTATE_H

// saving the state will save a .map3drc file which will save things like mesh
// color, transformation matrices, scaling options, etc.  Reading the state 
// will simply prepend the arguments to the initial argv so they can be parsed
// as though by the command line.  The state can be overridden by command-line
// arguments.
bool ReadMap3drc(int& argc, char**& argv);

// saving a script will result in a map3d script which will be as near as
// possible to the current session as possible.  The difference between
// saving a script and saving state is that a script contains 
// information for each surface, and is a complete map3d command line,
// and state is only a set of options.
void SaveSettings(const char* filename, bool map3drc, bool windows);

#endif
