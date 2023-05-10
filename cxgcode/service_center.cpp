#include "service_center.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QVariantList>

#include <qcxutil/quazip/quazipfile.h>

#include <qtusercore/module/systemutil.h>

#include "tool/history_tool.h"
#include "net/common_request.h"
#include "net/gcode_request.h"
#include "net/model_request.h"
#include "net/oss_request.h"

namespace cxcloud {

ServiceCenter::ServiceCenter(QObject* parent)
    : QObject(parent)
    , qrcode_image_provider_(new QrcodeImageProvider)  // qml engine will takes the ownership
    , download_service_(new DownloadService{ this })
    , qrcode_login_request_(new QrLoginInfoRequest{
      [this]() { refreshQrcode(); },
      [this](const QString& identical) {
        qrcode_image_provider_->setIdentical(identical);
        Q_EMIT qrcodeRefreshed();
      },
      [this](const UserBaseInfo& base_info) { loadUserDetailInfo(base_info); },
      this
    }) {}

ServiceCenter::~ServiceCenter() {}

void ServiceCenter::initialize() {}

void ServiceCenter::uninitialize() {
  qrcode_login_request_->stopWorking();
}

void ServiceCenter::loadUserSettings() {
  QSettings setting;
  setting.beginGroup(QStringLiteral("cloud_service"));
  QString token = setting.value(QStringLiteral("token"), {}).toString();
  QString user_id = setting.value(QStringLiteral("userId"), {}).toString();
  bool auto_login = setting.value(QStringLiteral("autoLogin"), 0).toBool();
  setting.endGroup();

  if (!token.isEmpty() && !user_id.isEmpty()) {
    UserBaseInfo info;
    info.token = token;
    info.user_id = user_id;
    if (auto_login) {
      loadUserDetailInfo(info);
    }
  }
}

QPointer<QrcodeImageProvider> ServiceCenter::getQrcodeImageProvider() const {
  return qrcode_image_provider_;
}

void ServiceCenter::setOpenUrlHandler(std::function<void(const QString&)> handler) {
  open_url_handler_ = handler;
}

void ServiceCenter::setOpenFileHandler(std::function<void(const QString&)> handler) {
  open_file_handler_ = handler;
}

void ServiceCenter::setOpenFileListHandler(std::function<void(const QStringList&)> handler) {
  open_file_list_handler_ = handler;
  download_service_->setOpenFileHandler(handler);
}

void ServiceCenter::setWritablePathCreater(
    std::function<QString(const QString&, const QString&)> creater) {
  writable_path_creater_ = creater;
}

void ServiceCenter::setModelGroupUrlCreater(std::function<QString(const QString&)> creater) {
  model_group_url_creater_ = creater;
}

void ServiceCenter::setCurrentSceneNameGetter(std::function<QString()> getter) {
  current_scene_name_getter_ = getter;
}

void ServiceCenter::setCurrentMachineNameGetter(std::function<QString()> getter) {
  current_machine_name_getter_ = getter;
}

void ServiceCenter::setCombineSaver(std::function<void(const QString&)> saver) {
  combine_saver_ = saver;
}

void ServiceCenter::setUncombineSaver(std::function<void(const QString&, QList<QString>&)> saver) {
  uncombine_saver_ = saver;
}

// ----------- sub servive [begin] -----------

QPointer<DownloadService> ServiceCenter::getDownloadService() const {
  return download_service_;
}

QObject* ServiceCenter::getDownloadServiceObject() const {
  return download_service_.data();
}

// ----------- sub servive [end] -----------

// ----------- about user account [begin] -----------

bool ServiceCenter::isLogined() const {
  return !base_info_.token.isEmpty() &&
         !base_info_.user_id.isEmpty();
}

const UserBaseInfo& ServiceCenter::userBaseInfo() const {
  return base_info_;
}

const UserDetailInfo& ServiceCenter::userDetailInfo() const {
  return detail_info_;
}

QString ServiceCenter::getToken() const {
  return base_info_.token;
}

QString ServiceCenter::getUserId() const {
  return base_info_.user_id;
}

QString ServiceCenter::getNickName() const {
  return detail_info_.nick_name;
}

QString ServiceCenter::getAvatar() const {
  return detail_info_.avatar;
}

double ServiceCenter::getUsedStorageSpace() const {
  return detail_info_.used_storage_space;
}

double ServiceCenter::getMaxStorageSpace() const {
  return detail_info_.max_storage_space;
}

void ServiceCenter::requestVerifyCode(const QString& phone_number) {
  QPointer<HttpRequest> request{ new VerificationCodeRequest{ phone_number } };
  request->setSuccessedCallback([this]() { Q_EMIT requestVerifyCodeSuccessed(); });
  request->setFailedCallback([this]() { Q_EMIT requestVerifyCodeFailed(); });
  HttpPost(request);
}

void ServiceCenter::refreshQrcode() {
  qrcode_login_request_->setState(QRLoginState::qr_query_identical);
  HttpPost(qrcode_login_request_.data());
}

void ServiceCenter::loginByMobilePassword(const QString& phone_number,
                                          const QString& area_code,
                                          const QString& password,
                                          bool auto_login) {
  loginByAccountPassword(area_code + phone_number, password, QStringLiteral("1"), auto_login);
}

void ServiceCenter::loginByEmailPassword(const QString& email,
                                         const QString& password,
                                         bool auto_login) {
  loginByAccountPassword(email, password, QStringLiteral("2"), auto_login);
}

void ServiceCenter::loginByMobileVerifyCode(const QString& phone_number,
                                            const QString& area_code,
                                            const QString& verify_code,
                                            bool auto_login) {
  QString account = area_code + phone_number;
  auto* request = new QuickLoginByAccountRequest{account, verify_code, area_code};

  request->setSuccessedCallback(
    [this, request]() { loadUserDetailInfo(request->getBaseInfo()); });

  HttpPost(request);
}

void ServiceCenter::logout() {
  bool need_emit = isLogined();

  base_info_ = UserBaseInfo{}; 
  detail_info_ = UserDetailInfo{};

  if (need_emit) {
    emit isLoginedChanged();
  }
}

void ServiceCenter::loginByAccountPassword(const QString& account,
                                           const QString& password,
                                           const QString& type,
                                           bool auto_login) {
  auto* request = new LoginByAccountRequest{ account, password, type };

  request->setSuccessedCallback([this, request, auto_login]() {
    QSettings setting;
    setting.beginGroup(QStringLiteral("cloud_service"));
    setting.setValue(QStringLiteral("autoLogin"), static_cast<int>(auto_login));
    setting.endGroup();

    loadUserDetailInfo(request->getBaseInfo());
  });

  request->setFailedCallback(
    [this, request]() { Q_EMIT loginFailed(QVariant::fromValue(request->getResponseMessage())); });

  request->setErredCallback([this, request]() {
    if (request->getResponseState() == cpr::ErrorCode::HOST_RESOLUTION_FAILURE) {
      Q_EMIT loginFailed(QVariant::fromValue(QStringLiteral("HOST_RESOLUTION_FAILURE")));
    }
  });

  HttpPost(request);
}

void ServiceCenter::loadUserDetailInfo(const UserBaseInfo& base_info) {
  auto* request = new UserDetailInfoRequest{ base_info.user_id, base_info.token };

  request->setSuccessedCallback([this, request, base_info]() {
    base_info_ = base_info;
    detail_info_ = request->getInfo();

    QSettings setting;
    setting.beginGroup(QStringLiteral("cloud_service"));
    setting.setValue(QStringLiteral("token"), base_info_.token);
    setting.setValue(QStringLiteral("userId"), base_info_.user_id);
    setting.endGroup();

    Q_EMIT isLoginedChanged();
    Q_EMIT loginSuccessed();

    download_service_->fulfillsAllDonwloadPromise();
  });

  HttpPost(request);
}

// ----------- about user account [begin] -----------

// ---------- about cloud model [begin] ----------

void ServiceCenter::loadModelGroupList(const QString& page, int page_size, int type) {
  QPointer<HttpRequest> request{ new GetCloudModelListRequest{
    static_cast<ModelFileType>(type), page, page_size, base_info_.user_id } };

  request->setSuccessedCallback([this, request, page_size, type]() {
    Q_EMIT loadModelGroupListSuccessed(QVariant::fromValue<QString>(request->getResponseData()),
                                   QVariant::fromValue(page_size),
                                   QVariant::fromValue(type));
  });

  HttpPost(request);
}

void ServiceCenter::uploadSelectedModels() {
  QPointer<HttpRequest> request{ new UploadModelRequest };

  request->setSuccessedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT uploadSelectModelsSuccessed(QVariant::fromValue<QString>(request->getResponseData()),
                                       QVariant::fromValue(false));
  });

  request->setFailedCallback([this]() { Q_EMIT uploadSelectModelsFailed(); });

  HttpPost(request);
}

