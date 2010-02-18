/* MainWindow.h */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QFrame;

class MainWindow : public QMainWindow
{
public:
  MainWindow();
  char mainWindowText[7][257];
  int textLines;
  bool startHidden;
  void adjustSize();  // to be called after setting size/pos of child widget

  QLabel* label;
  QWidget* childrenFrame;

public slots:
  void updateLabel();
};

enum Menu_item
{ quit = -99, back_color, fore_color, info };


#endif
