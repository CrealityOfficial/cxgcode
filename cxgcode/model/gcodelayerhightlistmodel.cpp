#include "gcodelayerhightlistmodel.h"

namespace cxgcode {

GcodeLayerHightListModel::GcodeLayerHightListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(gcode::GCodeVisualType::gvt_layerHight, parent) {}

void GcodeLayerHightListModel::setRange(double min, double max)
{
    m_min = min;
    m_max = max;

    if (min > 0.0 && (max-min) < min / 50.0)
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
