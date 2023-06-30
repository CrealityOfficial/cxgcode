#include "gcodelinewidthlistmodel.h"

namespace cxgcode {

GcodeLineWidthListModel::GcodeLineWidthListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_lineWidth, parent) {}

}  // namespace cxgcode
