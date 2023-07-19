#include "gcodetemperaturelistmodel.h"

namespace cxgcode {

GcodeTemperatureListModel::GcodeTemperatureListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_temperature, parent) {}

void GcodeTemperatureListModel::setRange(double min, double max) {
    //assert(min <= max && "min value is bigger than max value");
    m_min = min;
    m_max = max;

    if (max - min <= 0.1)
    {
        QList<QColor> c;
        c << m_colors[0];
        setColors(c);
    }
    else {
        resetData();
    }
}

}  // namespace cxgcode
