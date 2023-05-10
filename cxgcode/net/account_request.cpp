#include "account_request.h"

#include <QtCore/QTime>
#include <QtCore/QVariant>

namespace cxcloud {

LoginByAccountRequest::LoginByAccountRequest(const QString& account,
                                             const QString& password,
                                             const QString& type,
                                             QObject* parent)
    : HttpRequest(parent)
    , account_(account)
    , password_(password)
    , type_(type) {}

LoginByAccountRequest::~LoginByAccountRequest() {}

const UserBaseInfo& LoginByAccountRequest::getBaseInfo() const {
  return base_info_;
}

QByteArray LoginByAccountRequest::getRequestData() const {
  return QStringLiteral(R"({"type":%1, "account":"%2", "password" :"%3"})")
    .arg(type_).arg(account_).arg(password_).toUtf8();
}

void LoginByAccountRequest::callWhenSuccessed() {
  auto result = getResponseJson().value(QStringLiteral("result")).toObject();
  base_info_.token = result.value(QStringLiteral("token")).toString();
  base_info_.user_id = result.value(QStringLiteral("userId")).toString();
}

QString LoginByAccountRequest::getUrlTail() const {
  return QStringLiteral("/api/account/loginV2");
}





QuickLoginByAccountRequest::QuickLoginByAccountRequest(const QString& account,
                                                       const QString& verifyCode,
                                                       const QString& phoneAreaCode,
                                                       QObject* parent)
    : HttpRequest(parent)
    , m_account(account)
    , m_areaCode(phoneAreaCode)
    , m_verifyCode(verifyCode) {}

QuickLoginByAccountRequest::~QuickLoginByAccountRequest() {}

const UserBaseInfo& QuickLoginByAccountRequest::getBaseInfo() const {
  return account_info_;
}

QByteArray QuickLoginByAccountRequest::getRequestData() const {
  return QStringLiteral(
           "{\"phoneNumber\": \"%1\", \"phoneAreaCode\" : \"%2\", \"verifyCode\" : \"%3\"}")
    .arg(m_account)
    .arg(m_areaCode)
    .arg(m_verifyCode)
    .toUtf8();
}

void QuickLoginByAccountRequest::callWhenSuccessed() {
  auto result = getResponseJson().value(QStringLiteral("result")).toObject();
  account_info_.token = result.value(QStringLiteral("token")).toString();
  account_info_.user_id = result.value(QStringLiteral("userId")).toString();
}

QString QuickLoginByAccountRequest::getUrlTail() const {
  return "/api/account/quickLogin";
}

UserDetailInfoRequest::UserDetailInfoRequest(const QString& uid,
                                             const QString& token,
                                             QObject* parent)
    : HttpRequest(parent), uid_(uid), token_(token) {}

UserDetailInfoRequest::~UserDetailInfoRequest() {}

QString UserDetailInfoRequest::getAccountUid() const {
  if (uid_.isEmpty()) {
    return HttpRequest::getAccountUid();
  }

  return uid_;
}

QString UserDetailInfoRequest::getAccountToken() const {
  if (token_.isEmpty()) {
    return HttpRequest::getAccountToken();
  }

  return token_;
}

const UserDetailInfo& UserDetailInfoRequest::getInfo() const {
  return info_;
}

void UserDetailInfoRequest::callWhenSuccessed() {
  auto result = getResponseJson().value(QStringLiteral("result")).toObject();
  auto userInfoObj = result.value(QStringLiteral("userInfo")).toObject();
  auto userbaseInfoObj = userInfoObj.value(QStringLiteral("base")).toObject();

  info_.avatar = userbaseInfoObj.value(QStringLiteral("avatar")).toString();
  info_.nick_name = userbaseInfoObj.value(QStringLiteral("nickName")).toString();
  info_.max_storage_space = userInfoObj.value(QStringLiteral("maxStorageSpace")).toDouble();
  info_.used_storage_space = userInfoObj.value(QStringLiteral("usedStorageSpace")).toDouble();
}

QByteArray UserDetailInfoRequest::getRequestData() const {
  return QStringLiteral("{}").toUtf8();
}

QString UserDetailInfoRequest::getUrlTail() const {
  return "/api/cxy/v2/user/getInfo";
}





VerificationCodeRequest::VerificationCodeRequest(const QString& phoneNumber, QObject* parent)
    : HttpRequest(parent), m_phoneNumber(phoneNumber) {}

VerificationCodeRequest::~VerificationCodeRequest() {}

QByteArray VerificationCodeRequest::getRequestData() const {
  return QStringLiteral("{\"account\":\"%1\", \"verifyCodeType\" : %2, \"accountType\" : %3}")
    .arg(m_phoneNumber)
    .arg(6)
    .arg(1)
    .toUtf8();
}

QString VerificationCodeRequest::getUrlTail() const {
  return "/api/account/getVerifyCode";
}





QrLoginInfoRequest::QrLoginInfoRequest(
  std::function<void(void)> qrcode_refresher,
  std::function<void(const QString&)> qrimage_refresher,
  std::function<void(const cxcloud::UserBaseInfo&)> userinfo_refresher,
  QObject* parent)
    : HttpRequest(parent)
    , stoped_(false)
    , qrcode_refresher_(qrcode_refresher)
    , qrimage_refresher_(qrimage_refresher)
    , userinfo_refresher_(userinfo_refresher)
    , m_state(QRLoginState::qr_query_identical) {
  m_timer = new QTimer(this);
  m_timer->setSingleShot(false);
  m_timer->setInterval(1000);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(queryMobileState()));
}

