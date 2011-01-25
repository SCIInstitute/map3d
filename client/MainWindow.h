/* MainWindow.h */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QFrame;
class QContextMenuEvent;

class MainWindow : public QWidget
{
  Q_OBJECT;
public:
  MainWindow();
  char mainWindowText[7][257];
  int textLines;
  bool startHidden;

  QLabel* label;
  QWidget* childrenFrame;
  QWidget* mainFrame;

  virtual void contextMenuEvent(QContextMenuEvent* event);
public slots:
  void updateLabel();
  void updateBGColor(QColor color);
  void updateFGColor(QColor color);
  void adjustSize();  // to be called after setting size/pos of child widget and label

  void closeEvent(QCloseEvent *event);
};

enum Menu_item
{ quit = -99, back_color, fore_color, info };


#endif
