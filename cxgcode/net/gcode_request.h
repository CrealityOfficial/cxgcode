#pragma once

#ifndef CXCLOUD_NET_GCODE_REQUEST_H_
#define CXCLOUD_NET_GCODE_REQUEST_H_

#include "../interface.hpp"
#include "http_request.h"

namespace cxcloud {

class CXCLOUD_API DeleteGCodeRequest : public HttpRequest {
  Q_OBJECT
public:
  DeleteGCodeRequest(const QString& id, QObject* parent = nullptr);
  virtual ~DeleteGCodeRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString m_id;
};

class CXCLOUD_API UploadGCodeRequest : public HttpRequest {
  Q_OBJECT
public:
  UploadGCodeRequest(const QString& name, const QString& ossKey, QObject* parent = nullptr);
  virtual ~UploadGCodeRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString m_name;
  QString m_ossKey;
};

class CXCLOUD_API DownloadGCodeRequest : public HttpRequest {
  Q_OBJECT
public:
  DownloadGCodeRequest(const QString& id,
                       const QString& name,
                       const QString& link,
                       QObject* parent = nullptr);
  virtual ~DownloadGCodeRequest();

  const QString& getZipPath() const;
  const QString& getFilePath() const;

protected:
  QString getDownloadDstPath() const override;
  QString getDownloadSrcUrl() const override;
  QString getUrlTail() const override;

protected:
  QString dir_path_;
  QString m_name;
  QString m_link;

  QString m_fileName;
};

}  // namespace cxcloud

#endif  // CXCLOUD_NET_GCODE_REQUEST_H_
