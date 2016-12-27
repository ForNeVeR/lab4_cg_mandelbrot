#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "exportdialog.h"
#include "mandelbrotwidget.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

 private slots:
  void on_actionExport_Image_triggered();

  void on_actionAbout_triggered();

 private:
  Ui::MainWindow *ui;
  MandelbrotWidget mandelbrot;
  ExportDialog dialog;
};

#endif  // MAINWINDOW_H
