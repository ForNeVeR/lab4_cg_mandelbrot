#ifndef MANDELBROTWIDGET_H
#define MANDELBROTWIDGET_H

#include <QPixmap>
#include <QWidget>
#include "renderthread.h"

class MandelbrotWidget : public QWidget {
  Q_OBJECT

 public:
  MandelbrotWidget(QWidget *parent = 0);

  double getCenterX() const;
  void setCenterX(double value);

  double getCenterY() const;
  void setCenterY(double value);

  double getPixmapScale() const;

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
  void updatePixmap(const QImage &image, double scaleFactor, qint64 elapsed);
  void zoom(double zoomFactor);

 private:
  void scroll(int deltaX, int deltaY);

  RenderThread thread;
  QPixmap pixmap;
  QPoint pixmapOffset, lastDragPos;
  QSize curSize;
  int supersampling;
  double centerX, centerY, pixmapScale, curScale;
  qint64 elapsedTimeMsec;
};

#endif  // MANDELBROTWIDGET_H
