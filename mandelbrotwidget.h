#ifndef MANDELBROTWIDGET_H
#define MANDELBROTWIDGET_H

#include <QPixmap>
#include <QWidget>
#include "renderthread.h"

class MandelbrotWidget : public QWidget {
  Q_OBJECT

 public:
  MandelbrotWidget(QWidget *parent = 0);

 protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
  void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
#ifndef QT_NO_WHEELEVENT
  void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
#endif
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

 private slots:
  void updatePixmap(const QImage &image, double scaleFactor,
                    double curImageScale, qint64 elapsed);
  void zoom(double zoomFactor);

 private:
  void scroll(int deltaX, int deltaY);

  RenderThread thread;
  QPixmap pixmap;
  QPoint pixmapOffset, lastDragPos;
  double centerX, centerY, pixmapScale, curScale, imageScale;
  qint64 elapsedTimeMsec;
};

#endif  // MANDELBROTWIDGET_H
