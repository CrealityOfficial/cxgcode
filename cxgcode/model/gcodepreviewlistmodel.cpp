#include "gcodepreviewlistmodel.h"

namespace cxgcode {

GcodePreviewListModel::GcodePreviewListModel(GCodeVisualType type, QObject* parent)
    : QAbstractListModel(parent)
    , type_(type) {}

GCodeVisualType GcodePreviewListModel::getVisualType() const {
  return type_;
}

int GcodePreviewListModel::getVisualTypeIndex() const {
  return static_cast<int>(getVisualType());
}

}  // namespace cxgcode