void ServiceCenter::uploadLocalModels(const QStringList files) {
  model_list_ = files;
  uploadSelectedModels();
}

void ServiceCenter::uploadModelGroup(int category_id,
                                     const QString& group_name,
                                     const QString& group_desc,
                                     bool is_share,
                                     bool is_original,
                                     int model_type,
                                     const QString& license,
                                     int is_combine) {
  if (!writable_path_creater_) { return; }

  auto* request = new ModelGroupUploadRequest{
    category_id, group_name, group_desc, is_share, is_original, model_type, license, is_combine };

  request->setModelList(model_list_);
  request->setDirPath(writable_path_creater_(QStringLiteral("/uploadModels/"), QString{}));
  request->setSceneNameGetter(current_scene_name_getter_);
  request->setMachineNameGetter(current_machine_name_getter_);
  request->setCombineSaver(combine_saver_);
  request->setUncombineSaver(uncombine_saver_);
  request->setSuccessedCallback([this]() { Q_EMIT uploadModelGroupSuccessed(); });
  request->setFailedCallback([this]() { Q_EMIT uploadModelGroupFailed(); });
  request->setProgressUpdatedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT uploadModelGroupProgressUpdated(QVariant::fromValue<int>(request->getCurrentProgress()));
  });

  StartOssRequest(request);
}

