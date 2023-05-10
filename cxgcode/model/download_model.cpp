#include "download_model.h"

#include <tuple>

namespace cxcloud {

DownloadItemListModel::DownloadItemListModel(QObject* parent) : QAbstractListModel(parent) {}

DownloadItemListModel::DownloadItemListModel(const std::vector<ModelInfo>& items, QObject* parent)
    : QAbstractListModel(parent)
    , items_(items) {}

DownloadItemListModel::DownloadItemListModel(std::vector<ModelInfo>&& items, QObject* parent)
    : QAbstractListModel(parent)
    , items_(std::move(items)) {}

DownloadItemListModel::~DownloadItemListModel() {}

int DownloadItemListModel::getDownloadingCount() const {
  int count{ 0 };

  for (const auto& item : items_) {
    if (item.state == ModelInfo::State::DOWNLOADING) {
      ++count;
    }
  }

  return count;
}

int DownloadItemListModel::getFinishedCount() const {
  int count{ 0 };

  for (const auto& item : items_) {
    if (item.state == ModelInfo::State::FINISHED) {
      ++count;
    }
  }

  return count;
}

auto DownloadItemListModel::find(const QString& uid) const -> bool {
  for (const auto& item : items_) {
    if (item.uid != uid) {
      continue;
    }

    return true;
  }

  return false;
}

auto DownloadItemListModel::load(const QString& uid) const -> ModelInfo {
  for (const auto& item : items_) {
    if (item.uid != uid) {
      continue;
    }

    return item;
  }

  return {};
}

auto DownloadItemListModel::append(const ModelInfo& info) -> bool {
  if (find(info.uid)) {
    return false;
  }

  std::function<void()> signal_sender = []()->void {};

  switch (info.state) {
    case ModelInfo::State::DOWNLOADING:
      signal_sender = [this]() {
        Q_EMIT downloadingCountChanged();
      };
      break;
    case ModelInfo::State::FINISHED:
      signal_sender = [this]() {
        Q_EMIT finishedCountChanged();
      };
      break;
    default:
      break;
  }

  const auto index = std::distance(items_.cbegin(), items_.cend());
  beginInsertRows(QModelIndex{}, index, index);
  items_.emplace_back(info);
  endInsertRows();

  signal_sender();

  return true;
}

auto DownloadItemListModel::append(ModelInfo&& info) -> bool {
  if (find(info.uid)) {
    return false;
  }

  std::function<void()> signal_sender = []()->void {};

  switch (info.state) {
    case ModelInfo::State::DOWNLOADING:
      signal_sender = [this]() {
        Q_EMIT downloadingCountChanged();
      };
      break;
    case ModelInfo::State::FINISHED:
      signal_sender = [this]() {
        Q_EMIT finishedCountChanged();
      };
      break;
    default:
      break;
  }

  const auto index = std::distance(items_.cbegin(), items_.cend());
  beginInsertRows(QModelIndex{}, index, index);
  items_.emplace_back(std::move(info));
  endInsertRows();

  signal_sender();

  return true;
}

auto DownloadItemListModel::update(const ModelInfo& info) -> bool {
  for (auto iter = items_.begin(); iter != items_.end(); ++iter) {
    if (iter->uid != info.uid) {
      continue;
    }

    auto signal_sender_list{ std::list<std::function<void()>>{
        [this, index = std::distance(items_.begin(), iter)]() {
          const auto model_index = createIndex(index, index);
          Q_EMIT dataChanged(model_index, model_index);
        },
      }
    };

    constexpr auto DOWNLOADING{ ModelInfo::State::DOWNLOADING };
    if ((iter->state == DOWNLOADING && info.state != DOWNLOADING) ||
        (iter->state != DOWNLOADING && info.state == DOWNLOADING)) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT downloadingCountChanged();
      });
    }

    constexpr auto FINISHED{ ModelInfo::State::FINISHED };
    if ((iter->state == FINISHED && info.state != FINISHED) ||
        (iter->state != FINISHED && info.state == FINISHED)) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT finishedCountChanged();
      });
    }

    *iter = info;

    for (const auto& signal_sender : signal_sender_list) {
      signal_sender();
    }

    return true;
  }

  return false;
}

