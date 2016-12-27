#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QColor>
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>

class QImage;

class RenderThread : public QThread {
  Q_OBJECT

 public:
  RenderThread(QObject *parent = 0);
  ~RenderThread();

  void render(double centerX, double centerY, double scaleFactor,
              int supersample = 1, QSize resultSize = QSize(),
              ulong maxIterations = 0);
 signals:
  void renderedImage(const QImage &image, double scaleFactor, qint64 elapsed);
 public slots:
  void restart();
  void abort();

 protected:
  void run() Q_DECL_OVERRIDE;

  void calculateInterlacedShift(QImage *image, int halfWidth, int halfHeight,
                                int interlaced_gap, int shift_x, int shift_y,
                                double center_x, double center_y,
                                int SUPERSAMPLE_SCALE, ulong MaxIterations);

 private:
  uint rgbFromWaveLength(double wave);

  QMutex mutex;
  QWaitCondition condition;
  qreal centerX, centerY, scaleFactor;
  ulong maxIterations;
  QSize resultSize;
  int supersampleScale = 1;
  static const int colormapSize = 12;
  QColor colormap[colormapSize];
  bool _restart, _abort;
};

#endif  // RENDERTHREAD_H
