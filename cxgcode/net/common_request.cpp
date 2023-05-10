#include "common_request.h"

#include <QtCore/QVariant>

namespace cxcloud {

SsoTokenRequest::SsoTokenRequest(QObject* parent) : HttpRequest(parent) {}

SsoTokenRequest::~SsoTokenRequest() {}

QString SsoTokenRequest::getSsoUrl() const {
  return sso_url_;
}

void SsoTokenRequest::callWhenSuccessed() {
  QJsonObject result = getResponseJson().value(QString::fromLatin1("result")).toObject();
  QString token = result.value(QString::fromLatin1("ssoToken")).toString();
  sso_url_ = getUrlHead() + "/my-print?pageType=job&slice-token=" + token;
}

QString SsoTokenRequest::getUrlTail() const {
  return "/api/cxy/account/v2/getSSOToken";
}

CommonRequest::CommonRequest(CommonRequest::Type type, QObject* parent)
    : HttpRequest(parent), type_(type) {}

CommonRequest::~CommonRequest() {}

QString CommonRequest::getUrlTail() const {
  QString url;

  switch (type_) {
    case Type::GET_PRINTER_TYPE_LIST:
      url = "/api/cxy/v2/device/printerTypeList";
      break;
    default:
      break;
  }

  return url;
}

}  // namespace cxcloud
