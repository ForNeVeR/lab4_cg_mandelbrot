#include <QKeyEvent>
#include <QPainter>

#include <math.h>

#include "mandelbrotwidget.h"

const double DefaultCenterX = -0.637011;
const double DefaultCenterY = -0.0395159;
const double DefaultScale = 0.00403897;

const double ZoomInFactor = 0.8;
const double ZoomOutFactor = 1 / ZoomInFactor;
const int ScrollStep = 20;

MandelbrotWidget::MandelbrotWidget(QWidget *parent) : QWidget(parent) {
  centerX = DefaultCenterX;
  centerY = DefaultCenterY;
  pixmapScale = DefaultScale;
  curScale = DefaultScale;

  connect(&thread, SIGNAL(renderedImage(QImage, double, double, qint64)), this,
          SLOT(updatePixmap(QImage, double, double, qint64)));

  setWindowTitle(tr("Mandelbrot set"));
#ifndef QT_NO_CURSOR
  setCursor(Qt::CrossCursor);
#endif
  resize(1000, 600);
}

void MandelbrotWidget::paintEvent(QPaintEvent * /* event */) {
  QPainter painter(this);
  painter.fillRect(rect(), Qt::black);

  if (pixmap.isNull()) {
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter,
                     tr("Rendering initial image, please wait..."));
    return;
  }

  if (curScale == pixmapScale) {
    painter.drawPixmap(pixmapOffset, pixmap);
  } else {
    double scaleFactor = pixmapScale / curScale;
    int newWidth = int(pixmap.width() * scaleFactor);
    int newHeight = int(pixmap.height() * scaleFactor);
    int newX = pixmapOffset.x() + (pixmap.width() - newWidth) / 2;
    int newY = pixmapOffset.y() + (pixmap.height() - newHeight) / 2;

    painter.save();
    painter.translate(newX, newY);
    painter.scale(scaleFactor, scaleFactor);
    QRectF exposed =
        painter.matrix().inverted().mapRect(rect()).adjusted(-1, -1, 1, 1);
    painter.drawPixmap(exposed, pixmap, exposed);
    painter.restore();
  }

  QString text = tr("Use mouse wheel or the '+' and '-' keys to zoom. "
                    "Press and hold left mouse button to scroll. "
                    "Elapsed: %1 msec")
                     .arg(elapsedTimeMsec);
  QFontMetrics metrics = painter.fontMetrics();
  int textWidth = metrics.width(text);

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(0, 0, 0, 127));
  painter.drawRect((width() - textWidth) / 2 - 5, 0, textWidth + 10,
                   metrics.lineSpacing() + 5);
  painter.setPen(Qt::white);
  painter.drawText((width() - textWidth) / 2,
                   metrics.leading() + metrics.ascent(), text);
}

void MandelbrotWidget::resizeEvent(QResizeEvent * /* event */) {
  thread.render(centerX, centerY, curScale, size());
}

void MandelbrotWidget::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_Plus:
      zoom(ZoomInFactor);
      break;
    case Qt::Key_Minus:
      zoom(ZoomOutFactor);
      break;
    case Qt::Key_Left:
      scroll(-ScrollStep, 0);
      break;
    case Qt::Key_Right:
      scroll(+ScrollStep, 0);
      break;
    case Qt::Key_Down:
      scroll(0, -ScrollStep);
      break;
    case Qt::Key_Up:
      scroll(0, +ScrollStep);
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

#ifndef QT_NO_WHEELEVENT
void MandelbrotWidget::wheelEvent(QWheelEvent *event) {
  int numDegrees = event->delta() / 8;
  double numSteps = numDegrees / 15.0;
  zoom(pow(ZoomInFactor, numSteps));
}
#endif

void MandelbrotWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) lastDragPos = event->pos();
}

void MandelbrotWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    pixmapOffset += event->pos() - lastDragPos;
    lastDragPos = event->pos();
    update();
  }
}

void MandelbrotWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    pixmapOffset += event->pos() - lastDragPos;
    lastDragPos = QPoint();

    int deltaX = (width() - pixmap.width()) / 2 - pixmapOffset.x();
    int deltaY = (height() - pixmap.height()) / 2 - pixmapOffset.y();
    scroll(deltaX, deltaY);
  }
}

void MandelbrotWidget::updatePixmap(const QImage &image, double scaleFactor,
                                    double curImageScale, qint64 elapsed) {
  if (!lastDragPos.isNull()) return;

  pixmap = QPixmap::fromImage(image);
  pixmapOffset = QPoint();
  lastDragPos = QPoint();
  pixmapScale = scaleFactor;
  elapsedTimeMsec = elapsed;
  imageScale = curImageScale;
  update();
}

void MandelbrotWidget::zoom(double zoomFactor) {
  curScale *= zoomFactor;
  update();
  thread.render(centerX, centerY, curScale, size());
}

void MandelbrotWidget::scroll(int deltaX, int deltaY) {
  centerX += deltaX * curScale;
  centerY += deltaY * curScale;
  update();
  thread.render(centerX, centerY, curScale, size());
}