auto DownloadItemListModel::update(ModelInfo&& info) -> bool {
  for (auto iter = items_.begin(); iter != items_.end(); ++iter) {
    if (iter->uid != info.uid) {
      continue;
    }

    auto signal_sender_list{ std::list<std::function<void()>>{
        [this, index = std::distance(items_.begin(), iter)]() {
          const auto model_index = createIndex(index, index);
          Q_EMIT dataChanged(model_index, model_index);
        },
      }
    };
    
    constexpr auto DOWNLOADING{ ModelInfo::State::DOWNLOADING };
    if ((iter->state == DOWNLOADING && info.state != DOWNLOADING) ||
        (iter->state != DOWNLOADING && info.state == DOWNLOADING)) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT downloadingCountChanged();
      });
    }

    constexpr auto FINISHED{ ModelInfo::State::FINISHED };
    if ((iter->state == FINISHED && info.state != FINISHED) ||
        (iter->state != FINISHED && info.state == FINISHED)) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT finishedCountChanged();
      });
    }

    *iter = std::move(info);

    for (const auto& signal_sender : signal_sender_list) {
      signal_sender();
    }

    return true;
  }

  return false;
}

auto DownloadItemListModel::emplace(const ModelInfo& info) -> void {
  if (find(info.uid)) {
    update(info);
  } else {
    append(info);
  }
}

auto DownloadItemListModel::emplace(ModelInfo&& info) -> void {
  if (find(info.uid)) {
    update(std::move(info));
  } else {
    append(std::move(info));
  }
}

auto DownloadItemListModel::remove(const QString& uid) -> bool {
  for (auto iter = items_.cbegin(); iter != items_.cend(); ++iter) {
    if (iter->uid != uid) {
      continue;
    }

    std::function<void()> signal_sender = []()->void {};

    switch (iter->state) {
      case ModelInfo::State::DOWNLOADING:
        signal_sender = [this]() {
          Q_EMIT downloadingCountChanged();
        };
        break;
      case ModelInfo::State::FINISHED:
        signal_sender = [this]() {
          Q_EMIT finishedCountChanged();
        };
        break;
      default:
        break;
    }
    
    const auto index = std::distance(items_.cbegin(), iter);
    beginRemoveRows(QModelIndex{}, index, index);
    items_.erase(iter);
    endRemoveRows();

    signal_sender();
    return true;
  }

  return false;
}

auto DownloadItemListModel::clear() -> void {
  beginResetModel();
  items_.clear();
  endResetModel();
}

auto DownloadItemListModel::rawData() const -> const std::vector<ModelInfo>& {
  return items_;
}

int DownloadItemListModel::rowCount(const QModelIndex& parent) const {
  std::ignore = parent;
  return items_.size();
}

QVariant DownloadItemListModel::data(const QModelIndex& index, int role) const {
  if (index.row() < 0 || index.row() >= items_.size()) {
    return {};
  }

  QVariant result;
  const auto& item = items_.at(index.row());
  switch (static_cast<DataRole>(role)) {
    case DataRole::UID:
      result = QVariant::fromValue(item.uid);
      break;
    case DataRole::NAME:
      result = QVariant::fromValue(item.name);
      break;
    case DataRole::IMAGE:
      result = QVariant::fromValue(item.image);
      break;
    case DataRole::SIZE:
      result = QVariant::fromValue(item.size);
      break;
    case DataRole::DATE:
      result = QVariant::fromValue(item.date);
      break;
    case DataRole::SPEED:
      result = QVariant::fromValue(item.speed);
      break;
    case DataRole::STATE:
      result = QVariant::fromValue(static_cast<int>(item.state));
      break;
    case DataRole::PROGRESS:
      result = QVariant::fromValue(item.progress);
      break;
    default:
      break;
  }

  return result;
}