QrLoginInfoRequest::~QrLoginInfoRequest() {}

void QrLoginInfoRequest::setState(QRLoginState state) {
  m_state = state;
}

void QrLoginInfoRequest::stopWorking() {
  stoped_ = true;
}

void QrLoginInfoRequest::callWhenSuccessed() {
  QByteArray bytes = getResponseData();
  QJsonObject jsonObject = getResponseJson();

  QJsonObject result = jsonObject.value(QStringLiteral("result")).toObject();
  if (m_state == QRLoginState::qr_query_identical) {
    m_identical = result.value(QStringLiteral("identical")).toString();

    if (!stoped_) { qrimage_refresher_(m_identical); }
    setState(QRLoginState::qr_query_mobile_state);
    m_timer->start();
  } else if (m_state == QRLoginState::qr_query_mobile_state) {
    int state = result.value(QStringLiteral("state")).toInt();
    if (state == 3) {
      UserBaseInfo account;
      account.user_id =
        QString("%1").arg(uint64_t(result.value(QStringLiteral("userId")).toDouble()));
      account.token = result.value(QStringLiteral("token")).toString();
      if (!stoped_) { userinfo_refresher_(account); }

      setState(QRLoginState::qr_query_identical);
      m_timer->stop();
    } else if (state == 4 || state == 5) {
      if (!stoped_) { qrcode_refresher_(); }
      m_timer->stop();
    }
  }
}

QByteArray QrLoginInfoRequest::getRequestData() const {
  if (m_state == QRLoginState::qr_query_mobile_state) {
    return QStringLiteral("{\"identical\": \"%1\"}").arg(m_identical).toUtf8();
  }

  return "{}";
}

QString QrLoginInfoRequest::getUrlTail() const {
  if (m_state == QRLoginState::qr_query_mobile_state) {
    return "/api/cxy/account/v2/qrQuery";
  }

  return "/api/cxy/account/v2/qrLogin";
}

bool QrLoginInfoRequest::isAutoDelete() {
  return false;
}

void QrLoginInfoRequest::queryMobileState() {
  if (m_state == QRLoginState::qr_query_mobile_state && !stoped_) {
    HttpPost(this);
  }
}





GetCloudModelListRequest::GetCloudModelListRequest(ModelFileType type,
                                                     const QString& page,
                                                     int pageSize,
                                                     QString user_id,
                                                     QObject* parent)
    : HttpRequest(parent), m_page(page), m_pageSize(pageSize), m_type(type), user_id_(user_id) {}

GetCloudModelListRequest::~GetCloudModelListRequest() {}

QByteArray GetCloudModelListRequest::getRequestData() const {
  if (m_type == ModelFileType::gft_purchased) {
    return QString("{\"cursor\":\"%1\", \"limit\":%2, \"userId\":%3, \"payStatus\":%4}")
      .arg(m_page)
      .arg(m_pageSize)
      .arg(user_id_)
      .arg(2)
      .toUtf8();
  } else if (m_type == ModelFileType::gft_favorited) {
    return QString("{\"cursor\":\"%1\", \"limit\":%2}").arg(m_page).arg(m_pageSize).toUtf8();
  } else if (m_type == ModelFileType::gft_my_upload) {
    return QString("{\"cursor\":\"%1\", \"limit\":%2}").arg(m_page).arg(m_pageSize).toUtf8();
  }

  return QString("{}").toUtf8();
}

