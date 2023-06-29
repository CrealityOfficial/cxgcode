#include "gcodetemperaturelistmodel.h"

namespace cxgcode {

GcodeTemperatureListModel::GcodeTemperatureListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void GcodeTemperatureListModel::setDataList(const QList<GcodeTemperatureListData>& data_list) {
  beginResetModel();
  data_list_ = data_list;
  endResetModel();
}

int GcodeTemperatureListModel::rowCount(const QModelIndex& parent) const {
  return data_list_.size();
}

QVariant GcodeTemperatureListModel::data(const QModelIndex& index, int role) const {
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

QHash<int, QByteArray> GcodeTemperatureListModel::roleNames() const {
  return {
    { static_cast<int>(DataRole::COLOR), QByteArrayLiteral("model_color") },
  };
}

}  // namespace cxgcode
