#include "http_request.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QUuid>
#include <QtNetwork/QNetworkInterface>

#include <buildinfo.h>

#include <qtusercore/module/systemutil.h>

namespace cxcloud {

namespace _intrenel {

inline std::string GetDuid() {
  for (const auto& network_interface : QNetworkInterface::allInterfaces()) {
    const auto flags = network_interface.flags();
    if (flags.testFlag(QNetworkInterface::IsUp) &&
        flags.testFlag(QNetworkInterface::IsRunning) &&
        !flags.testFlag(QNetworkInterface::IsLoopBack)) {
      return network_interface.hardwareAddress().toStdString();
    }
  }

  return {};
}

inline std::string GetUuid() {
  return QUuid::createUuid().toString().toStdString();
}

inline std::string I18nToLangaugeIndex(const std::string& i18n) {
  static const std::map<std::string, std::string> I18N_INDEX_MAP{
    { "en"   , "0" },
    { "zh_cn", "1" },
    { "zh_tw", "2" },
  };

  auto iter = I18N_INDEX_MAP.find(i18n);
  if (iter == I18N_INDEX_MAP.cend()) {
    return "0";
  }

  return iter->second;
}

inline std::string GetLanguageIndex() {
  QSettings setting;
  setting.beginGroup(QStringLiteral("language_perfer_config"));
  std::string i18n = setting.value(QStringLiteral("language_type"))
                            .toString()
                            .toLower()
                            .remove(QStringLiteral(".ts"))
                            .toStdString();
  setting.endGroup();
  return I18nToLangaugeIndex(i18n);
}

inline std::string GetPlatform() {
  return "11";
}

}  // namespace _intrenel

void HttpGet(QPointer<HttpRequest> request) {
  if (!request || !HttpRequest::IsInitlized()) { return; }

  request->feature_ = cpr::GetCallback(
    [request](cpr::Response response) {
      if (!request) { return; }
      request->response_ = response;
      request->setResponseData(QByteArray{response.text.c_str()});
      request->callWhenResponsed();
      request->callAfterResponsed();
    },
    cpr::Url{request->getUrl().toStdString()},
    cpr::Parameters{request->getParameters()},
    cpr::Header{request->getHeader()});
}

void HttpPost(QPointer<HttpRequest> request) {
  if (!request || !HttpRequest::IsInitlized()) { return; }

  request->feature_ = cpr::PostCallback(
    [request](cpr::Response response) {
      if (!request) { return; }
      request->response_ = response;

      if (response.error.code != cpr::ErrorCode::OK) {
        request->setResponseState(response.error.code);
        request->setResponseMessage(QString::fromStdString(response.error.message));
        QMetaObject::invokeMethod(request, [request]() {
          if (!request) { return; }
          request->callWhenErred();
          request->callAfterErred();
        }, Qt::ConnectionType::QueuedConnection);
        return;
      }

      request->setResponseData(QByteArray{response.text.c_str()});
      request->callWhenResponsed();
      request->callAfterResponsed();
    },
    cpr::Url{request->getUrl().toStdString()},
    cpr::Body{request->getRequestData().toStdString()},
    // cpr::VerifySsl{false},
    cpr::Header{request->getHeader()});
}

void HttpDownload(QPointer<HttpRequest> request) {
  if (!request || !HttpRequest::IsInitlized()) { return; }

  QString download_url = request->getDownloadSrcUrl();
  QString download_path = request->getDownloadDstPath();
  if (download_url.isEmpty() || download_path.isEmpty()) { return; }

  mkMutiDir(QFileInfo{download_path}.absolutePath());

  qDebug().noquote() << QStringLiteral(
                          "---------- cxcloud::HttpDownload ----------\nurl: %1\nfile: %2")
                          .arg(download_url)
                          .arg(download_path);

  request->response_ = cpr::Download(
    download_path.toStdString(),
    cpr::Url{download_url.toStdString()},
    cpr::VerifySsl{false},
    cpr::ProgressCallback{[request](cpr::cpr_off_t download_total,
                                    cpr::cpr_off_t download_current,
                                    cpr::cpr_off_t upload_total,
                                    cpr::cpr_off_t upload_current,
                                    intptr_t userdata) -> bool {
      if (!request) {
        return false;
      }

      request->setTotalProgress(download_total);
      request->setCurrentProgress(download_current);
      request->callWhenProgressUpdated();
      request->callAfterProgressUpdated();

      // @see https://docs.libcpr.org/advanced-usage.html#download-file
      // Return `true` on success, or `false` to **cancel** the transfer.
      return !request->isCancelDownloadLater();
    }});

  request->setResponseData(QStringLiteral("{\"code\":\"%1\"}")
                             .arg(QString::number(request->response_.error ? 1 : 0))
                             .toUtf8());
  request->callWhenResponsed();
  request->callAfterResponsed();
}

std::function<QString(void)> HttpRequest::Url_Head_Getter_ = nullptr;
std::function<QString(void)> HttpRequest::Account_Token_Getter_ = nullptr;
std::function<QString(void)> HttpRequest::Account_Uid_Getter_ = nullptr;

void HttpRequest::RegisterUrlHeadGetter(std::function<QString(void)> getter) {
  Url_Head_Getter_ = getter;
}

void HttpRequest::RegisterAccountUidGetter(std::function<QString(void)> getter) {
  Account_Uid_Getter_ = getter;
}

void HttpRequest::RegisterAccountTokenGetter(std::function<QString(void)> getter) {
  Account_Token_Getter_ = getter;
}

bool HttpRequest::IsInitlized() {
  bool initlized = Url_Head_Getter_ != nullptr &&
                   Account_Token_Getter_ != nullptr &&
                   Account_Uid_Getter_ != nullptr;
  assert(initlized && "register getter for HttpRequest first plz.");
  return initlized;
}

HttpRequest::HttpRequest(QObject* parent)
  : QObject(parent)
  , cancel_download_later_(false)
  , total_progress_(100)
  , current_progress_(0) {}

HttpRequest::~HttpRequest() {}

QByteArray HttpRequest::getRequestData() const {
  return QByteArrayLiteral("{}");
}

QString HttpRequest::getDownloadSrcUrl() const {
  return QStringLiteral("");
}

QString HttpRequest::getDownloadDstPath() const {
  return QStringLiteral("");
}

QString HttpRequest::getAccountUid() const {
  if (!Account_Uid_Getter_) { return {}; }
  return Account_Uid_Getter_();
}

QString HttpRequest::getAccountToken() const {
  if (!Account_Token_Getter_) { return {}; }
  return Account_Token_Getter_();
}

cpr::Header HttpRequest::getHeader() const {
  if (!header_.empty()) {
    return header_;
  }

  header_ = {
    { "__CXY_OS_VER_"   , "Win"                         },
    { "__CXY_DUID_"     , _intrenel::GetDuid()          },
    { "__CXY_OS_LANG_"  , _intrenel::GetLanguageIndex() },
    { "__CXY_APP_ID_"   , "creality_model"              },
    { "__CXY_PLATFORM_" , _intrenel::GetPlatform()      },
    { "__CXY_REQUESTID_", _intrenel::GetUuid()          },
    { "__CXY_APP_VER_"  , "4.0.0"                       },
    { "Content-Type"    , "application/json"            },
  };

  using pair_t = cpr::Header::value_type;

  auto token = getAccountToken().toStdString();
  if (!token.empty()) {
    header_.emplace(pair_t{"__CXY_TOKEN_", std::move(token)});
  }

  auto uid = getAccountUid().toStdString();
  if (!uid.empty()) {
    header_.emplace(pair_t{"__CXY_UID_", std::move(uid)});
  }

  return header_;
}

void HttpRequest::setParameters(cpr::Parameters parameters) {
  parameters_ = parameters;
}

cpr::Parameters HttpRequest::getParameters() const {
  return parameters_;
}

QString HttpRequest::getUrlHead() const {
  return Url_Head_Getter_ ? Url_Head_Getter_() : QString{};
}

QString HttpRequest::getUrlTail() const {
  return {};
}

QString HttpRequest::getUrl() const {
  return getUrlHead() + getUrlTail();
}

void HttpRequest::callWhenSuccessed() {}

void HttpRequest::callWhenFailed() {}

void HttpRequest::callWhenErred() {}

void HttpRequest::callWhenProgressUpdated() {}

bool HttpRequest::isAutoDelete() {
  return true;
}

bool HttpRequest::handleResponseData() {
  bool successed{true};
  QByteArray bytes = getResponseData();
  QJsonObject object{};
  QString message{};

  QJsonParseError error;
  const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);
  successed = error.error == QJsonParseError::ParseError::NoError;

