#include "exportdialog.h"
#include <QFileDialog>

#include <QProgressDialog>
#include "ui_exportdialog.h"
ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ExportDialog),
      supersample(1),
      image_width(1280),
      image_height(1024) {
  ui->setupUi(this);
  QObject::connect(this, &ExportDialog::supersamplingChanged,
                   ui->supersamplingBox, &QSpinBox::setValue);
  QObject::connect(this, &ExportDialog::imageWidthChanged, ui->widthBox,
                   &QSpinBox::setValue);
  QObject::connect(this, &ExportDialog::imageHeightChanged, ui->heightBox,
                   &QSpinBox::setValue);
}

ExportDialog::~ExportDialog() { delete ui; }

void ExportDialog::on_buttonBox_accepted() {
  dlg = new QProgressDialog(this);
  dlg->setRange(0, 0);
  dlg->setModal(true);
  dlg->setWindowTitle("Rendering Mandelbrot Set");
  dlg->setLabelText("Rendering image...");
  dlg->show();
  renderer = new RenderThread(dlg);
  QObject::connect(dlg, &QProgressDialog::canceled, this,
                   &ExportDialog::cancelRender);
  QObject::connect(renderer, SIGNAL(renderedImage(QImage, double, qint64)),
                   this, SLOT(saveImage(QImage, double, qint64)));
  renderer->render(posX, posY, scale, ui->supersamplingBox->value(),
                   QSize(ui->widthBox->value(), ui->heightBox->value()),
                   static_cast<ulong>(ui->iterationsBox->value()));
}

void ExportDialog::cancelRender() {
  renderer->abort();
  delete renderer;
  delete dlg;
}

void ExportDialog::saveImage(const QImage &image, double scaleFactor,
                             qint64 elapsed) {
  Q_UNUSED(scaleFactor)
  Q_UNUSED(elapsed)
  dlg->setLabelText("Saving image...");
  QString filename = QFileDialog::getSaveFileName(
      dynamic_cast<QWidget *>(this), "Save file as", "~/", "PNG images(*.png)");
  if (!filename.isEmpty()) {
    image.save(filename, "png", 100);
  }
  dlg->cancel();
  delete dlg;
}

qreal ExportDialog::getScale() const { return scale; }

void ExportDialog::setScale(const qreal &value) { scale = value; }

qreal ExportDialog::getPosY() const { return posY; }

void ExportDialog::setPosY(const qreal &value) { posY = value; }

qreal ExportDialog::getPosX() const { return posX; }

void ExportDialog::setPosX(const qreal &value) { posX = value; }

int ExportDialog::getimageHeight() const { return image_height; }

void ExportDialog::setimageHeight(int value) {
  image_height = value;
  emit imageHeightChanged(value);
}

int ExportDialog::getimageWidth() const { return image_width; }

void ExportDialog::setimageWidth(int value) {
  image_width = value;
  emit imageWidthChanged(value);
}

int ExportDialog::getSupersample() const { return supersample; }

void ExportDialog::setSupersample(int value) {
  supersample = value;
  emit supersamplingChanged(value);
}
