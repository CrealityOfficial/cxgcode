#include "gcodelinewidthlistmodel.h"

namespace cxgcode {

GcodeLineWidthListModel::GcodeLineWidthListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(gcode::GCodeVisualType::gvt_lineWidth, parent) {}

}  // namespace cxgcode
