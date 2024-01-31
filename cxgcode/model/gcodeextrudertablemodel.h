#pragma once

#ifndef CXGCODE_MODEL_GCODEEXTRUDERTABLEMODEL_H_
#define CXGCODE_MODEL_GCODEEXTRUDERTABLEMODEL_H_

#include <QtCore/QList>
#include <QtGui/QColor>

#include "cxgcode/interface.h"
#include "cxgcode/model/gcodeextruderlistmodel.h"

namespace cxgcode {

class MaterialConsume : public QObject //����������
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
    Q_INVOKABLE double getMaterialIndex() { return materialIndex; };
    Q_INVOKABLE QColor getMaterialColor() { return materialColor; };

    double length = 0.0;
    double weight = 0.0;
    int materialIndex = 0;
    QColor materialColor;
};

class ModelItem : public QObject 
{
    Q_OBJECT
    public:
    enum ConsumeType //���ĵ�����
    {
        CT_Color,
        CT_ColorIndex,
        CT_Model,
        CT_Flush,
        CT_FilamentTower,
        CT_Total
    };

   ModelItem(MaterialConsume* model = nullptr, MaterialConsume* flush = nullptr, 
       MaterialConsume* filamentTower = nullptr, MaterialConsume* total = nullptr, QObject* parent = nullptr);
   ~ModelItem();

   MaterialConsume* getDataByType(ModelItem::ConsumeType type);

private:
    MaterialConsume* modelMaterial() const;
    MaterialConsume* flushMaterial() const;
    MaterialConsume* filamentTower() const;
    MaterialConsume* totalMaterial() const;

    MaterialConsume* m_Model = nullptr;
    MaterialConsume* m_Flush = nullptr;
    MaterialConsume* m_FilamentTower = nullptr;

    MaterialConsume* m_Total = nullptr;
};

class CXGCODE_API GcodeExtruderTableModel : public QAbstractTableModel 
{
  Q_OBJECT
  Q_PROPERTY(bool isSingleColor READ isSingleColor WRITE setIsSingleColor NOTIFY isSingleColorChanged)

public:
  explicit GcodeExtruderTableModel(QObject* parent = nullptr);
  virtual ~GcodeExtruderTableModel() = default;

  void setColorList(const QList<GcodeExtruderData>& data_list);

  void setData(QList<ModelItem* > dataList);
  void setData(std::vector<std::pair<int, double>>& model, std::vector<std::pair<int, double>>& flush, std::vector<std::pair<int, double>>& filamentTower);

  bool isSingleColor();
  void setIsSingleColor(bool isSingle);

protected:
  int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  int columnCount(const QModelIndex & = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::ItemDataRole::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

signals:
  void isSingleColorChanged();

private:
    QList<ModelItem* > m_Materials;
    QList<QColor> m_Colors;
    QList<GcodeExtruderData> m_ColorsData;

    bool m_IsSingleColor = false;
};
}  // namespace cxgcode

#endif  // CXGCODE_MODEL_GCODEEXTRUDERTABLEMODEL_H_
