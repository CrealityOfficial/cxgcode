#include "gcodefanspeedlistmodel.h"

namespace cxgcode {

GcodeFanSpeedListModel::GcodeFanSpeedListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_fanSpeed, parent) {}

}  // namespace cxgcode
