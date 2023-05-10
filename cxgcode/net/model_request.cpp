#include "model_request.h"

#include <QCryptographicHash>
#include <QDate>
#include <QVariant>

namespace cxcloud {

DeleteModelRequest::DeleteModelRequest(const QString& id, QObject* parent)
    : HttpRequest(parent), id_(id) {}

DeleteModelRequest::~DeleteModelRequest() {}

QByteArray DeleteModelRequest::getRequestData() const {
  return QStringLiteral(R"({"id":"%1"})").arg(id_).toUtf8();
}

QString DeleteModelRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/modelGroupDelete");
}

CollectModelRequest::CollectModelRequest(const QString& id, bool favorite, QObject* parent)
    : HttpRequest(parent), id_(id), favorite_(favorite) {}

CollectModelRequest::~CollectModelRequest() {}

QByteArray CollectModelRequest::getRequestData() const {
  QString str = favorite_ ? QStringLiteral("true") : QStringLiteral("false");
  return QStringLiteral(R"({"id":"%1", "action":%2})").arg(id_).arg(str).toUtf8();
}

QString CollectModelRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/modelGroupCollection");
}

UploadModelRequest::UploadModelRequest(QObject* parent) : HttpRequest(parent) {}

UploadModelRequest::~UploadModelRequest() {}

QByteArray UploadModelRequest::getRequestData() const {
  return QStringLiteral(R"({"type":%1})").arg(7).toUtf8();
}

QString UploadModelRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v2/common/categoryList");
}

GetModelCategoriesRequest::GetModelCategoriesRequest(QObject* parent)
    : HttpRequest(parent) {}

GetModelCategoriesRequest::~GetModelCategoriesRequest() {}

QByteArray GetModelCategoriesRequest::getRequestData() const {
  return QStringLiteral(R"({"type":%1})").arg(7).toUtf8();
}

QString GetModelCategoriesRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v2/common/categoryList");
}

CategoryListRequest::CategoryListRequest(int type, QObject* parent)
    : HttpRequest(parent), type_(type) {}

CategoryListRequest::~CategoryListRequest() {}

QByteArray CategoryListRequest::getRequestData() const {
  return QStringLiteral(R"({"type":%1})").arg(type_).toUtf8();
}

QString CategoryListRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v2/common/categoryList");
}

namespace {

std::map<QString, QString> Group_Id_Name_Cache_Map{};

}  // namespace

RecommendModelsForPageRequest::RecommendModelsForPageRequest(int page_index,
                                                             int page_size,
                                                             QObject* parent)
    : HttpRequest(parent), page_index_(page_index), page_size_(page_size) {}

RecommendModelsForPageRequest::~RecommendModelsForPageRequest() {}

void RecommendModelsForPageRequest::callWhenSuccessed() {
  const auto result = getResponseJson().value(QStringLiteral("result")).toObject();
  const auto group_list = result.value(QStringLiteral("list")).toArray();
  if (!result.empty() && !group_list.empty()) {
    for (const auto& group_value : group_list) {
      const auto group_info = group_value.toObject();
      const auto model_info = group_info.value(QStringLiteral("model")).toObject();
      auto group_id = model_info.value(QStringLiteral("id")).toString();
      auto group_name = model_info.value(QStringLiteral("groupName")).toString();
      Group_Id_Name_Cache_Map.emplace(std::make_pair(std::move(group_id), std::move(group_name)));
    }
  }
}

QByteArray RecommendModelsForPageRequest::getRequestData() const {
  QString page_index_str = page_index_ > 0 ? QString::number(page_index_) : "";
  return QStringLiteral(R"({"cursor":"%1", "limit":%2, "excludeAd":true})")
    .arg(page_index_str)
    .arg(page_size_)
    .toUtf8();
}

QString RecommendModelsForPageRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/recommend");
}

SearchModelRequest::SearchModelRequest(const QString& keyword,
                                       int page_index,
                                       int page_size,
                                       QObject* parent)
    : HttpRequest(parent)
    , keyword_(keyword)
    , page_index_(page_index)
    , page_size_(page_size) {}

SearchModelRequest::~SearchModelRequest() {}

void SearchModelRequest::callWhenSuccessed() {
  const auto result = getResponseJson().value(QStringLiteral("result")).toObject();
  const auto group_list = result.value(QStringLiteral("list")).toArray();
  if (!result.empty() && !group_list.empty()) {
    for (const auto& group_value : group_list) {
      const auto group_info = group_value.toObject();
      auto group_id = group_info.value(QStringLiteral("id")).toString();
      auto group_name = group_info.value(QStringLiteral("groupName")).toString();
      Group_Id_Name_Cache_Map.emplace(std::make_pair(std::move(group_id), std::move(group_name)));
    }
  }
}

QByteArray SearchModelRequest::getRequestData() const {
  return QStringLiteral(R"({"page":%1, "pageSize":%2, "keyword":"%3"})")
    .arg(page_index_)
    .arg(page_size_)
    .arg(keyword_)
    .toUtf8();
}