QString GetCloudModelListRequest::getUrlTail() const {
  if (m_type == ModelFileType::gft_purchased)
    return "/api/cxy/v3/model/modelOrderList";
  else if (m_type == ModelFileType::gft_favorited)
    return "/api/cxy/v3/model/listCollect";
  else if (m_type == ModelFileType::gft_my_upload)
    return "/api/cxy/v3/model/listUpload";

  return QString("");
}





GetGCodeListRequest::GetGCodeListRequest(GCodeFileType type,
                                           int pageIndex,
                                           int pageSize,
                                           QObject* parent)
    : HttpRequest(parent), m_type(type), m_pageIndex(pageIndex), m_pageSize(pageSize) {}

GetGCodeListRequest::~GetGCodeListRequest() {}

QByteArray GetGCodeListRequest::getRequestData() const {
  QString upload = m_type == GCodeFileType::gft_cloud ? "false" : "true";
  return QString("{\"page\":%1, \"pageSize\":%2, \"isUpload\":%3}")
    .arg(m_pageIndex)
    .arg(m_pageSize)
    .arg(upload)
    .toUtf8();
}

QString GetGCodeListRequest::getUrlTail() const {
  return "/api/cxy/v2/gcode/ownerList";
}





GetDeviceListRequest::GetDeviceListRequest(
  int pageIndex,
  int pageSize,
  std::function<void(const QVariant&, const QVariant&)> info_updater,
  std::function<void(const QVariant&, const QVariant&)> realtime_info_updater,
  std::function<void(const QVariant&, const QVariant&)> position_info_updater,
  std::function<void(const QVariant&, const QVariant&)> status_updater,
  QObject* parent)
    : HttpRequest(parent)
    , m_pageIndex(pageIndex)
    , m_pageSize(pageSize)
    , info_updater_(info_updater)
    , realtime_info_updater_(realtime_info_updater)
    , position_info_updater_(position_info_updater)
    , status_updater_(status_updater) {}

GetDeviceListRequest::~GetDeviceListRequest() {}

void GetDeviceListRequest::callWhenSuccessed() {
  QJsonObject result = getResponseJson().value(QStringLiteral("result")).toObject();
  int count = result.value("count").toInt();
  if (count == 0) return;

  QList<DeviceInfo> infos;
  QJsonArray objs = result.value("list").toArray();

  for (QJsonValueRef ref : objs) {
    DeviceInfo info;
    QJsonObject object = ref.toObject();

    info.productKey = object.value("productKey").toString();
    info.deviceName = object.value("deviceName").toString();
    info.iotId = object.value("iotId").toString();
    info.tbId = object.value("tbId").toString();
    info.iotType = object.value("iotType").toInt();

    infos.append(info);
  }

  for (const DeviceInfo& info : infos) {
    if (info.iotType == 1) {
      HttpPost(new QueryDeviceInfoResponse(DeviceQueryType::dqt_property_status,
                                           info,
                                           info_updater_,
                                           realtime_info_updater_,
                                           position_info_updater_,
                                           status_updater_,
                                           this->parent()));

      HttpPost(new QueryDeviceInfoResponse(DeviceQueryType::dqt_status,
                                           info,
                                           info_updater_,
                                           realtime_info_updater_,
                                           position_info_updater_,
                                           status_updater_,
                                           this->parent()));
    } else if (info.iotType == 2) {
      {
        auto* request = new QueryDeviceInfoResponse(DeviceQueryType::dqt_property_iot,
                                                    info,
                                                    info_updater_,
                                                    realtime_info_updater_,
                                                    position_info_updater_,
                                                    status_updater_,
                                                    this->parent());
        request->setParameters(cpr::Parameters{
          {"keys",
           "netIP,tfCard,state,active,model,connect,printId,boxVersion,upgradeStatus,upgrade,"
           "bedTemp2,nozzleTemp2,fan,layer,setPosition,video"}});
        HttpGet(request);
      }

      {
        auto* request = new QueryDeviceInfoResponse(DeviceQueryType::dqt_property_iot_realtime,
                                                    info,
                                                    info_updater_,
                                                    realtime_info_updater_,
                                                    position_info_updater_,
                                                    status_updater_,
                                                    this->parent());
        request->setParameters(cpr::Parameters{
          {"keys",
           "nozzleTemp,bedTemp,curFeedratePct,dProgress,printProgress,printJobTime,printLeftTime"},
          {"useStrictDataTypes", "true"}});
        HttpGet(request);
      }

      HttpPost(new QueryDeviceInfoResponse(DeviceQueryType::dqt_property_iot_position,
                                           info,
                                           info_updater_,
                                           realtime_info_updater_,
                                           position_info_updater_,
                                           status_updater_,
                                           this->parent()));
    }
  }
}

