#pragma once

#ifndef CXCLOUD_TOOL_DOWNLOAD_SERVICE_H_
#define CXCLOUD_TOOL_DOWNLOAD_SERVICE_H_

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include <QObject>
#include <QPointer>
#include <QThread>

#include "../interface.hpp"
#include "../model/download_model.h"
#include "../net/model_request.h"

namespace cxcloud {

class CXCLOUD_API DownloadService final : public QObject {
  Q_OBJECT;

public:
  explicit DownloadService(QObject* parent = nullptr);
  virtual ~DownloadService();

public:
  std::function<void(const QStringList&)> getOpenFileHandler() const;
  void setOpenFileHandler(std::function<void(const QStringList&)> handler);

public:  // local cache
  Q_INVOKABLE bool checkModelDownloaded(const QString& group_id, const QString& model_id);
  Q_INVOKABLE bool checkModelGroupDownloaded(const QString& group_id);

  Q_INVOKABLE bool checkModelDownloading(const QString& group_id, const QString& model_id);
  Q_INVOKABLE bool checkModelGroupDownloading(const QString& group_id);

  /// @brief !checkModelDownloaded && !checkModelDownloading
  /// @see checkModelDownloaded
  /// @see checkModelDownloading
  Q_INVOKABLE bool checkModelDownloadable(const QString& group_id, const QString& model_id);
  Q_INVOKABLE bool checkModelGroupDownloadable(const QString& group_id);

  Q_INVOKABLE void deleteModelCache(const QStringList& group_model_list);
  Q_INVOKABLE void deleteModelCache(const QString& group_id, const QString& model_id);
  Q_INVOKABLE void importModelCache(const QStringList& group_model_list);
  Q_INVOKABLE void importModelCache(const QString& group_id, const QString& model_id);
  Q_INVOKABLE bool exportModelCache(const QString& group_id,
                                    const QString& model_id,
                                    const QString& dir_path);

public:  // download promise
  Q_INVOKABLE void makeModelGroupDownloadPromise(const QString& group_id);
  Q_INVOKABLE void makeModelDownloadPromise(const QString& group_id, const QString& model_id);
  Q_INVOKABLE void fulfillsAllDonwloadPromise();
  Q_INVOKABLE void breachAllDonwloadPromise();

public:  // download task
  Q_INVOKABLE void startModelGroupDownloadTask(const QString& group_id);
  Q_INVOKABLE void startModelDownloadTask(const QString& group_id, const QString& model_id);
  Q_INVOKABLE void pauseModelDownloadTask(const QString& group_id, const QString& model_id);
  Q_INVOKABLE void continueModelDownloadTask(const QString& group_id, const QString& model_id);
  Q_INVOKABLE void cancelModelDownloadTask(const QString& group_id, const QString& model_id);

public:
  Q_PROPERTY(QAbstractListModel* downloadTaskModel READ getDownloadTaskModel CONSTANT);
  QAbstractListModel* getDownloadTaskModel() const;

  QString loadModelName(const QString& group_id, const QString& model_id);

private:
  QString loadGroupCachePath(const QString& group_id);
  QString loadModelCachePath(const QString& group_id, const QString& model_id);
  QString loadCacheInfoPath() const;

  void syncCacheFromLocal();
  void syncCacheToLocal() const;
  void clearExpiredCache() const;

  /// @brief check whether there are models in the group whose state in the state list
  /// @param group_id id of group
  /// @param states target state list
  /// @return true if found any model's state is in the state list, or false
  bool checkModelGroupState(const QString& group_id,
                            std::list<DownloadItemListModel::ModelInfo::State> states);

  /// @brief check model's state is in the state list
  /// @param group_id id of model's group
  /// @param model_id id of model
  /// @param states target state list
  /// @return true if model's state is in the state list, or false
  bool checkModelState(const QString& group_id,
                       const QString& model_id,
                       std::list<DownloadItemListModel::ModelInfo::State> states);

  Q_SLOT void onModelGroupInfoRequestFinished(const QString& group_id);
  Q_SLOT void onDownloadProgressUpdated(const QString& group_id, const QString& model_id);
  Q_SLOT void onDownloadFinished(const QString& group_id, const QString& model_id);

private:
  const QString cache_dir_path_;

  std::function<void(const QStringList&)> open_file_handler_;

  struct Promise {
    bool is_group{false};
    QString group_id{};
    QString model_id{};
  };

  std::list<Promise> promise_list_;

  QPointer<QThread> response_thread_;
  std::map<QString, std::unique_ptr<ModelDownloadInfoRequest>> info_request_map_;
  std::map<QString, std::unique_ptr<DownloadModelRequest>> task_request_map_;

  QPointer<DownloadTaskListModel> download_task_model_;
};

}  // namespace cxcloud

#endif  // CXCLOUD_TOOL_DOWNLOAD_SERVICE_H_
