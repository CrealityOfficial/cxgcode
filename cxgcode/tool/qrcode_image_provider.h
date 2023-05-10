#pragma once

#ifndef CXCLOUD_MODEL_QRCODE_IMAGE_PROVIDER_H_
#define CXCLOUD_MODEL_QRCODE_IMAGE_PROVIDER_H_

#include <QtGui/QImage>
#include <QtQuick/QQuickImageProvider>

#include "../interface.hpp"

namespace cxcloud {

class CXCLOUD_API QrcodeImageProvider : public QObject, public QQuickImageProvider {
public:
  QrcodeImageProvider(QObject* parent = nullptr);
  virtual ~QrcodeImageProvider();

  void setIdentical(const QString& identical);
  QImage Url2QrCodeImage(QString urlStr, int imageWidth, int imageHeight);
  QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);

private:
  QImage m_image;
};

}  // namespace cxcloud

#endif  // CXCLOUD_MODEL_QRCODE_IMAGE_PROVIDER_H_