QByteArray GetDeviceListRequest::getRequestData() const {
  return QString("{\"page\":%1, \"pageSize\":%2}").arg(m_pageIndex).arg(m_pageSize).toUtf8();
}

QString GetDeviceListRequest::getUrlTail() const {
  return "/api/cxy/v2/device/onwerDeviceList";
}





QueryDeviceInfoResponse::QueryDeviceInfoResponse(
  DeviceQueryType type,
  const DeviceInfo& deviceInfo,
  std::function<void(const QVariant&, const QVariant&)> info_updater,
  std::function<void(const QVariant&, const QVariant&)> realtime_info_updater,
  std::function<void(const QVariant&, const QVariant&)> position_info_updater,
  std::function<void(const QVariant&, const QVariant&)> status_updater,
  QObject* parent)
    : HttpRequest(parent)
    , info_updater_(info_updater)
    , realtime_info_updater_(realtime_info_updater)
    , position_info_updater_(position_info_updater)
    , status_updater_(status_updater)
    , m_deviceInfo(deviceInfo)
    , m_type(type)
    , m_postStage(true)
    , use_iot_url_(false) {}

QueryDeviceInfoResponse::~QueryDeviceInfoResponse() {}

QString QueryDeviceInfoResponse::getUrl() const {
  if (use_iot_url_) {
    return "https://iot.cn-shanghai.aliyuncs.com/";
  }

  return HttpRequest::getUrl();
}

void QueryDeviceInfoResponse::callWhenSuccessed() {
  QByteArray bytes = getResponseData();
  QJsonObject jsonObject = getResponseJson();

  QString data(bytes);
  if (m_postStage) {
    switch (m_type) {
      case DeviceQueryType::dqt_property_status: {
        const QJsonObject& aliyunInfo = jsonObject.value("aliyunInfo").toObject();
        QString accessKeyId = aliyunInfo.value("accessKeyId").toString();
        QString secretAccessKey = aliyunInfo.value("secretAccessKey").toString();
        QString sessionToken = aliyunInfo.value("sessionToken").toString();

        use_iot_url_ = true;
        setParameters(createParameters("QueryDevicePropertyStatus", accessKeyId, sessionToken));
        HttpGet(this);
      } break;
      case DeviceQueryType::dqt_status: {
        const QJsonObject& aliyunInfo = jsonObject.value("aliyunInfo").toObject();
        QString accessKeyId = aliyunInfo.value("accessKeyId").toString();
        QString secretAccessKey = aliyunInfo.value("secretAccessKey").toString();
        QString sessionToken = aliyunInfo.value("sessionToken").toString();

        use_iot_url_ = true;
        setParameters(createParameters("GetDeviceStatus", accessKeyId, sessionToken));
        HttpGet(this);
      } break;
      case DeviceQueryType::dqt_property_iot: {
        // invokeCloudUserInfoFunc("updateDeviceInformation_NewIOT",
        //                         QVariant::fromValue(data),
        //                         QVariant::fromValue(m_deviceInfo.deviceName));
        info_updater_(QVariant::fromValue(data), QVariant::fromValue(m_deviceInfo.deviceName));
      } break;
      case DeviceQueryType::dqt_property_iot_realtime: {
        // invokeCloudUserInfoFunc("updateDeviceInformation_NewIOT_RealTime",
        //                         QVariant::fromValue(data),
        //                         QVariant::fromValue(m_deviceInfo.deviceName));
        realtime_info_updater_(QVariant::fromValue(data),
                               QVariant::fromValue(m_deviceInfo.deviceName));
      } break;
      case DeviceQueryType::dqt_property_iot_position: {
        // invokeCloudUserInfoFunc("updateDeviceInformation_NewIOT_Position",
        //                         QVariant::fromValue(data),
        //                         QVariant::fromValue(m_deviceInfo.deviceName));
        position_info_updater_(QVariant::fromValue(data),
                               QVariant::fromValue(m_deviceInfo.deviceName));
      } break;
      default:
        break;
    }

    m_postStage = false;
  } else {
    switch (m_type) {
      case DeviceQueryType::dqt_property_status: {
        // invokeCloudUserInfoFunc("updateDeviceInformation_NewIOT",
        //                         QVariant::fromValue(data),
        //                         QVariant::fromValue(m_deviceInfo.deviceName));
        info_updater_(QVariant::fromValue(data), QVariant::fromValue(m_deviceInfo.deviceName));
      } break;
      case DeviceQueryType::dqt_status: {
        // invokeCloudUserInfoFunc("updateDeviceStatus",
        //                         QVariant::fromValue(data),
        //                         QVariant::fromValue(m_deviceInfo.deviceName));
        status_updater_(QVariant::fromValue(data), QVariant::fromValue(m_deviceInfo.deviceName));
      } break;
      case DeviceQueryType::dqt_property_iot:
        break;
      case DeviceQueryType::dqt_property_iot_realtime:
        break;
      case DeviceQueryType::dqt_property_iot_position:
        break;
      default:
        break;
    }
  }
}

