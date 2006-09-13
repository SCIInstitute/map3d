/* regressiontest.cxx */

#include <stdio.h>
#include "GeomWindow.h"
#include "eventdata.h"
#include "WindowManager.h"
#include "regressiontest.h"
#include "regressionlist.h"

void geomMenuTest(void)
{
  static int i = 1;
  static bool done = false;

  //for (i = 1; i < 250; i++) {
  if (menulist[i] == NULL && !done) {
    done = true;
    printf("Completed geomMenuTest\n");
  }
  if (done)
    return;
  printf("Testing geom menu item %-50s ", menulist[i]);
  fflush(stdout);

  menu_data md(i, GetGeomWindow(0));
  GeomWindowMenu(&md);
  printf("[[32mOK[0m]\n");
  i++;
}

//called from the idle loop, if you're going to add other stuff,
//  make sure you can tell it how many times to execute
void regressionTest(void)
{
  geomMenuTest();
}
