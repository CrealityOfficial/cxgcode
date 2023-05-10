#pragma once

#ifndef CXCLOUD_NET_OSS_REQUEST_H_
#define CXCLOUD_NET_OSS_REQUEST_H_

#include <functional>

#include <QPointer>

#include "../interface.hpp"
#include "http_request.h"

namespace cxcloud {

class OssRequest;

CXCLOUD_API void StartOssRequest(QPointer<OssRequest> request);

enum class OSSState {
  oss_info,
  oss_bucket,
  oss_real,
};

class CXCLOUD_API OssRequest : public HttpRequest {
  Q_OBJECT
public:
  OssRequest(QObject* parent = nullptr);
  virtual ~OssRequest();

  void callWhenResponsed() override;
  QByteArray getRequestData() const override;
  void setState(OSSState state);

protected:
  virtual QString ossRequest() const;
  virtual void ossSuccess(const QJsonObject& jsonObject, const QByteArray& bytes);
  virtual void onOssReady();
  QString getUrlTail() const override;
  virtual QString ossTail() const;

  QString nullRequest() const;
  bool ossReady();

  bool isAutoDelete() override;

protected:
  QString m_accessKeyId;
  QString m_secretAccessKey;
  QString m_sessionToken;
  QString m_endPoint;
  QString m_fileBucket;
  QString m_prefixPath;
  QString m_internalBucket;
  QString m_internalPrefixPath;

  OSSState m_state;
};

class CXCLOUD_API GcodeUploadRequest : public OssRequest {
  Q_OBJECT
public:
  GcodeUploadRequest(const QString& name, const QString& fileName, QObject* parent = nullptr);
  virtual ~GcodeUploadRequest();

  const QString& getName() const;
  const QString& getFileKey() const;

protected:
  void ossSuccess(const QJsonObject& jsonObject, const QByteArray& bytes) override;
  void onOssReady() override;
  QString ossTail() const override;
protected slots:
  void uploadProgress(int value);
  void uploadResult(bool success);

protected:
  QString m_fileName;
  QString m_name;
  QString m_fileKey;
};

class CXCLOUD_API ModelGroupUploadRequest : public OssRequest {
  Q_OBJECT
public:
  ModelGroupUploadRequest(int categoryId,
                          const QString& groupName,
                          const QString& groupDesc,
                          bool isShare,
                          bool isOriginal,
                          int modelType,
                          const QString& license,
                          int uploadType,
                          QObject* parent = nullptr);
  virtual ~ModelGroupUploadRequest();
  void setModelList(const QStringList& modelList);

  void setDirPath(const QString& path);
  void setSceneNameGetter(std::function<QString(void)>);
  void setMachineNameGetter(std::function<QString(void)>);
  void setCombineSaver(std::function<void(const QString&)>);
  void setUncombineSaver(std::function<void(const QString&, QList<QString>&)>);

protected:
  void ossSuccess(const QJsonObject& jsonObject, const QByteArray& bytes) override;
  void onOssReady() override;
  QString ossRequest() const override;
  QString ossTail() const override;
protected slots:
  void uploadProgress(int value);
  void uploadResult(bool success);
  void uncombineUploadResult(bool success);

protected:
  void processCombine();
  void processCombineLocal();
  void processUncombine();

protected:
  int m_categoryId;
  QString m_groupName;
  QString m_groupDesc;
  bool m_isShare;
  bool m_isOriginal;
  int m_modelType;
  QString m_license;
  int m_uploadType;

  QString m_ossKey;
  QString m_fileName;

  QString m_uncombinePath;
  int m_uncombineCount;
  int m_uploadCount;
  QList<QString> m_uncombineNames;
  QList<QString> m_uncombineOssKeys;
  QList<qint64> m_uncombineFileSizes;

private:
  QStringList m_ModelList;
  QString dir_path_;
  std::function<QString(void)> scene_name_getter_;
  std::function<QString(void)> machine_name_getter_;
  std::function<void(const QString&)> combine_saver_;
  std::function<void(const QString&, QList<QString>&)> uncombine_saver_;
};

}  // namespace cxcloud

#endif  // CXCLOUD_NET_OSS_REQUEST_H_
