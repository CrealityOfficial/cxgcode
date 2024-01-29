#pragma once

#ifndef CXGCODE_MODEL_GCODEEXTRUDERTABLEMODEL_H_
#define CXGCODE_MODEL_GCODEEXTRUDERTABLEMODEL_H_

#include <QtCore/QList>
#include <QtGui/QColor>

#include "cxgcode/interface.h"
#include "cxgcode/model/gcodeextruderlistmodel.h"

namespace cxgcode {

class MaterialConsume : public QObject //重量，长度
{
    Q_OBJECT
public:
    //Q_PROPERTY(double length READ getLength)
    //Q_PROPERTY(double weight READ getLength)
    
    //double getLength() { return length;  }
    //double getWeight() { return weight;  }
    MaterialConsume(QObject* parent = nullptr):QObject(parent){ }
    ~MaterialConsume() {}
    MaterialConsume(const MaterialConsume& modelItem) {};
    Q_INVOKABLE double getLength() { return length; };
    Q_INVOKABLE double getWeight() { return weight; };

    double length;
    double weight;
};

class ModelItem : public QObject 
{
    Q_OBJECT
    public:
    enum ConsumeType //消耗的类型
    {
        CT_Color,
        CT_Model,
        CT_Flush,
        CT_FilamentTower,
        CT_Total
    };

   ModelItem(QObject* parent = nullptr);
   ~ModelItem();
   MaterialConsume* getDataByType(ModelItem::ConsumeType type);

private:
    MaterialConsume* modelMaterial() const;
    MaterialConsume* flushMaterial() const;
    MaterialConsume* filamentTower() const;

    MaterialConsume* m_Model = nullptr;
    MaterialConsume* m_Flush = nullptr;
    MaterialConsume* m_FilamentTower = nullptr;
};

class CXGCODE_API GcodeExtruderTableModel : public QAbstractTableModel 
{
  Q_OBJECT
public:
  explicit GcodeExtruderTableModel(QObject* parent = nullptr);
  virtual ~GcodeExtruderTableModel() = default;
  void setDataList(const QList<GcodeExtruderData>& data_list);
protected:
  int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  int columnCount(const QModelIndex & = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::ItemDataRole::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    QList<ModelItem* > m_Materials;
    QList<QColor> m_Colors;
    QList<cxgcode::GcodeExtruderData> m_ColorsData;
};
}  // namespace cxgcode

#endif  // CXGCODE_MODEL_GCODEEXTRUDERTABLEMODEL_H_
