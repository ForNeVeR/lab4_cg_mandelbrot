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
              QSize resultSize);

 signals:
  void renderedImage(const QImage &image, double scaleFactor, double imageScale,
                     qint64 elapsed);

 protected:
  void run() Q_DECL_OVERRIDE;

  void calculateIntelacedShift(QImage *image, int halfWidth, int halfHeight,
                               int interlaced_gap, int shift_x, int shift_y,
                               double center_x, double center_y,
                               ulong MaxIterations);

 private:
  uint rgbFromWaveLength(double wave);

  QMutex mutex;
  QWaitCondition condition;
  qreal centerX, centerY, scaleFactor;
  QSize resultSize;
  static const int colormapSize = 512;
  QColor colormap[colormapSize];
  bool restart, abort;
};

#endif  // RENDERTHREAD_H
