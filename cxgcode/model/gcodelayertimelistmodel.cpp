#include "gcodelayertimelistmodel.h"

namespace cxgcode {

GcodeLayerTimeListModel::GcodeLayerTimeListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_layerTime, parent) {}

}  // namespace cxgcode
