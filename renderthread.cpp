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
#include <QtWidgets>
#include <cmath>

template <typename T>
T lerp(T v0, T v1, T t) {
  return (1 - t) * v0 + t * v1;
}

template <typename T, typename U>
T lerp(T v0, T v1, U t) {
  return (1 - t) * v0 + t * v1;
}

QColor myColorInterpolator(const QColor &start, const QColor &end,
                           qreal progress) {
  return QColor::fromRgbF(lerp(start.redF(), end.redF(), progress),
                          lerp(start.greenF(), end.greenF(), progress),
                          lerp(start.blueF(), end.blueF(), progress));
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
  forever {
    mutex.lock();
    QSize resultSize = this->resultSize;
    double scaleFactor = this->scaleFactor;
    double centerX = this->centerX;
    double centerY = this->centerY;
    mutex.unlock();
    QElapsedTimer timer;
    timer.start();
    int halfWidth = resultSize.width() / 2;
    int halfHeight = resultSize.height() / 2;
    QImage image(resultSize, QImage::Format_RGB32);
    ulong MaxIterations = 1000 + static_cast<ulong>(1 / (scaleFactor * 2000));
    const int Limit = (1 << 16);
    int interlaced_gap = 1;
    // draw interlaced
    int pass = 0;

    image.fill(Qt::black);
    int x = -halfWidth;
    while (pass < 1000) {
      if (restart) break;
      if (abort) return;
      int shift_x = 0;
      int shift_y = 0;
      if (pass > 1 || MaxIterations > 5000) {
        interlaced_gap = 3;
      }

      while (shift_y < interlaced_gap) {
        if (restart) break;
        if (abort) return;

        for (int y = -halfHeight; y < halfHeight; y += interlaced_gap) {
          if (restart) break;
          if (abort) return;
          if (!((y + shift_y) < halfHeight)) break;
          int y_d = (y + shift_y);
          long double y0 = static_cast<long double>(centerY) +
                           (y_d * static_cast<long double>(scaleFactor));

          for (x = -halfWidth; x < halfWidth; x += interlaced_gap) {
            if (restart) break;
            if (abort) return;
            if (!((x + shift_x) < halfWidth)) break;
            int x_d = (x + shift_x);
            long double x0 = static_cast<long double>(centerX) +
                             (x_d * static_cast<long double>(scaleFactor));
            long double cr = x0;
            long double ci = y0;
            ulong numIterations = 0;
            do {
              ++numIterations;
              long double buffer = cr;
              cr = (cr * cr) - (ci * ci) + x0;
              ci = (2 * buffer * ci) + y0;

            } while ((cr * cr) + (ci * ci) <= Limit &&
                     numIterations < MaxIterations);

            if (numIterations < MaxIterations) {
              qreal log_zn(static_cast<qreal>(logl(cr * cr + ci * ci) / 2)),
                  nu(numIterations + 1 - (log(log_zn / log(2)) / log(2))),
                  smoothcolor(nu - floor(nu));

              image.setPixelColor(
                  x_d + halfWidth, y_d + halfHeight,
                  myColorInterpolator(
                      colormap[(numIterations + 1) % colormapSize],
                      colormap[(numIterations) % colormapSize],
                      1 - smoothcolor));
            }
          }
        }

        if (!restart)
          emit renderedImage(image, scaleFactor, interlaced_gap,
                             timer.elapsed());
        if (shift_x < interlaced_gap - 1) {
          ++shift_x;
        } else {
          ++shift_y;
          shift_x = 0;
        }
      }

      pass++;
      MaxIterations += 1000;
    }
    mutex.lock();
    if (!restart) condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

uint RenderThread::rgbFromWaveLength(double wave) {
  double r = 0.0;
  double g = 0.0;
  double b = 0.0;

  if (wave >= 380.0 && wave <= 440.0) {
    r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
    b = 1.0;
  } else if (wave >= 440.0 && wave <= 490.0) {
    g = (wave - 440.0) / (490.0 - 440.0);
    b = 1.0;
  } else if (wave >= 490.0 && wave <= 510.0) {
    g = 1.0;
    b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
  } else if (wave >= 510.0 && wave <= 580.0) {
    r = (wave - 510.0) / (580.0 - 510.0);
    g = 1.0;
  } else if (wave >= 580.0 && wave <= 645.0) {
    r = 1.0;
    g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
  } else if (wave >= 645.0 && wave <= 780.0) {
    r = 1.0;
  }

  double s = 1.0;
  if (wave > 700.0)
    s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
  else if (wave < 420.0)
    s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

  r = std::pow(r * s, 0.8);
  g = std::pow(g * s, 0.8);
  b = std::pow(b * s, 0.8);
  return qRgb(int(r * 255), int(g * 255), int(b * 255));
}