void ServiceCenter::deleteModelGroup(const QString& id) {
  QPointer<HttpRequest> request{ new DeleteModelRequest{ id } };
  request->setSuccessedCallback([this, id]() { Q_EMIT deleteModelGroupSuccessed(id); });
  request->setFailedCallback([this, id]() { Q_EMIT deleteModelGroupFailed(id); });
  HttpPost(request);
}

void ServiceCenter::collectModelGroup(const QString& id, bool collect) {
  QPointer<HttpRequest> request{ new CollectModelRequest{ id, collect } };
  request->setSuccessedCallback([this, id, collect]() {
    collect ? Q_EMIT modelGroupCollected(QVariant::fromValue(id))
            : Q_EMIT modelGroupUncollected(QVariant::fromValue(id));
    loadModelGroupInfo(id);
  });
  HttpPost(request);
}

void ServiceCenter::uncollectModelGroup(const QString& id) {
  collectModelGroup(id, false);
}

void ServiceCenter::shareModelGroup(const QString& id) {
  if (model_group_url_creater_) {
    qtuser_core::copyString2Clipboard(model_group_url_creater_(id));
  }
  Q_EMIT shareModelGroupSuccessed();
}

// ---------- about cloud model [end] ----------

// ---------- about cloud slice & gcode [begin] ----------

void ServiceCenter::loadGcodeList(int page_index, int page_size, int type) {
  QPointer<HttpRequest> request{
    new GetGCodeListRequest{ static_cast<GCodeFileType>(type), page_index, page_size } };

  request->setSuccessedCallback([this, request, page_index]() {
    if (!request) { return; }
    Q_EMIT loadGcodeListSuccessed(QVariant::fromValue<QString>(request->getResponseData()),
                                   QVariant::fromValue(page_index));
  });

  HttpPost(request);
}

void ServiceCenter::uploadGcode(const QString& gcode_name, const QString& gz_name) {
  auto* request = new GcodeUploadRequest{ gcode_name, gz_name };

  request->setSuccessedCallback([this, name = request->getName(), key = request->getFileKey()]() {
    QPointer<HttpRequest> request{ new UploadGCodeRequest{ name, key } };
    request->setSuccessedCallback([this]() { Q_EMIT uploadGcodeSuccessed(); });
    request->setFailedCallback([this]() { Q_EMIT uploadGcodeFailed(); });
    HttpPost(request);
  });

  request->setProgressUpdatedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT uploadGcodeProgressUpdated(QVariant::fromValue<int>(request->getCurrentProgress()));
  });

  StartOssRequest(request);
}

void ServiceCenter::importGcode(const QString& id, const QString& name, const QString& link) {
  if (!writable_path_creater_) { return; }

  auto* request =
    new DownloadGCodeRequest{writable_path_creater_(QStringLiteral("/myGcodes/"), id), name, link};

  request->setSuccessedCallback([this, request]() {
    if (!request) { return; }

    auto file_path = request->getZipPath();
    if (file_path.isEmpty()) {
      Q_EMIT importGcodeFailed();
      return;
    }

    auto dir_path = request->getFilePath();
    if (!qcxutil::unZipLocalFile(file_path, dir_path)) {
      Q_EMIT importGcodeFailed();
      return;
    }

    if (open_file_handler_) { open_file_handler_(dir_path); }
    Q_EMIT importGcodeSuccessed();
  });

  HttpDownload(request);
}

void ServiceCenter::deleteGCode(const QString& id) {
  QPointer<HttpRequest> request{ new DeleteGCodeRequest{ id } };

  request->setSuccessedCallback(
    [this, id]() { Q_EMIT deleteGcodeSuccessed(QVariant::fromValue(id)); });

  request->setFailedCallback([this, id]() { Q_EMIT deleteGcodeFailed(QVariant::fromValue(id)); });

  HttpPost(request);
}

