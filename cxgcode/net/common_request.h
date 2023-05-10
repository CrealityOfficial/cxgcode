#pragma once

#ifndef CXCLOUD_NET_COMMON_REQUEST_H_
#define CXCLOUD_NET_COMMON_REQUEST_H_

#include "../interface.hpp"
#include "http_request.h"

namespace cxcloud {

class CXCLOUD_API SsoTokenRequest : public HttpRequest {
  Q_OBJECT;

public:
  SsoTokenRequest(QObject* parent = nullptr);
  virtual ~SsoTokenRequest();

  QString getSsoUrl() const;

protected:
  void callWhenSuccessed() override;
  QString getUrlTail() const override;

private:
  QString sso_url_;
};

class CXCLOUD_API CommonRequest : public HttpRequest {
  Q_OBJECT;

public:
  enum class Type {
    GET_PRINTER_TYPE_LIST,
  };

public:
  CommonRequest(Type type, QObject* parent = nullptr);
  virtual ~CommonRequest();

protected:
  QString getUrlTail() const override;

protected:
  Type type_;
};

}  // namespace cxcloud

#endif  // CXCLOUD_NET_COMMON_REQUEST_H_
