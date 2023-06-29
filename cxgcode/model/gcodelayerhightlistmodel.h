#pragma once

#ifndef CXGCODE_MODEL_GCODELAYERHIGHTLISTMODEL_H_
#define CXGCODE_MODEL_GCODELAYERHIGHTLISTMODEL_H_

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtGui/QColor>

#include "cxgcode/interface.h"

namespace cxgcode {

struct GcodeLayerHightData{
  QColor color;
};

class CXGCODE_API GcodeLayerHightListModel : public QAbstractListModel {
  Q_OBJECT;

public:
  explicit GcodeLayerHightListModel(QObject* parent = nullptr);
  virtual ~GcodeLayerHightListModel() = default;

public:
  void setDataList(const QList<GcodeLayerHightData>& data_list);

protected:
  int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::ItemDataRole::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

private:
  enum DataRole : int {
    COLOR = Qt::ItemDataRole::UserRole + 1,
  };

  QList<GcodeLayerHightData> data_list_;
};

}  // namespace cxgcode

#endif  // CXGCODE_MODEL_GCODELAYERHIGHTLISTMODEL_H_
