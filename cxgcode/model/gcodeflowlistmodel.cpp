#include "gcodeflowlistmodel.h"

namespace cxgcode {

GcodeFlowListModel::GcodeFlowListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_flow, parent) {}

}  // namespace cxgcode
