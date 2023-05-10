#pragma once

#ifndef CXCLOUD_NET_HTTP_REQUEST_H_
#define CXCLOUD_NET_HTTP_REQUEST_H_

#include <atomic>
#include <functional>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QPointer>

#include <cpr/cpr.h>
#include <cpr/cprtypes.h>

#include "../interface.hpp"

namespace cxcloud {

class HttpRequest;

CXCLOUD_API void HttpGet(QPointer<HttpRequest> request);
CXCLOUD_API void HttpPost(QPointer<HttpRequest> request);
CXCLOUD_API void HttpDownload(QPointer<HttpRequest> request);

class CXCLOUD_API HttpRequest : public QObject {
  Q_OBJECT;

  HttpRequest(HttpRequest&&) = delete;
  HttpRequest(const HttpRequest&) = delete;
  HttpRequest& operator=(HttpRequest&&) = delete;
  HttpRequest& operator=(const HttpRequest&) = delete;

  friend CXCLOUD_API void HttpGet(QPointer<HttpRequest>);
  friend CXCLOUD_API void HttpPost(QPointer<HttpRequest>);
  friend CXCLOUD_API void HttpDownload(QPointer<HttpRequest>);

public:
  static void RegisterUrlHeadGetter(std::function<QString(void)> getter);
  static void RegisterAccountUidGetter(std::function<QString(void)> getter);
  static void RegisterAccountTokenGetter(std::function<QString(void)> getter);

  static bool IsInitlized();

public:
  explicit HttpRequest(QObject* parent = nullptr);
  virtual ~HttpRequest();

  virtual QByteArray getRequestData() const;
  virtual QString getDownloadSrcUrl() const;
  virtual QString getDownloadDstPath() const;

  virtual QString getAccountUid() const;
  virtual QString getAccountToken() const;
  virtual cpr::Header getHeader() const;

  void setParameters(cpr::Parameters Parameters);
  cpr::Parameters getParameters() const;

  virtual QString getUrlHead() const;
  virtual QString getUrlTail() const;
  virtual QString getUrl() const;

  size_t getTotalProgress() const;
  size_t getCurrentProgress() const;
  bool isCancelDownloadLater() const;
  void setCancelDownloadLater(bool cancel);

  const QByteArray& getResponseData() const;
  const QJsonObject& getResponseJson() const;
  cpr::ErrorCode getResponseState() const;
  const QString& getResponseMessage() const;

public:
  void setResponsedCallback(std::function<void(void)> callback);
  void setSuccessedCallback(std::function<void(void)> callback);
  void setFailedCallback(std::function<void(void)> callback);
  void setErredCallback(std::function<void(void)> callback);
  void setProgressUpdatedCallback(std::function<void(void)> callback);

protected:
  virtual bool isAutoDelete();

  virtual bool handleResponseData();

  void setTotalProgress(size_t progress);
  void setCurrentProgress(size_t progress);

  void setResponseData(const QByteArray& data);
  void setResponseJson(const QJsonObject& json);
  void setResponseState(const cpr::ErrorCode& state);
  void setResponseMessage(const QString& message);

protected:
  virtual void callWhenResponsed();
  virtual void callWhenSuccessed();
  virtual void callWhenFailed();
  virtual void callWhenErred();
  virtual void callWhenProgressUpdated();

  void callAfterResponsed();
  void callAfterSuccessed();
  void callAfterFailed();
  void callAfterErred();
  void callAfterProgressUpdated();

private:
  static std::function<QString(void)> Url_Head_Getter_;
  static std::function<QString(void)> Account_Uid_Getter_;
  static std::function<QString(void)> Account_Token_Getter_;

private:
  mutable cpr::Header header_;
  cpr::Parameters parameters_;
  std::future<void> feature_;
  cpr::Response response_;

  std::atomic<bool> cancel_download_later_;
  size_t total_progress_;
  size_t current_progress_;

  QByteArray response_data_;
  QJsonObject response_json_; 
  cpr::ErrorCode response_state_;
  QString response_message_;

  std::function<void(void)> after_responsed_callback_;
  std::function<void(void)> after_successed_callback_;
  std::function<void(void)> after_failed_callback_;
  std::function<void(void)> after_erred_callback_;
  std::function<void(void)> after_progress_updated_callback_;
};

}  // namespace cxcloud

#endif  // CXCLOUD_NET_HTTP_REQUEST_H_
