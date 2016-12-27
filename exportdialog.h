#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include "renderthread.h"
namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ExportDialog(QWidget *parent = 0);
  ~ExportDialog();

  int getSupersample() const;
  void setSupersample(int value);

  int getimageWidth() const;
  void setimageWidth(int value);

  int getimageHeight() const;
  void setimageHeight(int value);
  qreal getPosX() const;
  void setPosX(const qreal &value);

  qreal getPosY() const;
  void setPosY(const qreal &value);

  qreal getScale() const;
  void setScale(const qreal &value);

 signals:
  void supersamplingChanged(int value);
  void imageWidthChanged(int value);
  void imageHeightChanged(int value);
 public slots:
  void saveImage(const QImage &image, double scaleFactor, qint64 elapsed);
 private slots:
  void on_buttonBox_accepted();
  void cancelRender();

 private:
  Ui::ExportDialog *ui;
  QProgressDialog *dlg;
  int supersample, image_width, image_height;
  qreal posX, posY, scale;
  RenderThread *renderer;
};

#endif  // EXPORTDIALOG_H
