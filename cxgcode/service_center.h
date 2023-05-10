#pragma once

#ifndef CXCLOUD_SERVICE_CENTER_H_
#define CXCLOUD_SERVICE_CENTER_H_

#include <functional>
#include <memory>

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QVariant>

#include "define.hpp"
#include "interface.hpp"
#include "net/account_request.h"
#include "service/download_service.h"
#include "tool/qrcode_image_provider.h"

namespace cxcloud {

class CXCLOUD_API ServiceCenter : public QObject {
  Q_OBJECT;

  ServiceCenter(ServiceCenter&&) = delete;
  ServiceCenter(const ServiceCenter&) = delete;
  ServiceCenter& operator=(ServiceCenter&&) = delete;
  ServiceCenter& operator=(const ServiceCenter&) = delete;

public:
  explicit ServiceCenter(QObject* parent = nullptr);
  virtual ~ServiceCenter();

public:  // for/from kernel
  void initialize();
  void uninitialize();
  void loadUserSettings();

  QPointer<QrcodeImageProvider> getQrcodeImageProvider() const;

  void setOpenUrlHandler(std::function<void(const QString&)> handler);
  void setOpenFileHandler(std::function<void(const QString&)> handler);
  void setOpenFileListHandler(std::function<void(const QStringList&)> handler);
  void setWritablePathCreater(std::function<QString(const QString&, const QString&)> creater);
  void setModelGroupUrlCreater(std::function<QString(const QString&)> creater);
  void setCurrentSceneNameGetter(std::function<QString()> getter);
  void setCurrentMachineNameGetter(std::function<QString()> getter);
  void setCombineSaver(std::function<void(const QString&)> saver);
  void setUncombineSaver(std::function<void(const QString&, QList<QString>&)> saver);

public:  // sub service
  QPointer<DownloadService> getDownloadService() const;
  QObject* getDownloadServiceObject() const;
  Q_PROPERTY(QObject* downloadService READ getDownloadServiceObject CONSTANT);

public:  // about user account
  bool isLogined() const;
  Q_SIGNAL void isLoginedChanged();
  Q_PROPERTY(bool isLogined READ isLogined NOTIFY isLoginedChanged);

  const UserBaseInfo& userBaseInfo() const;
  const UserDetailInfo& userDetailInfo() const;

  QString getToken() const;
  Q_PROPERTY(QString token READ getToken NOTIFY isLoginedChanged);

  QString getUserId() const;
  Q_PROPERTY(QString userId READ getUserId NOTIFY isLoginedChanged);

  QString getNickName() const;
  Q_PROPERTY(QString nickName READ getNickName NOTIFY isLoginedChanged);

  QString getAvatar() const;
  Q_PROPERTY(QString avatar READ getAvatar NOTIFY isLoginedChanged);

  double getUsedStorageSpace() const;
  Q_PROPERTY(double usedStorageSpace READ getUsedStorageSpace NOTIFY isLoginedChanged);

  double getMaxStorageSpace() const;
  Q_PROPERTY(double maxStorageSpace READ getMaxStorageSpace NOTIFY isLoginedChanged);

  Q_INVOKABLE void requestVerifyCode(const QString& phone_number);
  Q_SIGNAL void requestVerifyCodeSuccessed();
  Q_SIGNAL void requestVerifyCodeFailed();

  Q_INVOKABLE void refreshQrcode();
  Q_SIGNAL void qrcodeRefreshed();

  Q_INVOKABLE void loginByMobilePassword(const QString& phone_number,
                                         const QString& area_code,
                                         const QString& password,
                                         bool auto_login);
  Q_INVOKABLE void loginByEmailPassword(const QString& email,
                                        const QString& password,
                                        bool auto_login);
  Q_INVOKABLE void loginByMobileVerifyCode(const QString& phone_number,
                                           const QString& area_code,
                                           const QString& verify_code,
                                           bool auto_login);
  Q_SIGNAL void loginSuccessed();
  Q_SIGNAL void loginFailed(const QVariant& message);

  Q_INVOKABLE void logout();

public:  // about cloud model
  Q_INVOKABLE void loadModelGroupList(const QString& page, int page_size, int type);
  Q_SIGNAL void loadModelGroupListSuccessed(const QVariant& json_string,
                                            const QVariant& page_index,
                                            const QVariant& request_type);

  Q_INVOKABLE void uploadSelectedModels();
  Q_INVOKABLE void uploadLocalModels(const QStringList files);
  Q_SIGNAL void uploadSelectModelsSuccessed(const QVariant& json_string,
                                            const QVariant& is_local_file);
  Q_SIGNAL void uploadSelectModelsFailed();

  Q_INVOKABLE void uploadModelGroup(int category_id,
                                    const QString& group_name,
                                    const QString& group_desc,
                                    bool is_share,
                                    bool is_original,
                                    int model_type,
                                    const QString& license,
                                    int is_combine);
  Q_SIGNAL void uploadModelGroupSuccessed();
  Q_SIGNAL void uploadModelGroupFailed();
  Q_SIGNAL void uploadModelGroupProgressUpdated(const QVariant& progress);

