/* regressiontest.h */

#ifndef _REGRESSIONTEST_H
#define _REGRESSIONTEST_H

#include <QTimer>

class MenuTester : public QObject
{
  Q_OBJECT;

public slots:
  void geomMenuTest(void);
  void regressionTest(void);
private:
  QTimer timer;
};

#endif
