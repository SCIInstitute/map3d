/* eventdata.h */

#ifndef EVENTDATA_H
#define EVENTDATA_H

#define MAP3D_MOUSE_MOTION  1
//#define MAP3D_MOUSE_BUTTON  2
#define MAP3D_UPDATE        3
#define MAP3D_KEY           4
#define MAP3D_MENU          5
#define MAP3D_REPAINT_ALL   6
#define MAP3D_FRAMES        10
#define MAP3D_PICK_FRAMES   11
#define MAP3D_MOUSE_BUTTON_PRESS 13
#define MAP3D_MOUSE_BUTTON_RELEASE 14
#define MAP3D_KEY_PRESS     15
#define MAP3D_KEY_RELEASE   16

struct mouse_button_data
{
  int button;
  int state;
  int modifiers;
  float x;
  float y;
};

struct mouse_motion_data
{
  float x;
  float y;
};

struct special_key_data
{
  int key;
  float x;
  float y;
};

struct key_data
{
  int key;
  float x;
  float y;
};

#endif