  Q_INVOKABLE void deleteModelGroup(const QString& id);
  Q_SIGNAL void deleteModelGroupSuccessed(const QVariant& id);
  Q_SIGNAL void deleteModelGroupFailed(const QVariant& id);

  Q_INVOKABLE void collectModelGroup(const QString& id, bool collect = true);
  Q_INVOKABLE void uncollectModelGroup(const QString& id);
  Q_SIGNAL void modelGroupCollected(const QVariant& id);
  Q_SIGNAL void modelGroupUncollected(const QVariant& id);

  Q_INVOKABLE void shareModelGroup(const QString& id);
  Q_SIGNAL void shareModelGroupSuccessed();

public:  // about cloud gcode
  Q_INVOKABLE void loadGcodeList(int page_index, int page_size, int type);
  Q_SIGNAL void loadGcodeListSuccessed(const QVariant& json_string, const QVariant& page_index);

  Q_INVOKABLE void uploadGcode(const QString& gcode_name, const QString& gz_name);
  Q_SIGNAL void uploadGcodeSuccessed();
  Q_SIGNAL void uploadGcodeFailed();
  Q_SIGNAL void uploadGcodeProgressUpdated(const QVariant& progress);

  Q_INVOKABLE void importGcode(const QString& id, const QString& name, const QString& link);
  Q_SIGNAL void importGcodeSuccessed();
  Q_SIGNAL void importGcodeFailed();

  Q_INVOKABLE void deleteGCode(const QString& id);
  Q_SIGNAL void deleteGcodeSuccessed(const QVariant& id);
  Q_SIGNAL void deleteGcodeFailed(const QVariant& id);

public:  // about cloud printer
  Q_INVOKABLE void loadPrinterList(int page_index, int page_size);
  Q_SIGNAL void loadPrinterListSuccessed(const QVariant& json_string);
  Q_SIGNAL void printerListInfoBotained(const QVariant& json_string);
  Q_SIGNAL void printerBaseInfoBotained(const QVariant& json_string, const QVariant& printer_name);
  Q_SIGNAL void printerRealtimeInfoBotained(const QVariant& json_string,
                                            const QVariant& printer_name);
  Q_SIGNAL void printerPostitonInfoBotained(const QVariant& json_string,
                                            const QVariant& printer_name);
  Q_SIGNAL void printerStateInfoBotained(const QVariant& json_string, const QVariant& printer_name);

  Q_INVOKABLE void openCloudPrintWebpage();

public:  // about model library
  Q_INVOKABLE void searchModelGroup(const QString& keyword, int page, int size);
  Q_SIGNAL void searchModelGroupSuccessed(const QVariant& json_string, const QVariant& page_index);
  
  Q_INVOKABLE void loadModelGroupCategoryList();
  Q_SIGNAL void loadModelGroupCategoryListSuccessed(const QVariant& json_string);
  
  Q_INVOKABLE void loadHistoryModelGroupList(int page_index, int page_size);
  Q_SIGNAL void loadHistoryModelGroupListSuccessed(const QVariant& json_string);
  Q_INVOKABLE void pushHistoryModelGroup(const QString& json_string);

  Q_INVOKABLE void loadRecommendModelGroupList(int page_index, int page_size);
  Q_SIGNAL void loadRecommendModelGroupListSuccessed(const QVariant& json_string,
                                                     const QVariant& page_index);

  Q_INVOKABLE void loadTypeModelGroupList(const QString& type_id, int page_index, int page_size);
  Q_SIGNAL void loadTypeModelGroupListSuccessed(const QVariant& json_string,
                                                const QVariant& page_index);

  Q_INVOKABLE void loadModelGroupInfo(const QString& group_id);
  Q_SIGNAL void loadModelGroupInfoSuccessed(const QVariant& json_string);
  
  Q_INVOKABLE void loadModelGroupFileListInfo(const QString& group_id, int count);
  Q_SIGNAL void loadModelGroupFileListInfoSuccessed(const QVariant& json_string);

protected:
  void loginByAccountPassword(const QString& account,
                              const QString& password,
                              const QString& type,
                              bool auto_login);
  void loadUserDetailInfo(const UserBaseInfo& base_info);

private:
  QPointer<DownloadService> download_service_;
  QPointer<QrcodeImageProvider> qrcode_image_provider_;
  QPointer<QrLoginInfoRequest> qrcode_login_request_;

  std::function<void(const QString&)> open_url_handler_;
  std::function<void(const QString&)> open_file_handler_;
  std::function<void(const QStringList&)> open_file_list_handler_;
  std::function<QString(const QString&, const QString&)> writable_path_creater_;
  std::function<QString(const QString&)> model_group_url_creater_;
  std::function<QString()> current_scene_name_getter_;
  std::function<QString()> current_machine_name_getter_;
  std::function<void(const QString&)> combine_saver_;
  std::function<void(const QString&, QList<QString>&)> uncombine_saver_;

  UserBaseInfo base_info_;
  UserDetailInfo detail_info_;

  QStringList model_list_;
};

}  // namespace cxcloud

#endif  // CXCLOUD_SERVICE_CENTER_H_
