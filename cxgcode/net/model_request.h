#pragma once

#ifndef CXCLOUD_NET_MODEL_REQUEST_H_
#define CXCLOUD_NET_MODEL_REQUEST_H_

#include <functional>
#include <memory>

#include <QTime>

#include "../interface.hpp"
#include "../model/download_model.h"
#include "http_request.h"

namespace cxcloud {

// create by ServiceCenter

class CXCLOUD_API DeleteModelRequest : public HttpRequest {
  Q_OBJECT;

public:
  DeleteModelRequest(const QString& id, QObject* parent = nullptr);
  virtual ~DeleteModelRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString id_;
};

class CXCLOUD_API CollectModelRequest : public HttpRequest {
  Q_OBJECT;

public:
  CollectModelRequest(const QString& id, bool favorite, QObject* parent = nullptr);
  virtual ~CollectModelRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString id_;
  bool favorite_;
};

class CXCLOUD_API UploadModelRequest : public HttpRequest {
  Q_OBJECT;

public:
  UploadModelRequest(QObject* parent = nullptr);
  virtual ~UploadModelRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;
};

class CXCLOUD_API GetModelCategoriesRequest : public HttpRequest {
  Q_OBJECT;

public:
  GetModelCategoriesRequest(QObject* parent = nullptr);
  virtual ~GetModelCategoriesRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;
};

class CXCLOUD_API CategoryListRequest : public HttpRequest {
  Q_OBJECT;

public:
  CategoryListRequest(int type, QObject* parent = nullptr);
  virtual ~CategoryListRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  int type_;
};

class CXCLOUD_API RecommendModelsForPageRequest : public HttpRequest {
  Q_OBJECT;

public:
  RecommendModelsForPageRequest(int page_index, int page_size, QObject* parent = nullptr);
  virtual ~RecommendModelsForPageRequest();

protected:
  void callWhenSuccessed() override;
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  int page_index_;
  int page_size_;
};

class CXCLOUD_API SearchModelRequest : public HttpRequest {
  Q_OBJECT;

public:
  SearchModelRequest(const QString& keyword,
                     int page_index,
                     int page_size,
                     QObject* parent = nullptr);
  virtual ~SearchModelRequest();

protected:
  void callWhenSuccessed() override;
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString keyword_;
  int page_index_;
  int page_size_;
};

class CXCLOUD_API TypeModelsForPageRequest : public HttpRequest {
  Q_OBJECT;

public:
  TypeModelsForPageRequest(const QString& type_id,
                           int page_index,
                           int page_size,
                           QObject* parent = nullptr);
  virtual ~TypeModelsForPageRequest();

protected:
  void callWhenSuccessed() override;
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString type_id_;
  int page_index_;
  int page_size_;
};

class CXCLOUD_API ModelGroupInfomationRequest : public HttpRequest {
  Q_OBJECT;

public:
  ModelGroupInfomationRequest(const QString& group_id, QObject* parent = nullptr);
  virtual ~ModelGroupInfomationRequest();

protected:
  virtual QByteArray getRequestData() const override final;
  virtual QString getUrlTail() const override final;

protected:
  QString group_id_;
};

class CXCLOUD_API ModelGroupDetailRequest : public HttpRequest {
  Q_OBJECT;

public:
  ModelGroupDetailRequest(const QString& group_id, int count, QObject* parent = nullptr);
  virtual ~ModelGroupDetailRequest();

protected:
  virtual QByteArray getRequestData() const final;
  virtual QString getUrlTail() const final;

protected:
  QString group_id_;
  int count_;
};

// create by DownloadService

class CXCLOUD_API ModelDownloadInfoRequest : public ModelGroupDetailRequest {
  Q_OBJECT;

public:
  explicit ModelDownloadInfoRequest(const QString& group_id,
                                    bool ready_to_download = false,
                                    QObject* parent = nullptr);
  virtual ~ModelDownloadInfoRequest();

public:
  Q_SIGNAL void requestFinished(const QString& group_id);

  bool isReadyToDownload() const;

  bool finished() const;
  bool successed() const;

  void syncInfoToModel(DownloadTaskListModel::ModelInfo& model) const;

protected:
  virtual void callWhenSuccessed() override;
  virtual void callWhenFailed() override;
  virtual bool isAutoDelete() override;

protected:
  bool ready_to_download_{false};
  bool finished_{false};
  bool successed_{false};
};

class CXCLOUD_API DownloadModelRequest : public HttpRequest {
  Q_OBJECT;

public:
  DownloadModelRequest(const QString& group_id,
                       const QString& model_id,
                       const QString& name,
                       QObject* parent = nullptr);
  virtual ~DownloadModelRequest();

public:
  Q_SIGNAL void progressUpdated(const QString& group_id, const QString& model_id);
  Q_SIGNAL void downloadFinished(const QString& group_id, const QString& model_id);

  void syncInfoToModel(DownloadItemListModel::ModelInfo& model) const;

  bool finished() const;
  bool successed() const;

protected:
  virtual void callWhenSuccessed() override;
  virtual void callWhenFailed() override;
  virtual bool isAutoDelete() override;
  virtual QByteArray getRequestData() const override;
  virtual QString getUrlTail() const override;

  virtual QString getDownloadDstPath() const override;
  virtual QString getDownloadSrcUrl() const override;
  virtual void callWhenProgressUpdated() override;

protected:
  QString group_id_{};
  QString model_id_{};
  QString download_path_{};
  QString download_url_{};

  bool finished_{false};
  bool successed_{false};

  uint32_t progress_{};

  QTime time_{};
  int downloaded_{};
  uint32_t speed_{};
};

class CXCLOUD_API UnlimitedDownloadModelRequest : public DownloadModelRequest {
  Q_OBJECT;

public:
  UnlimitedDownloadModelRequest(const QString& group_id,
                                const QString& model_id,
                                const QString& name,
                                QObject* parent = nullptr);
  virtual ~UnlimitedDownloadModelRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;
};

}  // namespace cxcloud

#endif  // CXCLOUD_NET_MODEL_REQUEST_H_
