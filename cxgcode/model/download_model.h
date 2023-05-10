#pragma once

#ifndef CXCLOUD_MODEL_DOWNLOAD_MODEL_H_
#define CXCLOUD_MODEL_DOWNLOAD_MODEL_H_

#include <vector>
#include <memory>

#include <QAbstractListModel>
#include <QString>

#include "../interface.hpp"

namespace cxcloud {

/// @brief Cloud model download task list.
///        Models in the list are come from the same group in general.
/// @see DownloadTaskListModel
class CXCLOUD_API DownloadItemListModel : public QAbstractListModel {
  Q_OBJECT;

public:
  struct ModelInfo {
    enum class State : int {
      UNREADY     = -1,
      READY       = 0,
      DOWNLOADING = 1,
      PAUSED      = 2,
      FAILED      = 3,
      FINISHED    = 4,
    };

    QString  uid     { ""             };
    QString  name    { ""             };
    QString  image   { ""             }; // URL
    size_t   size    { 0              }; // B
    QString  date    { "0000-00-00"   }; // yyyy-MM-dd
    uint32_t speed   { 0              }; // B/s
    State    state   { State::UNREADY };
    uint32_t progress{ 0              }; // 0-100
  };

public:
  explicit DownloadItemListModel(QObject* parent = nullptr);
  explicit DownloadItemListModel(const std::vector<ModelInfo>& items, QObject* parent = nullptr);
  explicit DownloadItemListModel(std::vector<ModelInfo>&& items, QObject* parent = nullptr);
  virtual ~DownloadItemListModel();

public:
  Q_PROPERTY(int downloadingCount READ getDownloadingCount NOTIFY downloadingCountChanged);
  Q_SIGNAL void downloadingCountChanged() const;
  Q_INVOKABLE int getDownloadingCount() const;

  Q_PROPERTY(int finishedCount READ getFinishedCount NOTIFY finishedCountChanged);
  Q_SIGNAL void finishedCountChanged() const;
  Q_INVOKABLE int getFinishedCount() const;

public:
  /// @brief check is there a item in list has the same uid
  /// @param uid item uid
  /// @return true if found, or false
  auto find(const QString& uid) const -> bool;

  /// @brief load a item info
  /// @param uid item uid
  /// @return real info if successed, otherwise the uid will be empty
  auto load(const QString& uid) const -> ModelInfo;

  /// @brief append a new item into model list,
  ///        and do nothing if the item uid already exist
  /// @param info item info
  /// @return false if the item uid already exist, or true
  auto append(const ModelInfo& info) -> bool;
  auto append(ModelInfo&& info) -> bool;

  /// @brief update a item info at model list,
  ///        and do nothing if the item uid not exist
  /// @param info item info
  /// @return false if the item uid not exist, or true
  auto update(const ModelInfo& info) -> bool;
  auto update(ModelInfo&& info) -> bool;

  /// @brief try append if uid not exist, otherwise try update
  /// @param info item info
  auto emplace(const ModelInfo& info) -> void;
  auto emplace(ModelInfo&& info) -> void;

  /// @brief try to remove a info which has the same uid
  /// @param uid item uid
  /// @return false if the item uid not exist, or true
  auto remove(const QString& uid) -> bool;

  /// @brief remove all model
  auto clear() -> void;

  /// @return reference of raw data
  auto rawData() const -> const std::vector<ModelInfo>&;

protected:
  virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual QHash<int, QByteArray> roleNames() const override;

private:
  enum class DataRole : int {
    UID     = Qt::UserRole + 1,
    NAME    ,
    IMAGE   ,
    SIZE    ,
    DATE    ,
    SPEED   ,
    STATE   ,
    PROGRESS,
  };

private:
  std::vector<ModelInfo> items_;

};





/// @brief Cloud model group download task list.
class CXCLOUD_API DownloadTaskListModel : public QAbstractListModel {
  Q_OBJECT;

public:
  struct ModelInfo {
    QString uid                                 { ""      };
    QString name                                { ""      };
    std::shared_ptr<DownloadItemListModel> items{ nullptr };
  };

public:
  explicit DownloadTaskListModel(QObject* parent = nullptr);
  virtual ~DownloadTaskListModel();

public:
  Q_PROPERTY(int downloadingCount READ getDownloadingCount NOTIFY downloadingCountChanged);
  Q_SIGNAL void downloadingCountChanged() const;
  Q_INVOKABLE int getDownloadingCount() const;

  Q_PROPERTY(int finishedCount READ getFinishedCount NOTIFY finishedCountChanged);
  Q_SIGNAL void finishedCountChanged() const;
  Q_INVOKABLE int getFinishedCount() const;

public:
  /// @brief check is there a group in list has the same uid
  /// @param uid group uid
  /// @return true if found, or false
  auto find(const QString& uid) const -> bool;

  /// @brief load a group info
  /// @param uid group uid
  /// @return real info if successed, otherwise the uid will be empty
  auto load(const QString& uid) const -> ModelInfo;

  /// @brief append a new group into model list,
  ///        and do nothing if the group uid already exist
  /// @param info group info
  /// @return false if the group uid already exist, or true
  auto append(const ModelInfo& info) -> bool;
  auto append(ModelInfo&& info) -> bool;

  /// @brief update a group info at model list,
  ///        and do nothing if the group uid not exist
  /// @param info group info
  /// @return false if the group uid not exist, or true
  auto update(const ModelInfo& info) -> bool;
  auto update(ModelInfo&& info) -> bool;

  /// @brief try append if uid not exist, otherwise try update
  /// @param info group info
  auto emplace(const ModelInfo& info) -> void;
  auto emplace(ModelInfo&& info) -> void;

  /// @brief try to remove a info which has the same uid
  /// @param uid group uid
  /// @return false if the group uid not exist, or true
  auto remove(const QString& uid) -> bool;
  
  /// @brief remove all model
  auto clear() -> void;

  /// @return reference of raw data
  auto rawData() const -> const std::vector<ModelInfo>&;

protected:
  virtual int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual QHash<int, QByteArray> roleNames() const override;

private:
  enum class DataRole : int {
    UID  = Qt::UserRole + 1,
    NAME ,
    ITEMS,
  };

private:
  std::vector<ModelInfo> groups_;

};

} // namespace cxcloud

#endif // !CXCLOUD_MODEL_DOWNLOAD_MODEL_H_
