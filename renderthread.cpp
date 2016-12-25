/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderthread.h"
#include <QElapsedTimer>
#include <QFuture>
#include <QtConcurrentMap>
#include <QtWidgets>
#include <cmath>
const int SUPERSAMPLE_SCALE(4);
// helpers
template <typename T>
T lerp(T v0, T v1, T t) {
  return (1 - t) * v0 + t * v1;
}

template <typename T, typename U>
T lerp(T v0, T v1, U t) {
  return (1 - t) * v0 + t * v1;
}
int limrand(int low, int high) {
  int x;
  do {
    x = qrand();
  } while (x >= (RAND_MAX - RAND_MAX % high));
  return low + x % high;
}

QColor myColorInterpolator(const QColor &start, const QColor &end,
                           qreal progress) {
  return QColor::fromRgbF(lerp(start.redF(), end.redF(), progress),
                          lerp(start.greenF(), end.greenF(), progress),
                          lerp(start.blueF(), end.blueF(), progress));
}

QColor colorAvgSum(const QColor (*color_array)[SUPERSAMPLE_SCALE]) {
  int r(0), g(0), b(0);
  for (QColor color : *color_array) {
    r += color.red();
    g += color.green();
    b += color.blue();
  }
  r /= SUPERSAMPLE_SCALE;
  g /= SUPERSAMPLE_SCALE;
  b /= SUPERSAMPLE_SCALE;
  return QColor(r, g, b);
}

RenderThread::RenderThread(QObject *parent) : QThread(parent) {
  restart = false;
  abort = false;
  // Ultra Fractal palette
  QColor mapping[16];
  mapping[0].setRgb(66, 30, 15);
  mapping[1].setRgb(25, 7, 26);
  mapping[2].setRgb(9, 1, 47);
  mapping[3].setRgb(4, 4, 73);
  mapping[4].setRgb(0, 7, 100);
  mapping[5].setRgb(12, 44, 138);
  mapping[6].setRgb(24, 82, 177);
  mapping[7].setRgb(57, 125, 209);
  mapping[8].setRgb(134, 181, 229);
  mapping[9].setRgb(211, 236, 248);
  mapping[10].setRgb(241, 233, 191);
  mapping[11].setRgb(248, 201, 95);
  mapping[12].setRgb(255, 170, 0);
  mapping[13].setRgb(204, 128, 0);
  mapping[14].setRgb(153, 87, 0);
  mapping[15].setRgb(106, 52, 3);
  for (int i = 0; i < colormapSize; ++i) {
    colormap[i] = mapping[i % 16];
  }
}

RenderThread::~RenderThread() {
  mutex.lock();
  abort = true;
  condition.wakeOne();
  mutex.unlock();

  wait();
}

void RenderThread::render(double centerX, double centerY, double scaleFactor,
                          QSize resultSize) {
  QMutexLocker locker(&mutex);

  this->centerX = centerX;
  this->centerY = centerY;
  this->scaleFactor = scaleFactor;
  this->resultSize = resultSize;

  if (!isRunning()) {
    start(LowPriority);
  } else {
    restart = true;
    condition.wakeOne();
  }
}

void RenderThread::run() {
  qsrand(static_cast<uint>(time(0)));
  forever {
    mutex.lock();
    QSize resultSize = this->resultSize;
    double scaleFactor = this->scaleFactor;
    double center_x = this->centerX;
    double center_y = this->centerY;

    mutex.unlock();
    int halfWidth = resultSize.width() / 2;
    int halfHeight = resultSize.height() / 2;
    QElapsedTimer timer;
    timer.start();

    ulong MaxIterations = 1000 + static_cast<ulong>(1 / (scaleFactor * 1000));

    int interlaced_gap = 10;
    // draw interlaced
    QVector<QPoint> shifts;
    if (restart) break;
    if (abort) return;
    int shift_x = 0;
    int shift_y = 0;
    while (shift_y < interlaced_gap) {
      shifts.append(QPoint(shift_x, shift_y));
      if (shift_x < interlaced_gap - 1) {
        ++shift_x;
      } else {
        ++shift_y;
        shift_x = 0;
      }
    }
    QImage image(resultSize, QImage::Format_RGB32);
    image.fill(Qt::black);
    auto calc = [&](QPoint shift) {
      return calculateIntelacedShift(&image, halfWidth, halfHeight,
                                     interlaced_gap, shift.x(), shift.y(),
                                     center_x, center_y, MaxIterations);
    };
    QtConcurrent::blockingMap(shifts, calc);

    if (!restart)
      emit renderedImage(image, scaleFactor, interlaced_gap, timer.elapsed());

    mutex.lock();
    if (!restart) condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

void RenderThread::calculateIntelacedShift(QImage *image, int halfWidth,
                                           int halfHeight, int interlaced_gap,
                                           int shift_x, int shift_y,
                                           double center_x, double center_y,
                                           ulong MaxIterations) {
  const int Limit = (1 << 16);
  for (int y = -halfHeight; y < halfHeight; y += interlaced_gap) {
    if (!((y + shift_y) < halfHeight)) break;
    int y_d = (y + shift_y);

    for (int x = -halfWidth; x < halfWidth; x += interlaced_gap) {
      if (!((x + shift_x) < halfWidth)) break;
      int x_d = (x + shift_x);

      // this uses random supersampling
      // calculate a given number of random subpixels inside a pixel and
      // average a sum of colors
      QColor sample_colors[SUPERSAMPLE_SCALE];
      for (int i(0); i < SUPERSAMPLE_SCALE; ++i) {
        if (restart) break;
        if (abort) return;

        double x0 =
            center_x +
            lerp(x_d - 0.5, x_d + 0.5, 1 / static_cast<double>(qrand())) *
                scaleFactor;
        double y0 =
            center_y +
            lerp(y_d - 0.5, y_d + 0.5, 1 / static_cast<double>(qrand())) *
                scaleFactor;
        ulong numIterations = 0;
        // cardioid check
        double q = (x0 * x0 - x0 / 2. + 1 / 16.) + y0 * y0;
        if (q * (q + (x0 - 1 / 4.)) < (y0 * y0 / 4.))
          numIterations = MaxIterations;
        double cr = x0;
        double ci = y0;
        while ((cr * cr) + (ci * ci) <= Limit &&
               numIterations < MaxIterations) {
          ++numIterations;
          double buffer_cr = (cr * cr) - (ci * ci) + x0;
          double buffer_ci = (2 * cr * ci) + y0;
          // periodic check
          if (fabs(cr - buffer_cr) < std::numeric_limits<double>::epsilon() &&
              fabs(ci - buffer_ci) < std::numeric_limits<double>::epsilon()) {
            numIterations = MaxIterations;
            break;
          }
          cr = buffer_cr;
          ci = buffer_ci;
        }
        if (numIterations < MaxIterations) {
          qreal log_zn(log(cr * cr + ci * ci) / 2),
              nu(numIterations + 1 - (log(log_zn / log(2)) / log(2))),
              smoothcolor(nu - floor(nu));

          sample_colors[i] = myColorInterpolator(
              colormap[(numIterations + 1) % colormapSize],
              colormap[(numIterations) % colormapSize], 1 - smoothcolor);
        } else {
          sample_colors[i] = qRgb(0, 0, 0);
        }
      }

      // sum of colors
      image->setPixelColor(x_d + halfWidth, y_d + halfHeight,
                           colorAvgSum(&sample_colors));
    }
  }
}