QString SearchModelRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/search/model");
}

TypeModelsForPageRequest::TypeModelsForPageRequest(const QString& type_id,
                                                   int page_index,
                                                   int page_size,
                                                   QObject* parent)
    : HttpRequest(parent)
    , type_id_(type_id)
    , page_index_(page_index)
    , page_size_(page_size) {}

TypeModelsForPageRequest::~TypeModelsForPageRequest() {}

void TypeModelsForPageRequest::callWhenSuccessed() {
  const auto result = getResponseJson().value(QStringLiteral("result")).toObject();
  const auto group_list = result.value(QStringLiteral("list")).toArray();
  if (!result.empty() && !group_list.empty()) {
    for (const auto& group_value : group_list) {
      const auto group_info = group_value.toObject();
      auto group_id = group_info.value(QStringLiteral("id")).toString();
      auto group_name = group_info.value(QStringLiteral("groupName")).toString();
      Group_Id_Name_Cache_Map.emplace(std::make_pair(std::move(group_id), std::move(group_name)));
    }
  }
}

QByteArray TypeModelsForPageRequest::getRequestData() const {
  return QStringLiteral(
           R"({"cursor":"%1","limit":%2, "categoryId":"%3", "filterType":%4, "isPay":%5})")
    .arg(page_index_)
    .arg(page_size_)
    .arg(type_id_)
    .arg(2)
    .arg(2 /*0: all; 1: pay;2: free*/)
    .toUtf8();
}

QString TypeModelsForPageRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/listCategory");
}

ModelGroupInfomationRequest::ModelGroupInfomationRequest(const QString& group_id, QObject* parent)
    : HttpRequest(parent), group_id_(group_id) {}

ModelGroupInfomationRequest::~ModelGroupInfomationRequest() {}

QByteArray ModelGroupInfomationRequest::getRequestData() const {
  return QStringLiteral(R"({"id":"%1"})").arg(group_id_).toUtf8();
}

QString ModelGroupInfomationRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/modelGroupInfo");
}

ModelGroupDetailRequest::ModelGroupDetailRequest(const QString& group_id,
                                                 int count,
                                                 QObject* parent)
    : HttpRequest(parent), group_id_(group_id), count_(count) {}

ModelGroupDetailRequest::~ModelGroupDetailRequest() {}

QByteArray ModelGroupDetailRequest::getRequestData() const {
  return QStringLiteral(R"({"cursor":"%1", "limit":%2, "modelId":"%3"})")
    .arg("")
    .arg(count_)
    .arg(group_id_)
    .toUtf8();
}

QString ModelGroupDetailRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/fileList");
}

namespace {

using GroupInfo = DownloadTaskListModel::ModelInfo;
using ItemInfo = DownloadItemListModel::ModelInfo;
using ItemState = ItemInfo::State;

}  // namespace

ModelDownloadInfoRequest::ModelDownloadInfoRequest(const QString& group_id,
                                                   bool ready_to_download,
                                                   QObject* parent)
    : ModelGroupDetailRequest(group_id, 100, parent), ready_to_download_(ready_to_download) {}

ModelDownloadInfoRequest::~ModelDownloadInfoRequest() {}

void ModelDownloadInfoRequest::syncInfoToModel(DownloadTaskListModel::ModelInfo& group) const {
  if (!finished() || !successed() || getResponseJson().empty()) {
    return;
  }

  const auto iter = Group_Id_Name_Cache_Map.find(group_id_);
  if (iter != Group_Id_Name_Cache_Map.cend()) {
    group.name = iter->second;
  }

  const auto result = getResponseJson().value(QStringLiteral("result")).toObject();
  const auto model_list = result.value(QStringLiteral("list")).toArray();
  if (result.empty() || model_list.empty()) {
    return;
  }

  for (const auto& model_info_ref : model_list) {
    if (!model_info_ref.isObject()) {
      continue;
    }

    const auto model_info = model_info_ref.toObject();

    auto model_id = model_info.value(QStringLiteral("id")).toString();
    auto model_name = model_info.value(QStringLiteral("fileName")).toString();
    auto model_image = model_info.value(QStringLiteral("coverUrl")).toString();
    auto model_size = model_info.value(QStringLiteral("fileSize")).toInt(0);

    if (group.items->find(model_id)) {
      auto model = group.items->load(model_id);
      model.name = std::move(model_name);
      model.image = std::move(model_image);
      model.size = std::move(model_size);
      if (model.state != ItemState::DOWNLOADING) {
        model.state = ItemState::READY;
      }
      group.items->update(std::move(model));
    } else {
      group.items->append({
        std::move(model_id),
        std::move(model_name),
        std::move(model_image),
        std::move(static_cast<size_t>(model_size)),
        {},
        0,
        ItemState::READY,
        0,
      });
    }
  }
}

