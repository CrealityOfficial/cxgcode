#include "gcodetemperaturelistmodel.h"

namespace cxgcode {

GcodeTemperatureListModel::GcodeTemperatureListModel(QObject* parent)
    : GcodePreviewRangeDivideModel(GCodeVisualType::gvt_temperature, parent) {}

void GcodeTemperatureListModel::setRange(double min, double max)
{
    m_min = min;
    m_max = max;

    float diff = m_max - m_min;

    if (diff == 0)
    {
        
        QList<QColor> c;
        c << m_colors[0];
        setColors(c);

    } else if (diff < 200.0)
    {
        if (m_colors.size() >= diff / 20.0)
        {
            QList<QColor> c;
            float t = diff;
            int index = 0;
            while (t > 0.0)
            {
                c << m_colors[index];
                t -= 20.0;
                index += 1;
            }

            setColors(c);
        }
        else {
            resetData();
        }
    }
    else {
        resetData();
    }

}

}  // namespace cxgcode