QHash<int, QByteArray> DownloadItemListModel::roleNames() const {
  static const QHash<int, QByteArray> role_names{
    { static_cast<int>(DataRole::UID     ), QByteArrayLiteral("model_uid"     ) },
    { static_cast<int>(DataRole::NAME    ), QByteArrayLiteral("model_name"    ) },
    { static_cast<int>(DataRole::IMAGE   ), QByteArrayLiteral("model_image"   ) },
    { static_cast<int>(DataRole::SIZE    ), QByteArrayLiteral("model_size"    ) },
    { static_cast<int>(DataRole::DATE    ), QByteArrayLiteral("model_date"    ) },
    { static_cast<int>(DataRole::SPEED   ), QByteArrayLiteral("model_speed"   ) },
    { static_cast<int>(DataRole::STATE   ), QByteArrayLiteral("model_state"   ) },
    { static_cast<int>(DataRole::PROGRESS), QByteArrayLiteral("model_progress") },
  };

  return role_names;
}





DownloadTaskListModel::DownloadTaskListModel(QObject* parent) : QAbstractListModel(parent) {}

DownloadTaskListModel::~DownloadTaskListModel() {}

int DownloadTaskListModel::getDownloadingCount() const {
  int count{ 0 };

  for (const auto& group : groups_) {
    count += group.items->getDownloadingCount();
  }

  return count;
}

int DownloadTaskListModel::getFinishedCount() const {
  int count{ 0 };

  for (const auto& group : groups_) {
    count += group.items->getFinishedCount();
  }

  return count;
}

auto DownloadTaskListModel::find(const QString& uid) const -> bool {
  for (const auto& group : groups_) {
    if (group.uid != uid) {
      continue;
    }

    return true;
  }

  return false;
}

auto DownloadTaskListModel::load(const QString& uid) const -> ModelInfo {
  for (const auto& group : groups_) {
    if (group.uid != uid) {
      continue;
    }

    return group;
  }

  return {};
}

auto DownloadTaskListModel::append(const ModelInfo& info) -> bool {
  if (find(info.uid)) {
    return false;
  }

  connect(info.items.get(), &DownloadItemListModel::downloadingCountChanged,
          this, &DownloadTaskListModel::downloadingCountChanged,
          Qt::ConnectionType::UniqueConnection);

  connect(info.items.get(), &DownloadItemListModel::finishedCountChanged,
          this, &DownloadTaskListModel::finishedCountChanged,
          Qt::ConnectionType::UniqueConnection);

  auto signal_sender_list{ std::list<std::function<void()>>{} };

  if (info.items->getDownloadingCount() != 0) {
    signal_sender_list.emplace_back([this]() {
      Q_EMIT downloadingCountChanged();
    });
  }

  if (info.items->getFinishedCount() != 0) {
    signal_sender_list.emplace_back([this]() {
      Q_EMIT finishedCountChanged();
    });
  }

  const auto index = std::distance(groups_.cbegin(), groups_.cend());
  beginInsertRows(QModelIndex{}, index, index);
  groups_.emplace_back(info);
  endInsertRows();

  for (const auto& signal_sender : signal_sender_list) {
    signal_sender();
  }

  return true;
}

auto DownloadTaskListModel::append(ModelInfo&& info) -> bool {
  if (find(info.uid)) {
    return false;
  }

  connect(info.items.get(), &DownloadItemListModel::downloadingCountChanged,
          this, &DownloadTaskListModel::downloadingCountChanged,
          Qt::ConnectionType::UniqueConnection);

  connect(info.items.get(), &DownloadItemListModel::finishedCountChanged,
          this, &DownloadTaskListModel::finishedCountChanged,
          Qt::ConnectionType::UniqueConnection);

  auto signal_sender_list{ std::list<std::function<void()>>{} };

  if (info.items->getDownloadingCount() != 0) {
    signal_sender_list.emplace_back([this]() {
      Q_EMIT downloadingCountChanged();
    });
  }

  if (info.items->getFinishedCount() != 0) {
    signal_sender_list.emplace_back([this]() {
      Q_EMIT finishedCountChanged();
    });
  }

  const auto index = std::distance(groups_.cbegin(), groups_.cend());
  beginInsertRows(QModelIndex{}, index, index);
  groups_.emplace_back(std::move(info));
  endInsertRows();

  for (const auto& signal_sender : signal_sender_list) {
    signal_sender();
  }

  return true;
}