// ---------- about cloud slice & gcode [end] ----------

// ---------- about cloud printer [begin] ----------

void ServiceCenter::loadPrinterList(int page_index, int page_size) {
  QPointer<HttpRequest> request{nullptr};

  request = new CommonRequest{CommonRequest::Type::GET_PRINTER_TYPE_LIST};
  request->setSuccessedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT loadPrinterListSuccessed(QVariant::fromValue<QString>(request->getResponseData()));
  });
  HttpPost(request);

  request = new GetDeviceListRequest{
    page_index,
    page_size,
    [this](auto&&... args) { printerBaseInfoBotained(std::forward<decltype(args)>(args)...); },
    [this](auto&&... args) { printerRealtimeInfoBotained(std::forward<decltype(args)>(args)...); },
    [this](auto&&... args) { printerPostitonInfoBotained(std::forward<decltype(args)>(args)...); },
    [this](auto&&... args) { printerStateInfoBotained(std::forward<decltype(args)>(args)...); },
  };
  request->setResponsedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT printerListInfoBotained(QVariant::fromValue<QString>(request->getResponseData()));
  });
  HttpPost(request);
}

void ServiceCenter::openCloudPrintWebpage() {
  auto* request = new SsoTokenRequest;
  request->setSuccessedCallback([this, request]() {
    if (!open_url_handler_) { return; }
    open_url_handler_(request->getSsoUrl());
  });
  HttpPost(request);
}

// ---------- about cloud printer [end] ----------

// ---------- about model library [begin] ----------

void ServiceCenter::searchModelGroup(const QString& keyword, int page, int size) {
  QPointer<HttpRequest> request{ new SearchModelRequest{ keyword, page, size } };
  request->setSuccessedCallback([this, request, page]() {
    if (!request) { return; }
    Q_EMIT searchModelGroupSuccessed(QVariant::fromValue<QString>(request->getResponseData()),
                                    QVariant::fromValue(page));
  });
  HttpPost(request);
}

void ServiceCenter::loadModelGroupCategoryList() {
  QPointer<HttpRequest> request{ new GetModelCategoriesRequest };
  request->setSuccessedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT loadModelGroupCategoryListSuccessed(
      QVariant::fromValue<QString>(request->getResponseData()));
  });
  HttpPost(request);
}

void ServiceCenter::loadHistoryModelGroupList(int page_index, int page_size) {
  Q_EMIT loadHistoryModelGroupListSuccessed(
    QVariant::fromValue(LoadModelHistory(page_index, page_size)));
}

void ServiceCenter::loadRecommendModelGroupList(int page_index, int page_size) {
  QPointer<HttpRequest> request{ new RecommendModelsForPageRequest{ page_index, page_size } };

  request->setSuccessedCallback([this, request, page_index]() {
    if (!request) { return; }
    Q_EMIT loadRecommendModelGroupListSuccessed(
      QVariant::fromValue<QString>(request->getResponseData()), QVariant::fromValue(page_index));
  });

  HttpPost(request);
}

void ServiceCenter::loadTypeModelGroupList(const QString& type_id, int page_index, int page_size) {
  QPointer<HttpRequest> request = new TypeModelsForPageRequest{ type_id, page_index, page_size };

  request->setSuccessedCallback([this, request, page_index]() {
    if (!request) { return; }
    Q_EMIT loadTypeModelGroupListSuccessed(QVariant::fromValue<QString>(request->getResponseData()),
                                           QVariant::fromValue(page_index));
  });

  HttpPost(request);
}

void ServiceCenter::pushHistoryModelGroup(const QString& json_string) {
  PushModelHistory(json_string);
}

void ServiceCenter::loadModelGroupInfo(const QString& group_id) {
  QPointer<HttpRequest> request{ new ModelGroupInfomationRequest{ group_id } };

  request->setSuccessedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT loadModelGroupInfoSuccessed(
      QVariant::fromValue<QString>(request->getResponseData()));
  });

  HttpPost(request);
}

void ServiceCenter::loadModelGroupFileListInfo(const QString& group_id, int count) {
  QPointer<HttpRequest> request{ new ModelGroupDetailRequest{ group_id, count } };

  request->setSuccessedCallback([this, request]() {
    if (!request) { return; }
    Q_EMIT loadModelGroupFileListInfoSuccessed(
      QVariant::fromValue<QString>(request->getResponseData()));
  });

  HttpPost(request);
}

// ---------- about model library [end] ----------

}  // namespace cxcloud