bool ModelDownloadInfoRequest::isReadyToDownload() const {
  return ready_to_download_;
}

bool ModelDownloadInfoRequest::finished() const {
  return finished_;
}

bool ModelDownloadInfoRequest::successed() const {
  return successed_;
}

void ModelDownloadInfoRequest::callWhenSuccessed() {
  finished_ = true;
  successed_ = true;
  Q_EMIT requestFinished(group_id_);
}

void ModelDownloadInfoRequest::callWhenFailed() {
  finished_ = true;
  Q_EMIT requestFinished(group_id_);
}

bool ModelDownloadInfoRequest::isAutoDelete() {
  return false;
}

DownloadModelRequest::DownloadModelRequest(const QString& group_id,
                                           const QString& model_id,
                                           const QString& download_path,
                                           QObject* parent)
    : HttpRequest(parent)
    , group_id_(group_id)
    , model_id_(model_id)
    , download_path_(download_path)
    , time_(QTime::currentTime()) {}

DownloadModelRequest::~DownloadModelRequest() {}

void DownloadModelRequest::syncInfoToModel(DownloadItemListModel::ModelInfo& model) const {
  if (finished_) {
    model.state = isCancelDownloadLater() ? ItemState::READY
                  : successed()           ? ItemState::FINISHED
                                          : ItemState::FAILED;
    model.date = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
  } else {
    model.state = ItemState::DOWNLOADING;
  }

  model.progress = progress_;
  model.speed = speed_;
}

bool DownloadModelRequest::finished() const {
  return finished_;
}

bool DownloadModelRequest::successed() const {
  return successed_;
}

void DownloadModelRequest::callWhenSuccessed() {
  if (download_url_.isEmpty()) {
    const auto result = getResponseJson().value(QStringLiteral("result")).toObject();
    download_url_ = result.value(QStringLiteral("signUrl")).toString();
    HttpDownload(this);
    return;
  }

  finished_ = true;
  successed_ = true;
  progress_ = 100;
  speed_ = 0;
  Q_EMIT progressUpdated(group_id_, model_id_);
  Q_EMIT downloadFinished(group_id_, model_id_);
}

void DownloadModelRequest::callWhenFailed() {
  if (download_url_.isEmpty()) {
    return;
  }

  finished_ = true;
  progress_ = 0;
  speed_ = 0;
  Q_EMIT progressUpdated(group_id_, model_id_);
  Q_EMIT downloadFinished(group_id_, model_id_);
}

bool DownloadModelRequest::isAutoDelete() {
  return false;
}

QByteArray DownloadModelRequest::getRequestData() const {
  return QStringLiteral(R"({"id":"%1"})").arg(model_id_).toUtf8();
}

QString DownloadModelRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/fileDownload");
}

QString DownloadModelRequest::getDownloadDstPath() const {
  return download_path_;
}

QString DownloadModelRequest::getDownloadSrcUrl() const {
  return download_url_;
}

void DownloadModelRequest::callWhenProgressUpdated() {
  auto current = QTime::currentTime();
  auto duration = time_.msecsTo(current);

  auto total = getTotalProgress();
  auto downloaded = getCurrentProgress();

  // update ui every 500 msecs
  if (duration <= 500) { return; }

  double progress = static_cast<double>(downloaded) / static_cast<double>(total) * 100.;
  progress_ = (std::max)(0., (std::min)(progress, 100.));

  speed_ = (downloaded - downloaded_) / duration;
  time_ = std::move(current);
  downloaded_ = std::move(downloaded);

  Q_EMIT progressUpdated(group_id_, model_id_);
}

UnlimitedDownloadModelRequest::UnlimitedDownloadModelRequest(const QString& group_id,
                                                             const QString& model_id,
                                                             const QString& download_path,
                                                             QObject* parent)
    : DownloadModelRequest(group_id, model_id, download_path, parent) {}

UnlimitedDownloadModelRequest::~UnlimitedDownloadModelRequest() {}

QByteArray UnlimitedDownloadModelRequest::getRequestData() const {
  const auto header = getHeader();

  const auto sign = QStringLiteral("%1%2%3")
                      .arg(QString::fromStdString(header.at("__CXY_PLATFORM_")))
                      .arg(QString::fromStdString(header.at("__CXY_REQUESTID_")))
                      .arg(QStringLiteral("CdbbRe3eE79cAf0dL5b1Icb7T799Y218223be01c"));

  const auto encoded_sign =
    QCryptographicHash::hash(sign.toUtf8(), QCryptographicHash::Sha256).toHex();

  return QStringLiteral(R"({"id":"%1", "sign":"%2"})")
    .arg(model_id_)
    .arg(QString{encoded_sign})
    .toUtf8();
}

QString UnlimitedDownloadModelRequest::getUrlTail() const {
  return QStringLiteral("/api/cxy/v3/model/downloadUnlimited");
}

}  // namespace cxcloud
