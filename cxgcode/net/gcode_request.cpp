#include "gcode_request.h"

#include <QtCore/QVariant>

namespace cxcloud {

DeleteGCodeRequest::DeleteGCodeRequest(const QString& id, QObject* parent)
    : HttpRequest(parent), m_id(id) {}

DeleteGCodeRequest::~DeleteGCodeRequest() {}

QByteArray DeleteGCodeRequest::getRequestData() const {
  return QString("{\"id\":\"%1\"}").arg(m_id).toUtf8();
}

QString DeleteGCodeRequest::getUrlTail() const {
  return "/api/cxy/v2/gcode/deleteGcode";
}

UploadGCodeRequest::UploadGCodeRequest(const QString& name, const QString& ossKey, QObject* parent)
    : HttpRequest(parent), m_name(name), m_ossKey(ossKey) {}

UploadGCodeRequest::~UploadGCodeRequest() {}

QByteArray UploadGCodeRequest::getRequestData() const {
  return QString("{\"list\":[{\"name\": \"%1\", \"filekey\" : \"%2\"}]}")
    .arg(m_name)
    .arg(m_ossKey)
    .toUtf8();
}

QString UploadGCodeRequest::getUrlTail() const {
  return "/api/cxy/v2/gcode/uploadGcode";
}

DownloadGCodeRequest::DownloadGCodeRequest(const QString& id,
                                           const QString& name,
                                           const QString& link,
                                           QObject* parent)
    : HttpRequest(parent), dir_path_(id), m_name(name), m_link(link) {
  m_fileName = QString("%1/%2.gcode.gz").arg(dir_path_).arg(name);
}

DownloadGCodeRequest::~DownloadGCodeRequest() {}

const QString& DownloadGCodeRequest::getZipPath() const {
  return m_fileName;
}

const QString& DownloadGCodeRequest::getFilePath() const {
  return m_fileName.left(m_fileName.lastIndexOf("."));
}

QString DownloadGCodeRequest::getDownloadDstPath() const {
  return m_fileName;
}

QString DownloadGCodeRequest::getDownloadSrcUrl() const {
  return m_link;
}

QString DownloadGCodeRequest::getUrlTail() const {
  return "";
}

}  // namespace cxcloud
