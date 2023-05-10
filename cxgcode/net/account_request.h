#pragma once

#ifndef CXCLOUD_NET_ACCOUNT_REQUEST_H_
#define CXCLOUD_NET_ACCOUNT_REQUEST_H_

#include <functional>
#include <atomic>

#include <QtCore/QTimer>

#include "../define.hpp"
#include "../interface.hpp"
#include "http_request.h"

namespace cxcloud {

class CXCLOUD_API LoginByAccountRequest : public HttpRequest {
  Q_OBJECT
public:
  LoginByAccountRequest(const QString& account,
                        const QString& password,
                        const QString& type,
                        QObject* parent = nullptr);
  virtual ~LoginByAccountRequest();

  const UserBaseInfo& getBaseInfo() const;

protected:
  QByteArray getRequestData() const override;
  void callWhenSuccessed() override;
  QString getUrlTail() const override;

protected:
  QString account_;
  QString password_;
  QString type_;

  UserBaseInfo base_info_;
};

class CXCLOUD_API QuickLoginByAccountRequest : public HttpRequest {
  Q_OBJECT
public:
  QuickLoginByAccountRequest(const QString& account,
                              const QString& verifyCode,
                              const QString& phoneAreaCode,
                              QObject* parent = nullptr);
  virtual ~QuickLoginByAccountRequest();

	const UserBaseInfo& getBaseInfo() const;

protected:
  QByteArray getRequestData() const override;
  void callWhenSuccessed() override;
  QString getUrlTail() const override;

protected:
  QString m_account;
  QString m_verifyCode;
  QString m_areaCode;

	UserBaseInfo account_info_;
};

class CXCLOUD_API UserDetailInfoRequest : public HttpRequest {
  Q_OBJECT
public:
  UserDetailInfoRequest(const QString& uid, const QString& token, QObject* parent = nullptr);
  virtual ~UserDetailInfoRequest();

  QString getAccountUid() const override;
  QString getAccountToken() const override;

	const UserDetailInfo& getInfo() const;

protected:
  void callWhenSuccessed() override;
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

private:
  QString uid_;
  QString token_; 
	UserDetailInfo info_;
};

class CXCLOUD_API VerificationCodeRequest : public HttpRequest {
  Q_OBJECT
public:
  VerificationCodeRequest(const QString& phoneNumber, QObject* parent = nullptr);
  virtual ~VerificationCodeRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString m_phoneNumber;
};

enum class CXCLOUD_API QRLoginState {
	qr_query_identical,
	qr_query_mobile_state
};

class CXCLOUD_API QrLoginInfoRequest : public HttpRequest {
  Q_OBJECT
public:
  QrLoginInfoRequest(std::function<void(void)> qrcode_refresher = nullptr,
										 std::function<void(const QString&)> qrimage_refresher = nullptr,
										 std::function<void(const UserBaseInfo&)> userinfo_refresher = nullptr,
										 QObject* parent = nullptr);
  virtual ~QrLoginInfoRequest();

  void setState(QRLoginState state);  // invoke on main thread

  void stopWorking();

protected:
  void callWhenSuccessed() override;
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;
  bool isAutoDelete() override;

protected slots:
  void queryMobileState();

protected:
  std::atomic_bool stoped_;

  QRLoginState m_state;
  QTimer* m_timer;

  QString m_identical;

	std::function<void(void)> qrcode_refresher_;
	std::function<void(const QString&)> qrimage_refresher_;
	std::function<void(const UserBaseInfo&)> userinfo_refresher_;
};

enum class CXCLOUD_API ModelFileType {
	gft_purchased,
	gft_favorited,
	gft_my_upload
};

class CXCLOUD_API GetCloudModelListRequest : public HttpRequest {
  Q_OBJECT
public:
  GetCloudModelListRequest(ModelFileType type,
                            const QString& page,
                            int pageSize,
														QString user_id,
                            QObject* parent = nullptr);
  virtual ~GetCloudModelListRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  QString m_page;
  int m_pageSize;
  ModelFileType m_type;
	QString user_id_;
};

enum class CXCLOUD_API GCodeFileType {
	gft_my_upload,
	gft_cloud
};

class CXCLOUD_API GetGCodeListRequest : public HttpRequest {
  Q_OBJECT
public:
  GetGCodeListRequest(GCodeFileType type, int pageIndex, int pageSize, QObject* parent = nullptr);
  virtual ~GetGCodeListRequest();

protected:
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  int m_pageIndex;
  int m_pageSize;
  GCodeFileType m_type;
};

class CXCLOUD_API GetDeviceListRequest : public HttpRequest {
  Q_OBJECT
public:
  GetDeviceListRequest(
		int pageIndex,
		int pageSize,
		std::function<void(const QVariant&, const QVariant&)> info_updater = nullptr,
		std::function<void(const QVariant&, const QVariant&)> realtime_info_updater = nullptr,
		std::function<void(const QVariant&, const QVariant&)> position_info_updater = nullptr,
		std::function<void(const QVariant&, const QVariant&)> status_updater = nullptr,
		QObject* parent = nullptr);
  virtual ~GetDeviceListRequest();

protected:
  void callWhenSuccessed() override;
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

protected:
  int m_pageIndex;
  int m_pageSize;

	std::function<void(const QVariant&, const QVariant&)> info_updater_;
	std::function<void(const QVariant&, const QVariant&)> realtime_info_updater_;
	std::function<void(const QVariant&, const QVariant&)> position_info_updater_;
	std::function<void(const QVariant&, const QVariant&)> status_updater_;
};

enum class CXCLOUD_API DeviceQueryType {
  dqt_property_status,
  dqt_status,
  dqt_property_iot,
  dqt_property_iot_realtime,
  dqt_property_iot_position
};

struct DeviceInfo {
  QString productKey;
  QString deviceName;
  QString iotId;
  QString tbId;
  int iotType;
};

class CXCLOUD_API QueryDeviceInfoResponse : public HttpRequest {
  Q_OBJECT
public:
  QueryDeviceInfoResponse(
		DeviceQueryType type,
		const DeviceInfo& deviceInfo,
		std::function<void(const QVariant&, const QVariant&)> info_updater = nullptr,
		std::function<void(const QVariant&, const QVariant&)> realtime_info_updater = nullptr,
		std::function<void(const QVariant&, const QVariant&)> position_info_updater = nullptr,
		std::function<void(const QVariant&, const QVariant&)> status_updater = nullptr,
		QObject* parent = nullptr);
  virtual ~QueryDeviceInfoResponse();

  QString getUrl() const override;

protected:
  void callWhenSuccessed() override;
  QByteArray getRequestData() const override;
  QString getUrlTail() const override;

  cpr::Parameters createParameters(const QString& action,
                                   const QString& accessKeyId,
                                   const QString& accessToken);

protected:
  DeviceInfo m_deviceInfo;
  DeviceQueryType m_type;
  bool m_postStage;

  bool use_iot_url_;

	std::function<void(const QVariant&, const QVariant&)> info_updater_;
	std::function<void(const QVariant&, const QVariant&)> realtime_info_updater_;
	std::function<void(const QVariant&, const QVariant&)> position_info_updater_;
	std::function<void(const QVariant&, const QVariant&)> status_updater_;
};

}  // namespace cxcloud

#endif  // CXCLOUD_NET_ACCOUNT_REQUEST_H_
