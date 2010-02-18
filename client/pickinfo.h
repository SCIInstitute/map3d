/* pickinfo.h */

#ifndef PICKINFO_H
#define PICKINFO_H

class PickWindow;
class Mesh_Info;

struct PickInfo
{
  PickWindow *pickwin;
  Mesh_Info* mesh;
  int node;
  int type;
  bool show;
  bool rms;
  double depth;
};

#define NEW_WINDOW_PICK_MODE 0
#define REFRESH_WINDOW_PICK_MODE 1
#define INFO_PICK_MODE 2
#define TRIANGLE_INFO_PICK_MODE 3
#define TRIANGULATE_PICK_MODE 4
#define FLIP_TRIANGLE_PICK_MODE 5
#define EDIT_NODE_PICK_MODE 6
#define EDIT_LANDMARK_PICK_MODE 7
#define DELETE_NODE_PICK_MODE 8
#define REFERENCE_PICK_MODE 9
#define MEAN_REFERENCE_PICK_MODE 10

#endif