  if (!successed) {
    message = error.errorString();

  } else {
    object = document.object();
    successed = object.value(QStringLiteral("code")).toInt() == 0;

    if (!successed) {
      message = object.value(QStringLiteral("msg")).toString();
    }
  }

  setResponseJson(object);
  setResponseMessage(message);

  return successed;
}

void HttpRequest::callWhenResponsed() {
  bool successed = handleResponseData();

  QMetaObject::invokeMethod(this, [this, successed]() {
    successed ? callWhenSuccessed() : callWhenFailed();
    successed ? callAfterSuccessed() : callAfterFailed();
    if (isAutoDelete()) { deleteLater(); }
  }, Qt::ConnectionType::QueuedConnection);
}

size_t HttpRequest::getTotalProgress() const {
  return total_progress_;
}

size_t HttpRequest::getCurrentProgress() const {
  return current_progress_;
}

bool HttpRequest::isCancelDownloadLater() const {
  return cancel_download_later_.load();
}

void HttpRequest::setCancelDownloadLater(bool cancel) {
  cancel_download_later_.store(cancel);
}

const QByteArray& HttpRequest::getResponseData() const {
  return response_data_;
}

const QJsonObject& HttpRequest::getResponseJson() const {
  return response_json_;
}

cpr::ErrorCode HttpRequest::getResponseState() const {
  return response_state_;
}

