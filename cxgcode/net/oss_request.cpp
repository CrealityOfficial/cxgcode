#include "oss_request.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <oss_constant.h>
#include <oss_interface.h>

#include <qtusercore/module/systemutil.h>

namespace cxcloud {

void StartOssRequest(QPointer<OssRequest> request) {
  if (!request) {
    return;
  }

  request->setState(OSSState::oss_info);
  HttpPost(request.data());

  request->setState(OSSState::oss_bucket);
  HttpPost(request.data());
}

OssRequest::OssRequest(QObject* parent) : HttpRequest(parent), m_state(OSSState::oss_info) {}

OssRequest::~OssRequest() {}

void OssRequest::callWhenResponsed() {
  handleResponseData();

  QByteArray bytes = getResponseData();
  QJsonObject jsonObject = getResponseJson();

  if (m_state == OSSState::oss_real) {
    ossSuccess(jsonObject, bytes);
    return;
  }

  QJsonObject result = jsonObject.value(QString::fromLatin1("result")).toObject();

  QJsonObject aliyunInfo = result.value(QString::fromLatin1("aliyunInfo")).toObject();
  QJsonObject info = result.value(QString::fromLatin1("info")).toObject();

  if (!aliyunInfo.isEmpty() && info.isEmpty()) {
    m_accessKeyId = aliyunInfo.value(QString::fromLatin1("accessKeyId")).toString();
    m_secretAccessKey = aliyunInfo.value(QString::fromLatin1("secretAccessKey")).toString();
    m_sessionToken = aliyunInfo.value(QString::fromLatin1("sessionToken")).toString();
  }

  if (aliyunInfo.isEmpty() && !info.isEmpty()) {
    QJsonObject file = info.value(QString::fromLatin1("file")).toObject();
    QJsonObject internal = info.value(QString::fromLatin1("internal")).toObject();
    m_endPoint = info.value(QString::fromLatin1("endpoint")).toString();
    m_fileBucket = file.value(QString::fromLatin1("bucket")).toString();
    m_prefixPath = file.value(QString::fromLatin1("prefixPath")).toString();
    m_internalBucket = internal.value(QString::fromLatin1("bucket")).toString();
    m_internalPrefixPath = internal.value(QString::fromLatin1("prefixPath")).toString();
  }

  if (ossReady()) {
    m_state = OSSState::oss_real;
    onOssReady();
  }
}

QByteArray OssRequest::getRequestData() const {
  if (m_state == OSSState::oss_real) return ossRequest().toUtf8();
  return nullRequest().toUtf8();
}

void OssRequest::setState(OSSState state) {
  m_state = state;
}

void OssRequest::ossSuccess(const QJsonObject& jsonObject, const QByteArray& bytes) {}

QString OssRequest::ossRequest() const {
  return "{}";
}

void OssRequest::onOssReady() {}

QString OssRequest::getUrlTail() const {
  if (m_state == OSSState::oss_info)
    return "/api/cxy/account/v2/getAliyunInfo";
  else if (m_state == OSSState::oss_bucket)
    return "/api/cxy/v2/common/getOssInfo";

  return ossTail();
}

QString OssRequest::ossTail() const {
  return "";
}

bool OssRequest::ossReady() {
  return !m_accessKeyId.isEmpty() && !m_secretAccessKey.isEmpty() && !m_sessionToken.isEmpty() &&
         !m_endPoint.isEmpty() && !m_fileBucket.isEmpty() && !m_prefixPath.isEmpty() &&
         !m_internalBucket.isEmpty() && !m_internalPrefixPath.isEmpty();
}

bool OssRequest::isAutoDelete() {
  return false;
}

QString OssRequest::nullRequest() const {
  return "{}";
}

GcodeUploadRequest::GcodeUploadRequest(const QString& name,
                                       const QString& fileName,
                                       QObject* parent)
    : OssRequest(parent), m_fileName(fileName), m_name(name) {}

GcodeUploadRequest::~GcodeUploadRequest() {}

const QString& GcodeUploadRequest::getName() const {
  return m_name;
}

const QString& GcodeUploadRequest::getFileKey() const {
  return m_fileKey;
}

void GcodeUploadRequest::ossSuccess(const QJsonObject& jsonObject, const QByteArray& bytes) {
  deleteLater();
}

void GcodeUploadRequest::uploadProgress(int value) {
  setCurrentProgress(value);
  callAfterProgressUpdated();
}

void GcodeUploadRequest::uploadResult(bool success) {
  success ? callAfterSuccessed() : callAfterErred();
  deleteLater();
}

void GcodeUploadRequest::onOssReady() {
  auto progress = [this](long long upload_size, long long total_size) {
    int value = (int)((upload_size * 1.0f) / (total_size * 1.0f) * 100.0f);
    if (value >= 100) value = 99;
    QMetaObject::invokeMethod(this, "uploadProgress", Qt::QueuedConnection, Q_ARG(int, value));
  };

  auto result = [this](const oss_wrapper::OSSError& error) {
    bool success = !std::strcmp(error.code_.c_str(), "200");
    QMetaObject::invokeMethod(this, "uploadResult", Qt::QueuedConnection, Q_ARG(bool, success));
  };

  oss_wrapper::OSSInterface::initOSSConfig(
    m_accessKeyId, m_secretAccessKey, m_endPoint, m_fileBucket, m_sessionToken);
  m_fileKey = m_prefixPath + "/" + qtuser_core::calculateFileMD5(m_fileName) + ".gcode.gz";
  oss_wrapper::OSSInterface::uploadObject(m_fileKey, m_fileName, progress, result);
}

QString GcodeUploadRequest::ossTail() const {
  return "";
}

ModelGroupUploadRequest::ModelGroupUploadRequest(int categoryId,
                                                 const QString& groupName,
                                                 const QString& groupDesc,
                                                 bool isShare,
                                                 bool isOriginal,
                                                 int modelType,
                                                 const QString& license,
                                                 int uploadType,
                                                 QObject* parent)
    : OssRequest(parent)
    , m_categoryId(categoryId)
    , m_groupName(groupName)
    , m_groupDesc(groupDesc)
    , m_isShare(isShare)
    , m_isOriginal(isOriginal)
    , m_modelType(modelType)
    , m_license(license)
    , m_uploadType(uploadType)
    , m_uncombineCount(0) {}

ModelGroupUploadRequest::~ModelGroupUploadRequest() {}

void ModelGroupUploadRequest::setModelList(const QStringList& modelList) {
  m_ModelList = modelList;
}

void ModelGroupUploadRequest::setDirPath(const QString& dir_path) {
  dir_path_ = dir_path;
}

void ModelGroupUploadRequest::setSceneNameGetter(std::function<QString(void)> scene_name_getter) {
  scene_name_getter_ = scene_name_getter;
}

void ModelGroupUploadRequest::setMachineNameGetter(
  std::function<QString(void)> machine_name_getter) {
  machine_name_getter_ = machine_name_getter;
}

void ModelGroupUploadRequest::setCombineSaver(std::function<void(const QString&)> combine_saver) {
  combine_saver_ = combine_saver;
}

void ModelGroupUploadRequest::setUncombineSaver(
  std::function<void(const QString&, QList<QString>&)> uncombine_saver) {
  uncombine_saver_ = uncombine_saver;
}

void ModelGroupUploadRequest::ossSuccess(const QJsonObject& jsonObject, const QByteArray& bytes) {
  callAfterSuccessed();
}

QString ModelGroupUploadRequest::ossTail() const {
  return "/api/cxy/v3/model/modelGroupCreate";
}

void ModelGroupUploadRequest::processCombine() {
  m_fileName = dir_path_ + scene_name_getter_() + ".stl";
  combine_saver_(m_fileName);

  m_ossKey = m_internalPrefixPath + "/" + qtuser_core::calculateFileMD5(m_fileName) + ".stl";
  oss_wrapper::OSSInterface::initOSSConfig(
    m_accessKeyId, m_secretAccessKey, m_endPoint, m_internalBucket, m_sessionToken);
  auto progress = [this](long long upload_size, long long total_size) {
    int value = (int)((upload_size * 1.0f) / (total_size * 1.0f) * 100.0f);
    if (value >= 100) value = 99;

    setCurrentProgress(value);
    callAfterProgressUpdated();
  };

  auto result = [this](const oss_wrapper::OSSError& error) {
    if (!std::strcmp(error.code_.c_str(), "200")) {
      HttpPost(this);

      if (QFile::exists(m_fileName)) QFile::remove(m_fileName);
    }
  };

  oss_wrapper::OSSInterface::uploadObject(m_ossKey, m_fileName, progress, result);
}

void ModelGroupUploadRequest::processCombineLocal() {
  QString modelPath = m_ModelList.at(0);
  QFileInfo info(modelPath);
  QString modelName = info.fileName();
  m_fileName = dir_path_ + machine_name_getter_() + "-" + modelName;
  // combine_saver_(m_fileName);
  QFile file(modelPath);
  if (file.exists()) {
    // 文件存在
    bool res = file.copy(m_fileName);
    std::ignore = res;
  } else {
    // 文件不存在
  }

  m_ossKey = m_internalPrefixPath + "/" + qtuser_core::calculateFileMD5(m_fileName) + ".stl";
  oss_wrapper::OSSInterface::initOSSConfig(
    m_accessKeyId, m_secretAccessKey, m_endPoint, m_internalBucket, m_sessionToken);
  auto progress = [this](long long upload_size, long long total_size) {
    int value = (int)((upload_size * 1.0f) / (total_size * 1.0f) * 100.0f);
    if (value >= 100) value = 99;
    setCurrentProgress(value);
    callAfterProgressUpdated();
  };

  auto result = [this](const oss_wrapper::OSSError& error) {
    if (!std::strcmp(error.code_.c_str(), "200")) {
      HttpPost(this);

      if (QFile::exists(m_fileName)) QFile::remove(m_fileName);
    }
  };

  oss_wrapper::OSSInterface::uploadObject(m_ossKey, m_fileName, progress, result);
}

void ModelGroupUploadRequest::processUncombine() {
  m_uncombinePath = getCanWriteFolder() + "/uploadModels";
  QDir tempDir;
  if (!tempDir.exists(m_uncombinePath)) tempDir.mkpath(m_uncombinePath);

  QList<QString> saveNames;
  uncombine_saver_(m_uncombinePath, saveNames);

  oss_wrapper::OSSInterface::initOSSConfig(
    m_accessKeyId, m_secretAccessKey, m_endPoint, m_internalBucket, m_sessionToken);

  m_uncombineCount = saveNames.size();
  m_uploadCount = 0;
  m_uncombineNames.clear();
  m_uncombineOssKeys.clear();
  m_uncombineFileSizes.clear();

  auto progress = [this](long long upload_size, long long total_size) {};

  auto result = [this](const oss_wrapper::OSSError& error) {
    bool success = !std::strcmp(error.code_.c_str(), "200");
    QMetaObject::invokeMethod(
      this, "uncombineUploadResult", Qt::QueuedConnection, Q_ARG(bool, success));
  };

  if (m_uncombineCount > 0) {
    for (size_t i = 0; i < m_uncombineCount; i++) {
      const QString& name = saveNames.at(i);
      QString strPath = m_uncombinePath + "/" + name;
      QString fileKey =
        m_internalPrefixPath + "/" + qtuser_core::calculateFileMD5(strPath) + ".stl";

      QFileInfo info(strPath);
      m_uncombineNames.append(strPath);
      m_uncombineOssKeys.append(fileKey);
      m_uncombineFileSizes.append(info.size());

      oss_wrapper::OSSInterface::uploadObject(fileKey, strPath, progress, result);
    }
  }
}

QString ModelGroupUploadRequest::ossRequest() const {
  QString request;
  if (m_uploadType == 1) {
    QFileInfo fileInfo(m_fileName);
    request = QString(
                "{\"groupItem\":{\"categoryId\": %1, \"groupName\": \"%2\", \"groupDesc\": \"%3\", "
                "\"isShared\": %4, \"modelColor\": %5, \"license\": \"%6\", \"isOriginal\": %7, "
                "\"covers\":[]}, \"modelList\":[{\"fileKey\":\"%8\", \"fileName\":\"%9\", "
                "\"fileSize\":%10}]}")
                .arg(m_categoryId)
                .arg(m_groupName)
                .arg(m_groupDesc)
                .arg(m_isShare ? "true" : "false")
                .arg(m_modelType)
                .arg(m_license)
                .arg(m_isOriginal ? "true" : "false")
                .arg(m_ossKey)
                .arg(fileInfo.baseName())
                .arg(fileInfo.size());
  } else if (m_uploadType == 0) {
    int count = m_uncombineNames.size();
    QString listStr = "[";
    for (int i = 0; i < count; i++) {
      QString strTmp = QString("{\"fileKey\":\"%1\", \"fileName\":\"%2\", \"fileSize\":%3},")
                         .arg(m_uncombineOssKeys.at(i))
                         .arg(m_uncombineNames.at(i))
                         .arg(m_uncombineFileSizes.at(i));
      listStr += strTmp;
    }
    listStr.chop(1);
    listStr += "]";

    request = QString(
                "{\"groupItem\":{\"categoryId\": %1, \"groupName\": \"%2\", \"groupDesc\": \"%3\", "
                "\"isShared\": %4, \"modelColor\": %5, \"license\": \"%6\", \"isOriginal\": %7, "
                "\"covers\":[]}, \"modelList\":%8}")
                .arg(m_categoryId)
                .arg(m_groupName)
                .arg(m_groupDesc)
                .arg(m_isShare ? "true" : "false")
                .arg(m_modelType)
                .arg(m_license)
                .arg(m_isOriginal ? "true" : "false")
                .arg(listStr);
  }
  return request;
}

void ModelGroupUploadRequest::onOssReady() {
  if (m_ModelList.size() > 0) {
    processCombineLocal();
  } else {
    if (m_uploadType == 1)  // combination
    {
      processCombine();
    } else if (m_uploadType == 0)  // no combination
    {
      processUncombine();
    }
  }
}

void ModelGroupUploadRequest::uploadProgress(int value) {}

void ModelGroupUploadRequest::uploadResult(bool success) {}

void ModelGroupUploadRequest::uncombineUploadResult(bool success) {
  if (success) {
    ++m_uploadCount;
    int value = ((m_uploadCount * 1.0 / m_uncombineCount * 1.0) * 100);
    if (value >= 100) {
      value = 99;
    }

    setCurrentProgress(value);
    callAfterProgressUpdated();

    if (m_uploadCount == m_uncombineCount) {
      HttpPost(this);

      QDir delDir(m_uncombinePath);
      delDir.removeRecursively();
    }
  }
}

}  // namespace cxcloud
