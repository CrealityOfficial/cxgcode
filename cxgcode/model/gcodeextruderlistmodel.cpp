#include "gcodeextruderlistmodel.h"

namespace cxgcode {

GcodeExtruderListModel::GcodeExtruderListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void GcodeExtruderListModel::setDataList(const QList<GcodeExtruderData>& data_list) {
  beginResetModel();
  data_list_ = data_list;
  endResetModel();
}

int GcodeExtruderListModel::rowCount(const QModelIndex& parent) const {
  return data_list_.size();
}

QVariant GcodeExtruderListModel::data(const QModelIndex& index, int role) const {
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

QHash<int, QByteArray> GcodeExtruderListModel::roleNames() const {
  return {
    { static_cast<int>(DataRole::COLOR), QByteArrayLiteral("model_color") },
  };
}

}  // namespace cxgcode
