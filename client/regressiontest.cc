/* regressiontest.cxx */

#include <stdio.h>
#include "GeomWindow.h"
#include "eventdata.h"
#include "WindowManager.h"
#include "regressiontest.h"
#include "regressionlist.h"

#include <QApplication>

void MenuTester::geomMenuTest(void)
{
  static int i = 1;
  static bool done = false;

  //for (i = 1; i < 250; i++) {
  if (menulist[i] == NULL && !done) {
    done = true;
    printf("Completed geomMenuTest\n");
  }
  if (done)
  {
    timer.stop();
    deleteLater();
    return;
  }

  printf("Testing geom menu item %-50s ", menulist[i]);
  fflush(stdout);

  GetGeomWindow(0)->MenuEvent(i);
  qApp->processEvents();
  printf("[[32mOK[0m]\n");
  i++;
}

//called from the idle loop, if you're going to add other stuff,
//  make sure you can tell it how many times to execute
void MenuTester::regressionTest(void)
{
  connect(&timer, SIGNAL(timeout()), this, SLOT(geomMenuTest()));
  timer.setInterval(1);
  timer.setSingleShot(false);
  timer.start();
}
