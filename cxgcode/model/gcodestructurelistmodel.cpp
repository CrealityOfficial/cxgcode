#include "gcodestructurelistmodel.h"

namespace cxgcode {

GcodeStructureListModel::GcodeStructureListModel(QObject* parent)
    : GcodePreviewListModel(GCodeVisualType::gvt_structure, parent)
    , data_list_({
      { QColor{ QStringLiteral("#772D28") }, QStringLiteral("Outer Perimeter") , 1 , true  },
      { QColor{ QStringLiteral("#028C05") }, QStringLiteral("Inner Perimeter") , 2 , true  },
      { QColor{ QStringLiteral("#FFB27F") }, QStringLiteral("Skin")            , 3 , true  },
      { QColor{ QStringLiteral("#058C8C") }, QStringLiteral("Support")         , 4 , true  },
      { QColor{ QStringLiteral("#511E54") }, QStringLiteral("SkirtBrim")       , 5 , true  },
      { QColor{ QStringLiteral("#E5DB33") }, QStringLiteral("Infill")          , 6 , true  },
      { QColor{ QStringLiteral("#B5BC38") }, QStringLiteral("SupportInfill")   , 7 , true  },
      { QColor{ QStringLiteral("#FF6B44") }, QStringLiteral("MoveCombing")     , 8 , true  },
      // { QColor{ QStringLiteral("#66334C") }, QStringLiteral("MoveRetraction")  , 9 , true  },
      // { QColor{ QStringLiteral("#D63A11") }, QStringLiteral("SupportInterface"), 10, true  },
      { QColor{ QStringLiteral("#339919") }, QStringLiteral("PrimeTower")      , 11, true  },
      { QColor{ QStringLiteral("#60595F") }, QStringLiteral("Travel")          , 13, false },
      // { QColor{ QStringLiteral("#0000FF") }, QStringLiteral("FlowTravel")      , 15, true  },
      // { QColor{ QStringLiteral("#FF0000") }, QStringLiteral("AdvanceTravel")   , 16, true  },
      { QColor{ QStringLiteral("#FFFFFF") }, QStringLiteral("Zseam")           , 17, true  },
      { QColor{ QStringLiteral("#FF00FF") }, QStringLiteral("Retraction")      , 18, false },
    }) {}

void GcodeStructureListModel::checkItem(int type, bool checked) {
  for (auto const& data : data_list_) {
    if (data.type == type) {
      Q_EMIT itemCheckedChanged(type, checked);
    }
  }
}

void GcodeStructureListModel::setDataList(const QList<GcodeStructureData>& data_list) {
  beginResetModel();
  data_list_ = data_list;
  endResetModel();
}

int GcodeStructureListModel::rowCount(const QModelIndex& parent) const {
  return data_list_.size();
}

QHash<int, QByteArray> GcodeStructureListModel::roleNames() const {
  return {
    { static_cast<int>(DataRole::COLOR)  , QByteArrayLiteral("model_color")   },
    { static_cast<int>(DataRole::NAME)   , QByteArrayLiteral("model_name")    },
    { static_cast<int>(DataRole::TYPE)   , QByteArrayLiteral("model_type")    },
    { static_cast<int>(DataRole::CHECKED), QByteArrayLiteral("model_checked") },
  };
}

QVariant GcodeStructureListModel::data(const QModelIndex& index, int role) const {
  QVariant result{ QVariant::Type::Invalid };
  if (index.row() < 0 || index.row() >= rowCount()) {
    return result;
  }

  auto const& data = data_list_[index.row()];
  switch (static_cast<DataRole>(role)) {
    case DataRole::COLOR:
      result = QVariant::fromValue(data.color);
      break;
    case DataRole::NAME:
      result = QVariant::fromValue(data.name);
      break;
    case DataRole::TYPE:
      result = QVariant::fromValue(data.type);
      break;
    case DataRole::CHECKED:
      result = QVariant::fromValue(data.checked);
      break;
    default:
      break;
  }

  return result;
}

}  // namespace cxgcode