const QString& HttpRequest::getResponseMessage() const {
  return response_message_;
}

void HttpRequest::setTotalProgress(size_t progress) {
  total_progress_ = progress;
}

void HttpRequest::setCurrentProgress(size_t progress) {
  current_progress_ = progress;
}

void HttpRequest::setResponseData(const QByteArray& data) {
  response_data_ = data;
}

void HttpRequest::setResponseJson(const QJsonObject& json) {
  response_json_ = json;
}

void HttpRequest::setResponseState(const cpr::ErrorCode& state) {
  response_state_ = state;
}

void HttpRequest::setResponseMessage(const QString& message) {
  response_message_ = message;
}

void HttpRequest::setResponsedCallback(std::function<void(void)> callback) {
  after_responsed_callback_ = callback;
}

void HttpRequest::setSuccessedCallback(std::function<void(void)> callback) {
  after_successed_callback_ = callback;
}

void HttpRequest::setFailedCallback(std::function<void(void)> callback) {
  after_failed_callback_ = callback;
}

void HttpRequest::setErredCallback(std::function<void(void)> callback) {
  after_erred_callback_ = callback;
}

void HttpRequest::setProgressUpdatedCallback(std::function<void(void)> callback) {
  after_progress_updated_callback_ = callback;
}

void HttpRequest::callAfterResponsed() {
  if (after_responsed_callback_) { after_responsed_callback_(); }
}

void HttpRequest::callAfterSuccessed() {
  if (after_successed_callback_) { after_successed_callback_(); }
}

void HttpRequest::callAfterFailed() {
  if (after_failed_callback_) { after_failed_callback_(); }
}

void HttpRequest::callAfterErred() {
  if (after_erred_callback_) { after_erred_callback_(); }
}

void HttpRequest::callAfterProgressUpdated() {
  if (after_progress_updated_callback_) { after_progress_updated_callback_(); }
}

}  // namespace cxcloud
