#include "gcodepreviewlistmodel.h"

namespace cxgcode {

GcodePreviewListModel::GcodePreviewListModel(GCodeVisualType type, QObject* parent)
    : QAbstractListModel(parent)
    , type_(type) {}

GCodeVisualType GcodePreviewListModel::getVisualType() const {
  return type_;
}

int GcodePreviewListModel::getVisualTypeIndex() const {
  return static_cast<int>(getVisualType());
}

}  // namespace cxgcode


namespace cxgcode {

    GcodePreviewRangeDivideModel::GcodePreviewRangeDivideModel(GCodeVisualType type, QObject* parent)
        :GcodePreviewListModel(type, parent)
        , m_min(0.0f)
        , m_max(1.0f)
    {
    }

    void GcodePreviewRangeDivideModel::setColors(const QList<QColor>& colors)
    {
        m_colors = colors;
        resetData();
    }

    void GcodePreviewRangeDivideModel::setRange(double min, double max) {
        assert(min <= max && "min value is bigger than max value");
        m_min = min;
        m_max = max;
        resetData();
    }

    int GcodePreviewRangeDivideModel::rowCount(const QModelIndex& parent) const {
        return m_dataList.size();
    }

    QVariant GcodePreviewRangeDivideModel::data(const QModelIndex& index, int role) const {
        QVariant result{ QVariant::Type::Invalid };
        if (index.row() < 0 || index.row() >= rowCount()) {
            return result;
        }

        auto const& data = m_dataList[index.row()];
        switch (static_cast<DataRole>(role)) {
        case DataRole::COLOR:
            result = QVariant::fromValue(data.color);
            break;
        case DataRole::VALUE:
            result = QVariant::fromValue(data.value);
            break;
        default:
            break;
        }

        return result;
    }

    QHash<int, QByteArray> GcodePreviewRangeDivideModel::roleNames() const {
        return {
          { static_cast<int>(DataRole::COLOR), QByteArrayLiteral("model_color") },
          { static_cast<int>(DataRole::VALUE), QByteArrayLiteral("model_value") },
        };
    }

    void GcodePreviewRangeDivideModel::resetData() {

        QList<GcodeRangeDivideData> data_list;

        size_t const data_size = m_colors.size();
        if (data_size == 0) {
            // do nothing
        } else if (data_size == 1) {
            data_list << GcodeRangeDivideData{ m_colors[0], m_min };
        } else {
            double const diff = (m_max - m_min) / (m_colors.size() - 1);
            for (size_t index = 0ull; index < m_colors.size(); ++index) {
                double value = (m_min + diff * index);
                data_list << GcodeRangeDivideData{ m_colors[index], value };
            }
        }

        std::reverse(data_list.begin(), data_list.end());

        beginResetModel();
        m_dataList = std::move(data_list);
        endResetModel();
    }

}
