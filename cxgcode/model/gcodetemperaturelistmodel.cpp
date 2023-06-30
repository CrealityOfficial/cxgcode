#include "gcodetemperaturelistmodel.h"

namespace cxgcode {

GcodeTemperatureListModel::GcodeTemperatureListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_temperature, parent) {}

}  // namespace cxgcode
