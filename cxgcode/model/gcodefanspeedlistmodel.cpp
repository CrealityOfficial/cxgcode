#include "gcodefanspeedlistmodel.h"

namespace cxgcode {

GcodeFanSpeedListModel::GcodeFanSpeedListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void GcodeFanSpeedListModel::setDataList(const QList<GcodeFanSpeedData>& data_list) {
  beginResetModel();
  data_list_ = data_list;
  endResetModel();
}

int GcodeFanSpeedListModel::rowCount(const QModelIndex& parent) const {
  return data_list_.size();
}

QVariant GcodeFanSpeedListModel::data(const QModelIndex& index, int role) const {
  QVariant result{ QVariant::Type::Invalid };
  if (index.row() < 0 || index.row() >= rowCount() || rowCount() < 2) {
    return result;
  }

  auto const& data = data_list_[index.row()];
  switch (static_cast<DataRole>(role)) {
    case DataRole::COLOR:
      result = QVariant::fromValue(data.color);
      break;
    default:
      break;
  }

  return result;
}

QHash<int, QByteArray> GcodeFanSpeedListModel::roleNames() const {
  return {
    { static_cast<int>(DataRole::COLOR), QByteArrayLiteral("model_color") },
  };
}

}  // namespace cxgcode
