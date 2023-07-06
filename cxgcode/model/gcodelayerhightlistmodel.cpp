#include "gcodelayerhightlistmodel.h"

namespace cxgcode {

GcodeLayerHightListModel::GcodeLayerHightListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_layerHight, parent) {}

void GcodeLayerHightListModel::setRange(double min, double max)
{
    m_min = min;
    m_max = max;

    if (min > 0.0 && (m_max-m_min) < min / 50.0)
    {
        int m = ((m_max + m_min) / 2.0 * 10.0 + 0.5);

        m = std::min(m_colors.size()-1, m);

        QList<QColor> c;
        c << m_colors[m];
        setColors(c);
    }
    else {
        resetData();
    }
}

}  // namespace cxgcode
