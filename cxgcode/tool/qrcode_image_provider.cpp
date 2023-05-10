#include "qrcode_image_provider.h"

#include <QtGui/QPainter>

#include <qrencode.h>

namespace cxcloud {

QrcodeImageProvider::QrcodeImageProvider(QObject* parent)
    : QObject(parent), QQuickImageProvider(QQuickImageProvider::Image) {}

QrcodeImageProvider::~QrcodeImageProvider() {}

void QrcodeImageProvider::setIdentical(const QString& identical) {
  // m_identical = identical;
  QString url = "https://share.creality.com/scan-code?i=" + identical;
  m_image = Url2QrCodeImage(url, 150, 150);
}

QImage QrcodeImageProvider::Url2QrCodeImage(QString urlStr, int imageWidth, int imageHeight) {
  // QR_ECLEVEL_Q
  QRcode* qrcode = QRcode_encodeString(urlStr.toStdString().c_str(), 2, QR_ECLEVEL_Q, QR_MODE_8, 1);
  qint32 temp_width = imageWidth;
  qint32 temp_height = imageHeight;
  qint32 qrcode_width = qrcode->width > 0 ? qrcode->width : 1;
  double scale_x = (double)temp_width / (double)qrcode_width;
  double scale_y = (double)temp_height / (double)qrcode_width;

  QImage mainimg = QImage(temp_width, temp_height, QImage::Format_ARGB32);
  QPainter painter(&mainimg);
  QColor background(Qt::white);
  painter.setBrush(background);
  painter.setPen(Qt::NoPen);
  painter.drawRect(0, 0, temp_width, temp_height);
  QColor foreground(Qt::black);
  painter.setBrush(foreground);
  for (qint32 y = 0; y < qrcode_width; y++) {
    for (qint32 x = 0; x < qrcode_width; x++) {
      unsigned char b = qrcode->data[y * qrcode_width + x];
      if (b & 0x01) {
        QRectF r(x * scale_x, y * scale_y, scale_x, scale_y);
        painter.drawRects(&r, 1);
      }
    }
  }

  return mainimg;
}

QImage QrcodeImageProvider::requestImage(const QString& id,
                                         QSize* size,
                                         const QSize& requestedSize) {
  return m_image;
}

}  // namespace cxcloud