QByteArray QueryDeviceInfoResponse::getRequestData() const {
  QString request = "{}";
  switch (m_type) {
    case DeviceQueryType::dqt_property_status:
    case DeviceQueryType::dqt_status:
      break;
    case DeviceQueryType::dqt_property_iot:
      break;
    case DeviceQueryType::dqt_property_iot_realtime:
      break;
    case DeviceQueryType::dqt_property_iot_position: {
      request = QString("{\"method\":\"get\",\"params\":{\"ReqPrinterPara\":1}}");
    } break;
    default:
      break;
  }

  return request.toUtf8();
}

QString QueryDeviceInfoResponse::getUrlTail() const {
  QString tail = "";
  switch (m_type) {
    case DeviceQueryType::dqt_property_status:
    case DeviceQueryType::dqt_status: {
      tail = "/api/cxy/account/v2/getAliyunInfo";
      break;
    }
    case DeviceQueryType::dqt_property_iot: {
      tail = "/api/rest/iotrouter/plugins/telemetry/" + m_deviceInfo.tbId + "/values/attributes";
    } break;
    case DeviceQueryType::dqt_property_iot_realtime: {
      tail = "/api/rest/iotrouter/" + m_deviceInfo.tbId + "/values/timeseries";
    } break;
    case DeviceQueryType::dqt_property_iot_position: {
      tail = "/api/rest/iotrouter/rpc/twoway/" + m_deviceInfo.tbId;
      break;
    }
    default:
      break;
  }

  return tail;
}

cpr::Parameters QueryDeviceInfoResponse::createParameters(const QString& action,
                                                          const QString& accessKeyId,
                                                          const QString& accessToken) {
  qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

  cpr::Parameters parameters = cpr::Parameters{};
  parameters.Add(cpr::Parameter{"Action", "QueryDevicePropertyStatus"});
  parameters.Add(cpr::Parameter{"Format", "JSON"});
  parameters.Add(cpr::Parameter{"Version", "2018-01-20"});
  parameters.Add(cpr::Parameter{"SignatureMethod", "HMAC-SHA1"});
  parameters.Add(cpr::Parameter{"SignatureNonce", QString::number(qrand()).toStdString()});
  parameters.Add(cpr::Parameter{"AccessKeyId", accessKeyId.toStdString()});
  parameters.Add(cpr::Parameter{"SecurityToken", accessToken.toStdString()});
  parameters.Add(cpr::Parameter{
    "Timestamp",
    QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddThh:mm:ssZ").toStdString()});
  parameters.Add(cpr::Parameter{"RegionId", "cn-shanghai"});
  parameters.Add(cpr::Parameter{"ProductKey", m_deviceInfo.productKey.toStdString()});
  parameters.Add(cpr::Parameter{"DeviceName", m_deviceInfo.deviceName.toStdString()});
  parameters.Add(cpr::Parameter{"IotId", m_deviceInfo.iotId.toStdString()});
  parameters.Add(cpr::Parameter{"Signature", m_deviceInfo.iotId.toStdString()});
  parameters.Add(cpr::Parameter{"SecurityToken", m_deviceInfo.iotId.toStdString()});
  parameters.Add(cpr::Parameter{"Timestamp", m_deviceInfo.iotId.toStdString()});

  return parameters;
}

}  // namespace cxcloud
