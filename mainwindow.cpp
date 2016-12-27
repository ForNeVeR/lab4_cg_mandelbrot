#include "mainwindow.h"
#include <QMessageBox>
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), dialog(this) {
  ui->setupUi(this);
  setCentralWidget(dynamic_cast<QWidget *>(&mandelbrot));
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_actionExport_Image_triggered() {
  dialog.setPosX(mandelbrot.getCenterX());
  dialog.setPosY(mandelbrot.getCenterY());
  dialog.setScale(mandelbrot.getPixmapScale());
  dialog.show();
}

void MainWindow::on_actionAbout_triggered() {
  QMessageBox::information(this, "About",
                           "Mandelbrot rendering with supersampling and "
                           "optimisations.\nAlexander Shaposhnikov 2016");
}
