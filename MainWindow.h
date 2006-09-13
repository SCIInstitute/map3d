/* MainWindow.h */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "GenericWindow.h"

typedef struct _GtkWidget GtkWidget;

class MainWindow : public GenericWindow
{
public:
  MainWindow();
  virtual void setupEventHandlers();
  void buildMenus();
  GtkWidget *window;
  GtkWidget *fixed;
  GtkWidget *focus;
  char mainWindowText[7][257];
  int textLines;
  bool startHidden;
};

enum Menu_item
{ quit = -99, back_color, fore_color, info };
#ifdef __cplusplus
extern "C"
{
#endif

  struct menu_data;
  typedef union _GdkEvent GdkEvent;
  typedef struct _GdkEventMotion GdkEventMotion;
  typedef struct _GdkEventButton GdkEventButton;
  typedef struct _GdkEventKey GdkEventKey;
  typedef void* gpointer;


  int MainWindowCreate( /*MainWindow * priv */ );
  void MainWindowRepaint(GtkWidget * widget, GdkEvent * event, gpointer data);
  void MainWindowReshape(GtkWidget * widget, GdkEvent * event, gpointer data);
  void MainWindowKeyboardPress(GtkWidget * widget, GdkEventKey * event, gpointer data);
  void MainWindowKeyboardRelease(GtkWidget * widget, GdkEventKey * event, gpointer data);
  //void MainWindowKeyboard(unsigned char, int x, int y);
  void MainWindowMouseButton(GtkWidget * widget, GdkEventButton * event, gpointer data);
  void MainWindowMouseMotion(int x, int y);
  void MainWindowMenu(menu_data * data);
  void MainBuildMenus();
#ifdef __cplusplus
}
#endif

#endif
