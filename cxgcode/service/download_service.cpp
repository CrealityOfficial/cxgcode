#include "download_service.h"

#include <set>
#include <chrono>
#include <fstream>

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QPointer>
#include <QDirIterator>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include <qtusercore/string/resourcesfinder.h>
#include <qtusercore/module/cxopenandsavefilemanager.h>

namespace cxcloud {

namespace {

using GroupInfo = DownloadTaskListModel::ModelInfo;
using ItemInfo = DownloadItemListModel::ModelInfo;
using ItemState = ItemInfo::State;

} // namespace

DownloadService::DownloadService(QObject* parent)
    : QObject(parent)
    , download_task_model_(QPointer<DownloadTaskListModel>{ new DownloadTaskListModel{ nullptr } })
    , cache_dir_path_(qtuser_core::getOrCreateAppDataLocation(QStringLiteral("../cloudModels")))
    , response_thread_(QPointer<QThread>{ new QThread{ this } }) {
  syncCacheFromLocal();
  response_thread_->start();
}

DownloadService::~DownloadService() {
  syncCacheToLocal();

  response_thread_->quit();
  response_thread_->wait();

  clearExpiredCache();

  if (download_task_model_) {
    download_task_model_->deleteLater();
  }
}

std::function<void(const QStringList&)> DownloadService::getOpenFileHandler() const {
  return open_file_handler_;
}

void DownloadService::setOpenFileHandler(std::function<void(const QStringList&)> handler) {
  open_file_handler_ = handler;
}

bool DownloadService::checkModelDownloaded(const QString& group_id, const QString& model_id) {
  if (!download_task_model_->find(group_id)) {
    return false;
  }

  if (!download_task_model_->load(group_id).items->find(model_id)) {
    return false;
  }

  return checkModelState(group_id, model_id, { ItemState::FINISHED });
}

bool DownloadService::checkModelGroupDownloaded(const QString& group_id) {
  if (!download_task_model_->find(group_id)) {
    return false;
  }

  return !checkModelGroupState(group_id, { ItemState::UNREADY,
                                           ItemState::READY,
                                           ItemState::DOWNLOADING,
                                           ItemState::PAUSED,
                                           ItemState::FAILED, });
}

bool DownloadService::checkModelDownloading(const QString& group_id, const QString& model_id) {
  if (!download_task_model_->find(group_id)) {
    return false;
  }

  if (!download_task_model_->load(group_id).items->find(model_id)) {
    return false;
  }

  return checkModelState(group_id, model_id, { ItemState::DOWNLOADING });
}

bool DownloadService::checkModelGroupDownloading(const QString& group_id) {
  if (!download_task_model_->find(group_id)) {
    return false;
  }

  return !checkModelGroupState(group_id, { ItemState::UNREADY,
                                           ItemState::READY,
                                           ItemState::PAUSED,
                                           ItemState::FAILED, 
                                           ItemState::FINISHED, });
}

bool DownloadService::checkModelDownloadable(const QString& group_id, const QString& model_id) {
  if (!download_task_model_->find(group_id)) {
    return true;
  }

  if (!download_task_model_->load(group_id).items->find(model_id)) {
    return true;
  }

  return checkModelState(group_id, model_id, { ItemState::UNREADY,
                                               ItemState::READY,
                                               ItemState::PAUSED,
                                               ItemState::FAILED, });
}

bool DownloadService::checkModelGroupDownloadable(const QString& group_id) {
  if (!download_task_model_->find(group_id)) {
    return true;
  }

  return checkModelGroupState(group_id, { ItemState::UNREADY,
                                          ItemState::READY,
                                          ItemState::PAUSED,
                                          ItemState::FAILED, });
}

void DownloadService::deleteModelCache(const QStringList& group_model_list) {
  // group_model_list : { "group_id#model_id", ... }
  // @see DownloadManageDialog.qml

  static const auto DELIMITER = QStringLiteral("#");

  bool deleted_any{ false };
  for (const auto& group_model_id : group_model_list) {
    if (!group_model_id.contains(DELIMITER)) {
      continue;
    }

    const auto id_list = group_model_id.split(DELIMITER);
    if (id_list.size() != 2) {
      continue;
    }

    const auto& group_id = id_list.at(0);
    const auto& model_id = id_list.at(1);

    deleteModelCache(group_id, model_id);
    deleted_any = true;
  }

  if (deleted_any) {
    syncCacheToLocal();
  }
}

void DownloadService::deleteModelCache(const QString& group_id, const QString& model_id) {
  if (!download_task_model_->find(group_id)) {
    return;
  }

  const auto group_info = download_task_model_->load(group_id);
  if (!group_info.items->find(model_id)) {
    return;
  }

  auto model_info = group_info.items->load(model_id);
  model_info.state = ItemState::UNREADY;
  group_info.items->update(std::move(model_info));

  QFile{ loadModelCachePath(group_id, model_id) }.remove();

  bool download_any{ false };
  for (const auto& other_model_info : group_info.items->rawData()) {
    if (other_model_info.state == ItemState::FINISHED) {
      download_any = true;
      break;
    }
  }

  if (!download_any) {
    download_task_model_->remove(group_id);
    QDir{ loadGroupCachePath(group_id) }.removeRecursively();
  }
}

void DownloadService::importModelCache(const QStringList& group_model_list) {
  // group_model_list : { "group_id#model_id", ... }
  // @see DownloadManageDialog.qml

  static const auto DELIMITER = QStringLiteral("#");

  QStringList file_list;

  for (const auto& group_model_id : group_model_list) {
    if (!group_model_id.contains(DELIMITER)) {
      continue;
    }

    const auto id_list = group_model_id.split(DELIMITER);
    if (id_list.size() != 2) {
      continue;
    }

    const auto& group_id = id_list.at(0);
    const auto& model_id = id_list.at(1);

    file_list.push_back(loadModelCachePath(group_id, model_id));
  }

  // getKernel()->ioManager()->openWithNames(file_list);
  if (open_file_handler_) { open_file_handler_(file_list); }
}

void DownloadService::importModelCache(const QString& group_id, const QString& model_id) {
  // getKernel()->ioManager()->openWithNames({ loadModelCachePath(group_id, model_id) });
  if (open_file_handler_) { open_file_handler_({ loadModelCachePath(group_id, model_id) }); }
}

bool DownloadService::exportModelCache(const QString& group_id,
                                           const QString& model_id,
                                           const QString& dir_path) {
  if (!download_task_model_->find(group_id)) {
    return false;
  }

  const auto group_info = download_task_model_->load(group_id);
  if (!group_info.items->find(model_id)) {
    return false;
  }

  const auto src_path = loadModelCachePath(group_id, model_id);
  const auto dst_path = QStringLiteral("%1/%2.stl").arg(QString{ dir_path }.remove(0, 8))
                                                   .arg(group_info.items->load(model_id).name);

  return qtuser_core::copyFile(src_path, dst_path, true);
}

void DownloadService::makeModelGroupDownloadPromise(const QString& group_id) {
  promise_list_.emplace_back(Promise{
    true,
    group_id,
  });
}

void DownloadService::makeModelDownloadPromise(const QString& group_id, const QString& model_id) {
  promise_list_.emplace_back(Promise{
    false,
    group_id,
    model_id,
  });
}

void DownloadService::fulfillsAllDonwloadPromise() {
  for (const auto& promise : promise_list_) {
    if (promise.is_group) {
      startModelGroupDownloadTask(promise.group_id);
    } else {
      startModelDownloadTask(promise.group_id, promise.model_id);
    }
  }

  promise_list_.clear();
}

void DownloadService::breachAllDonwloadPromise() {
  promise_list_.clear();
}

void DownloadService::startModelGroupDownloadTask(const QString& group_id) {
  // update model data : init group data if not exist
  if (!download_task_model_->find(group_id)) {
    download_task_model_->append(GroupInfo{
      group_id,
      {},
      std::make_shared<DownloadItemListModel>(),
    });
  }

  auto group_info = download_task_model_->load(group_id);

  // check the group info, and request for it if not got
  if (info_request_map_.find(group_id) == info_request_map_.cend() &&
      group_info.name.isEmpty()) {
    auto request = std::make_unique<ModelDownloadInfoRequest>(group_id, true);

    request->moveToThread(response_thread_);

    connect(request.get(), &ModelDownloadInfoRequest::requestFinished,
            this, &DownloadService::onModelGroupInfoRequestFinished,
            Qt::ConnectionType::QueuedConnection);

    HttpPost(request.get());
    info_request_map_.emplace(std::make_pair(group_id, std::move(request)));

  // if the group info allready got, try to download each model
  } else if (!group_info.items->rawData().empty()) {
    for (const auto& model_info : group_info.items->rawData()) {
      // if the model is downloading or downloaded now, skip it
      if (model_info.state == ItemState::DOWNLOADING ||
          model_info.state == ItemState::FINISHED) {
        continue;
      }

      const auto model_id = model_info.uid;

      // check the model download task, and start it if not started
      if (task_request_map_.find(model_id) == task_request_map_.cend()) {
        auto request = std::make_unique<DownloadModelRequest>(
          group_id, model_id, loadModelCachePath(group_id, model_id));
    
        request->moveToThread(response_thread_);

        connect(request.get(), &DownloadModelRequest::progressUpdated,
                this, &DownloadService::onDownloadProgressUpdated,
                Qt::ConnectionType::QueuedConnection);
    
        connect(request.get(), &DownloadModelRequest::downloadFinished,
                this, &DownloadService::onDownloadFinished,
                Qt::ConnectionType::QueuedConnection);
    
        HttpPost(request.get());
        task_request_map_.emplace(std::make_pair(model_id, std::move(request)));
      }
    }
  }
}

void DownloadService::startModelDownloadTask(const QString& group_id,
                                                 const QString& model_id) {
  // update model data : init group data if not exist
  if (!download_task_model_->find(group_id)) {
    download_task_model_->append(GroupInfo{
      group_id,
      {},
      std::make_shared<DownloadItemListModel>(),
    });
  }

  // update model data : set the model's state to ItemState::READY
  auto group_info = download_task_model_->load(group_id);
  if (!group_info.items->find(model_id)) {
    group_info.items->append(ItemInfo{
      model_id,
      {},
      {},
      0,
      {},
      0,
      ItemState::UNREADY,
      0,
    });

  } else {
    // if it's downloading or downloaded now, skip it
    auto model_info = group_info.items->load(model_id);
    if (model_info.state == ItemState::DOWNLOADING || 
        model_info.state == ItemState::FINISHED) {
      return;
    }
  }

  // check the group info, and request for it if not got
  if (info_request_map_.find(group_id) == info_request_map_.cend() &&
      group_info.name.isEmpty()) {
    auto request = std::make_unique<ModelDownloadInfoRequest>(group_id);

    request->moveToThread(response_thread_);

    connect(request.get(), &ModelDownloadInfoRequest::requestFinished,
            this, &DownloadService::onModelGroupInfoRequestFinished,
            Qt::ConnectionType::QueuedConnection);

    HttpPost(request.get());
    info_request_map_.emplace(std::make_pair(group_id, std::move(request)));
  }

  // check the model download task, and start it if not started
  if (task_request_map_.find(model_id) == task_request_map_.cend()) {
    auto request = std::make_unique<UnlimitedDownloadModelRequest>(
      group_id, model_id, loadModelCachePath(group_id, model_id));
    
    request->moveToThread(response_thread_);

    connect(request.get(), &DownloadModelRequest::progressUpdated,
            this, &DownloadService::onDownloadProgressUpdated,
            Qt::ConnectionType::QueuedConnection);
    
    connect(request.get(), &DownloadModelRequest::downloadFinished,
            this, &DownloadService::onDownloadFinished,
            Qt::ConnectionType::QueuedConnection);

    HttpPost(request.get());
    task_request_map_.emplace(std::make_pair(model_id, std::move(request)));
  }
}

void DownloadService::pauseModelDownloadTask(const QString& group_id,
                                                 const QString& model_id) {
  // todo
}

void DownloadService::continueModelDownloadTask(const QString& group_id,
                                                    const QString& model_id) {
  // todo
}

void DownloadService::cancelModelDownloadTask(const QString& group_id,
                                                  const QString& model_id) {
  const auto iter = task_request_map_.find(model_id);
  if (iter == task_request_map_.cend() || !iter->second || iter->second->finished()) {
    return;
  }

  iter->second->setCancelDownloadLater(true);
}

QAbstractListModel* DownloadService::getDownloadTaskModel() const {
  return download_task_model_.data();
}

QString DownloadService::loadModelName(const QString& group_id, const QString& model_id) {
  if (!download_task_model_->find(group_id)) {
    return {};
  }

  const auto group_info = download_task_model_->load(group_id);
  if (!group_info.items->find(model_id)) {
    return {};
  }

  return group_info.items->load(model_id).name;
}

QString DownloadService::loadGroupCachePath(const QString& group_id) {
  return QStringLiteral("%1/%2/").arg(cache_dir_path_).arg(group_id);
}

QString DownloadService::loadModelCachePath(const QString& group_id, const QString& model_id) {
  return QStringLiteral("%1/%2/%3.stl").arg(cache_dir_path_).arg(group_id).arg(model_id);
}

QString DownloadService::loadCacheInfoPath() const {
  return QStringLiteral("%1/cache_info.json").arg(cache_dir_path_);
}

void DownloadService::syncCacheFromLocal() {
  // read file data, parse json format

  std::ifstream ifstream{ loadCacheInfoPath().toStdString(), std::ios::in | std::ios::binary };
  rapidjson::IStreamWrapper wrapper{ ifstream };
  rapidjson::Document document;
  if (document.ParseStream(wrapper).HasParseError()) {
    return;
  }

  // reset model

  download_task_model_->clear();
  
  // read info from local cache to model

  if (!document.HasMember("group_list") || !document["group_list"].IsArray()) {
    return;
  }

  for (const auto& group : document["group_list"].GetArray()) {
    if (!group.IsObject()) {
      continue;
    }

    if (!group.HasMember("id") || !group["id"].IsString() ||
        !group.HasMember("name") || !group["name"].IsString() ||
        !group.HasMember("model_list") || !group["model_list"].IsArray()) {
      continue;
    }

    // group info

    const auto download_item_model = std::make_shared<DownloadItemListModel>();
    download_task_model_->append({
      group["id"].GetString(),
      group["name"].GetString(),
      download_item_model
    });

    // models info in group

    for (const auto& model : group["model_list"].GetArray()) {
      if (!model.IsObject()) {
        continue;
      }

      if (!model.HasMember("id") || !model["id"].IsString() ||
          !model.HasMember("name") || !model["name"].IsString() ||
          !model.HasMember("image") || !model["image"].IsString() ||
          !model.HasMember("size") || !model["size"].IsUint() ||
          !model.HasMember("date") || !model["date"].IsString() ||
          !model.HasMember("downloaded") || !model["downloaded"].IsBool()) {
        continue;
      }

      download_item_model->append({
        model["id"].GetString(),
        model["name"].GetString(),
        model["image"].GetString(),
        model["size"].GetUint(),
        model["downloaded"].GetBool() ? model["date"].GetString() : QString{},
        0,
        model["downloaded"].GetBool() ? ItemState::FINISHED : ItemState::UNREADY,
        0
      });
    }
  }
}

void DownloadService::syncCacheToLocal() const {
  // read info from model to json

  rapidjson::Document document;
  document.SetObject();

  auto& allocator = document.GetAllocator();

  rapidjson::Value group_list;
  group_list.SetArray();

  for (const auto& group_info : download_task_model_->rawData()) {
    const auto& model_info_list = group_info.items->rawData();
    if (model_info_list.empty()) {
      continue;
    }

    // if none of model has been downloaded in this group, skip it

    bool downloaded_any{ false };
    for (const auto& model_info : model_info_list) {
      if (model_info.state == ItemState::FINISHED) {
        downloaded_any = true;
        break;
      }
    }

    if (!downloaded_any) {
      continue;
    }

    // group info

    rapidjson::Value group;
    group.SetObject();

    rapidjson::Value group_id;
    group_id.SetString(group_info.uid.toUtf8(), allocator);
    group.AddMember("id", std::move(group_id), allocator);

    rapidjson::Value group_name;
    group_name.SetString(group_info.name.toUtf8(), allocator);
    group.AddMember("name", std::move(group_name), allocator);

    rapidjson::Value model_list;
    model_list.SetArray();

    for (const auto& model_info : model_info_list) {
      // models info in group

      rapidjson::Value model;
      model.SetObject();

      rapidjson::Value model_id;
      model_id.SetString(model_info.uid.toUtf8(), allocator);
      model.AddMember("id", std::move(model_id), allocator);

      rapidjson::Value model_name;
      model_name.SetString(model_info.name.toUtf8(), allocator);
      model.AddMember("name", std::move(model_name), allocator);

      rapidjson::Value model_image;
      model_image.SetString(model_info.image.toUtf8(), allocator);
      model.AddMember("image", std::move(model_image), allocator);

      rapidjson::Value model_size;
      model_size.SetUint(model_info.size);
      model.AddMember("size", std::move(model_size), allocator);

      rapidjson::Value model_date;
      model_date.SetString(model_info.date.toUtf8(), allocator);
      model.AddMember("date", std::move(model_date), allocator);

      rapidjson::Value model_downloaded;
      model_downloaded.SetBool(model_info.state == ItemState::FINISHED);
      model.AddMember("downloaded", std::move(model_downloaded), allocator);

      model_list.PushBack(std::move(model), allocator);
    }

    group.AddMember("model_list", std::move(model_list), allocator);

    group_list.PushBack(std::move(group), allocator);
  }
  
  document.AddMember("group_list", group_list, allocator);

  // write file data

  std::ofstream ofstream{ loadCacheInfoPath().toStdString(), std::ios::out | std::ios::binary };
  rapidjson::OStreamWrapper wrapper{ ofstream };
  rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer{ wrapper };

#ifdef QT_DEBUG
  writer.SetIndent(' ', 2);
#endif // QT_DEBUG

  document.Accept(writer);
}

void DownloadService::clearExpiredCache() const {
  // find uids of all downloaded model 
  std::map<QString, std::set<QString>> downloaded_group_models_map;
  for (const auto& group_info : download_task_model_->rawData()) {
    std::set<QString> downloaded_model_set;
    for (const auto& model_info : group_info.items->rawData()) {
      if (model_info.state == ItemState::FINISHED) {
        downloaded_model_set.emplace(model_info.uid);
      }
    }

    if (!downloaded_model_set.empty()) {
      downloaded_group_models_map.emplace(
        std::make_pair(group_info.uid, std::move(downloaded_model_set)));
    }
  }

  // delete the cache dir if none of model has been downloaded
  if (downloaded_group_models_map.empty()) {
    QDir{ cache_dir_path_ }.removeRecursively();
    return;
  }

  // walk the cache dir and delete the expired cache
  const auto cache_info_path = QFileInfo{ loadCacheInfoPath() };
  const auto filter = QDir::Filter::Dirs
                    | QDir::Filter::Files
                    | QDir::Filter::NoDotAndDotDot
                    | QDir::Filter::NoSymLinks;
  QDirIterator dir_iter{ cache_dir_path_, filter, QDirIterator::IteratorFlag::Subdirectories };
  while (dir_iter.hasNext()) {
    dir_iter.next();
    
    const auto file_info = dir_iter.fileInfo();
    if (!file_info.isWritable()) {
      continue;
    }

    const auto is_dir = file_info.isDir();
    const auto dir_name = file_info.dir().dirName();
    const auto dir_path = file_info.absolutePath();

    const auto is_file = !is_dir;
    const auto file_name = file_info.baseName();  // without the suffix
    const auto file_path = file_info.absoluteFilePath();

    if (is_file && cache_info_path == QFileInfo{ file_path }) {
      continue;
    }

    auto map_iter = downloaded_group_models_map.find(is_dir ? file_name : dir_name);
    if (map_iter == downloaded_group_models_map.cend()) {
      is_dir ? QDir{ file_info.absoluteFilePath() }.removeRecursively()
             : QFile{ file_info.absoluteFilePath() }.remove();
      continue;
    }

    const auto set_iter = map_iter->second.find(file_name);
    if (is_file && set_iter == map_iter->second.cend()) {
      QFile{ file_info.absoluteFilePath() }.remove();
      continue;
    }
  }
}

bool DownloadService::checkModelGroupState(const QString& group_id,
                                               std::list<ItemState> states) {
  if (group_id.isEmpty() || states.empty()) {
    return false;
  }

  if (!download_task_model_->find(group_id)) {
    return false;
  }

  const auto group_info = download_task_model_->load(group_id);
  for (const auto& model_info : group_info.items->rawData()) {
    for (const auto& state : states) {
      if (model_info.state == state) {
        return true;
      }
    }
  }

  return false;
}

bool DownloadService::checkModelState(const QString& group_id,
                                          const QString& model_id,
                                          std::list<ItemState> states) {
  if (states.empty() || group_id.isEmpty() || model_id.isEmpty()) {
    return false;
  }
  
  if (!download_task_model_->find(group_id)) {
    return false;
  }

  const auto group_info = download_task_model_->load(group_id);
  if (!group_info.items->find(model_id)) {
    return false;
  }

  const auto item_state = group_info.items->load(model_id).state;

  for (const auto& state : states) {
    if (item_state == state) {
      return true;
    }
  }

  return false;
}

void DownloadService::onModelGroupInfoRequestFinished(const QString& group_id) {
  const auto iter = info_request_map_.find(group_id);
  if (iter == info_request_map_.cend() || !download_task_model_->find(group_id)) {
    return;
  }

  if (!iter->second || !iter->second->finished() || !iter->second->successed()) {
    download_task_model_->remove(group_id);
    return;
  }

  auto model = download_task_model_->load(group_id);
  iter->second->syncInfoToModel(model);
  download_task_model_->update(model);

  if (iter->second->isReadyToDownload()) {
    for (const auto& item : model.items->rawData()) {
      const auto& model_id = item.uid;

      if (task_request_map_.find(model_id) != task_request_map_.cend()) {
        continue;
      }

      auto request = std::make_unique<UnlimitedDownloadModelRequest>(
        group_id, model_id, loadModelCachePath(group_id, model_id));

      request->moveToThread(response_thread_);

      connect(request.get(), &DownloadModelRequest::progressUpdated,
              this, &DownloadService::onDownloadProgressUpdated,
              Qt::ConnectionType::QueuedConnection);

      connect(request.get(), &DownloadModelRequest::downloadFinished,
              this, &DownloadService::onDownloadFinished,
              Qt::ConnectionType::QueuedConnection);

      HttpPost(request.get());
      task_request_map_.emplace(std::make_pair(model_id, std::move(request)));
    }
  }

  info_request_map_.erase(iter);
}

void DownloadService::onDownloadProgressUpdated(const QString& group_id,
                                                    const QString& model_id) {
  const auto iter = task_request_map_.find(model_id);
  if (iter == task_request_map_.cend() || !download_task_model_->find(group_id)) {
    return;
  }

  auto items = download_task_model_->load(group_id).items;
  if (!items->find(model_id)) {
    return;
  }

  auto model = items->load(model_id);
  iter->second->syncInfoToModel(model);
  items->update(std::move(model));
}

void DownloadService::onDownloadFinished(const QString& group_id, const QString& model_id) {
  task_request_map_.erase(model_id);

  syncCacheToLocal();
}

} // namespace cxcloud