auto DownloadTaskListModel::update(const ModelInfo& info) -> bool {
  for (auto iter = groups_.begin(); iter != groups_.end(); ++iter) {
    if (iter->uid != info.uid) {
      continue;
    }

    auto signal_sender_list{ std::list<std::function<void()>>{
        [this, index = std::distance(groups_.begin(), iter)]()->void {
          const auto model_index = createIndex(index, index);
          Q_EMIT dataChanged(model_index, model_index);
        }
      }
    };

    if (iter->items->getDownloadingCount() != 0) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT downloadingCountChanged();
      });
    }

    if (iter->items->getFinishedCount() != 0) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT finishedCountChanged();
      });
    }

    *iter = info;

    for (const auto& signal_sender : signal_sender_list) {
      signal_sender();
    }

    return true;
  }

  return false;
}

auto DownloadTaskListModel::update(ModelInfo&& info) -> bool {
  for (auto iter = groups_.begin(); iter != groups_.end(); ++iter) {
    if (iter->uid != info.uid) {
      continue;
    }

    auto signal_sender_list{ std::list<std::function<void()>>{
        [this, index = std::distance(groups_.begin(), iter)]()->void {
          const auto model_index = createIndex(index, index);
          Q_EMIT dataChanged(model_index, model_index);
        }
      }
    };

    if (iter->items->getDownloadingCount() != 0) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT downloadingCountChanged();
      });
    }

    if (iter->items->getFinishedCount() != 0) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT finishedCountChanged();
      });
    }

    *iter = std::move(info);
    
    for (const auto& signal_sender : signal_sender_list) {
      signal_sender();
    }

    return true;
  }

  return false;
}

auto DownloadTaskListModel::emplace(const ModelInfo& info) -> void {
  if (find(info.uid)) {
    update(info);
  } else {
    append(info);
  }
}

auto DownloadTaskListModel::emplace(ModelInfo&& info) -> void {
  if (find(info.uid)) {
    update(std::move(info));
  } else {
    append(std::move(info));
  }
}

auto DownloadTaskListModel::remove(const QString& uid) -> bool {
  for (auto iter = groups_.cbegin(); iter != groups_.cend(); ++iter) {
    if (iter->uid != uid) {
      continue;
    }

    iter->items->disconnect(this);

    auto signal_sender_list{ std::list<std::function<void()>>{} };

    if (iter->items->getDownloadingCount() != 0) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT downloadingCountChanged();
      });
    }

    if (iter->items->getFinishedCount() != 0) {
      signal_sender_list.emplace_back([this]() {
        Q_EMIT finishedCountChanged();
      });
    }

    const auto index = std::distance(groups_.cbegin(), iter);
    beginRemoveRows(QModelIndex{}, index, index);
    groups_.erase(iter);
    endRemoveRows();

    for (const auto& signal_sender : signal_sender_list) {
      signal_sender();
    }

    return true;
  }

  return false;
}

auto DownloadTaskListModel::clear() -> void {
  beginResetModel();
  groups_.clear();
  endResetModel();
}

auto DownloadTaskListModel::rawData() const -> const std::vector<ModelInfo>& {
  return groups_;
}

int DownloadTaskListModel::rowCount(const QModelIndex& parent) const {
  std::ignore = parent;
  return groups_.size();
}

QVariant DownloadTaskListModel::data(const QModelIndex& index, int role) const {
  if (index.row() < 0 || index.row() >= groups_.size()) {
    return {};
  }

  QVariant result;
  const auto& group = groups_.at(index.row());
  switch (static_cast<DataRole>(role)) {
    case DataRole::UID:
      result = QVariant::fromValue(group.uid);
      break;
    case DataRole::NAME:
      result = QVariant::fromValue(group.name);
      break;
    case DataRole::ITEMS:
      result = QVariant::fromValue(group.items.get());
      break;
    default:
      break;
  }
  
  return result;
}

QHash<int, QByteArray> DownloadTaskListModel::roleNames() const {
  static const QHash<int, QByteArray> role_names{
    { static_cast<int>(DataRole::UID         ), QByteArrayLiteral("model_uid"         ) },
    { static_cast<int>(DataRole::NAME        ), QByteArrayLiteral("model_name"        ) },
    { static_cast<int>(DataRole::ITEMS       ), QByteArrayLiteral("model_items"       ) },
  };
  return role_names;
}

} // namespace cxcloud
