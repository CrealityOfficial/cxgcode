#include "gcodeextrudertablemodel.h"

namespace cxgcode {

ModelItem::ModelItem(QObject* parent)
    : QObject(parent) 
{
    m_Model = new MaterialConsume();// { 6.23, 3.3 };
    m_Flush = new MaterialConsume();;
    m_FilamentTower = new MaterialConsume();

    m_Model->weight = 5.23;
    m_Model->length = 3.3;

    m_Flush->weight = 5.23;
    m_Flush->length = 3.3;

    m_FilamentTower->weight = 5.23;
    m_FilamentTower->length = 3.3;
}

ModelItem::~ModelItem() {}

MaterialConsume* ModelItem::getDataByType(ModelItem::ConsumeType type)
{
    switch (type)
    {
    case cxgcode::ModelItem::CT_Color:
        return modelMaterial();
        break;
    case cxgcode::ModelItem::CT_Model:
        return modelMaterial();
        break;
    case cxgcode::ModelItem::CT_Flush:
        return flushMaterial();
        break;
    case cxgcode::ModelItem::CT_FilamentTower:
        return filamentTower();
        break;
    case cxgcode::ModelItem::CT_Total:
        return modelMaterial();
        break;
    default:
        break;
    }

    return nullptr;
}

MaterialConsume* ModelItem::modelMaterial() const
{
    return m_Model;
}

MaterialConsume* ModelItem::flushMaterial() const
{
    return m_Flush;
}

MaterialConsume* ModelItem::filamentTower() const
{
    return m_FilamentTower;
}

GcodeExtruderTableModel::GcodeExtruderTableModel(QObject* parent)
: QAbstractTableModel(parent)
{
    m_Materials.append(new ModelItem(this));
    m_Materials.append(new ModelItem(this));
    m_Materials.append(new ModelItem(this));
    m_Materials.append(new ModelItem(this));
    m_Materials.append(new ModelItem(this));
}

void GcodeExtruderTableModel::setDataList(const QList<cxgcode::GcodeExtruderData>& data_list) 
{
    beginResetModel();
    m_ColorsData = data_list;
    endResetModel();
}

int GcodeExtruderTableModel::rowCount(const QModelIndex& parent) const 
{
  return m_Materials.count();
}

int GcodeExtruderTableModel::columnCount(const QModelIndex & parent) const
{
    return 3;
}

QVariant GcodeExtruderTableModel::data(const QModelIndex& index, int role) const 
{
  QVariant result{ QVariant::Type::Invalid };

  if (index.row() < 0 || index.row() >= rowCount() || rowCount() < 2) {
    return result;
  }

  if (m_ColorsData.count() <= index.row())
  {
      return result;
  }

  ModelItem* itemData = m_Materials.at(index.row());

  switch (role) 
  {
      case ModelItem::CT_Color:
      {
          cxgcode::GcodeExtruderData ged = m_ColorsData.at(index.row());
          return ged.color;
      }
      case ModelItem::CT_Model:
      {
          MaterialConsume* mc = itemData->getDataByType((ModelItem::ConsumeType)index.column());
          return QVariant::fromValue(mc);
      }
      case ModelItem::CT_Flush:
      {
          MaterialConsume* mc = itemData->getDataByType((ModelItem::ConsumeType)index.column());
          return QVariant::fromValue(mc);
      }
      case ModelItem::CT_FilamentTower:
      {
          MaterialConsume* mc = itemData->getDataByType((ModelItem::ConsumeType)index.column());
          return QVariant::fromValue(mc);
      }
      case ModelItem::CT_Total:
      {
          MaterialConsume* mc = itemData->getDataByType((ModelItem::ConsumeType)index.column());
          return QVariant::fromValue(mc);
      }
      
      default:
          break;
  }

  return QVariant();
}

QHash<int, QByteArray> GcodeExtruderTableModel::roleNames() const {
    QHash<int, QByteArray> hash =  { {ModelItem::CT_Model, "ctModel"},
        {ModelItem::CT_Color, "ctColor"},
        {ModelItem::CT_Flush, "ctFlush"},
        {ModelItem::CT_FilamentTower, "ctFilamentTower"},
        {ModelItem::CT_Total, "ctTotal"}, };

    return hash;
}

QVariant GcodeExtruderTableModel::headerData(int section, Qt::Orientation orientation, int role) const 
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        // ·µ»ØË®Æ½±íÍ·
        return QString("Column %1").arg(section);
    }
    else {
        // ·µ»Ø´¹Ö±±íÍ·
        return QString("Row %1").arg(section);
    }
}
}; // namespace cxgcode
