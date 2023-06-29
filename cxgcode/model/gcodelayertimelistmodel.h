#pragma once

#ifndef CXGCODE_MODEL_GCODELAYERTIMELISTMODEL_H_
#define CXGCODE_MODEL_GCODELAYERTIMELISTMODEL_H_

#include <QtCore/QList>
#include <QtGui/QColor>

#include "cxgcode/interface.h"
#include "cxgcode/model/gcodepreviewlistmodel.h"

namespace cxgcode {

struct GcodeLayerTimeData{
  QColor color;
};

class CXGCODE_API GcodeLayerTimeListModel : public GcodePreviewListModel {
  Q_OBJECT;

public:
  explicit GcodeLayerTimeListModel(QObject* parent = nullptr);
  virtual ~GcodeLayerTimeListModel() = default;

public:
  void setDataList(const QList<GcodeLayerTimeData>& data_list);

protected:
  int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::ItemDataRole::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

private:
  enum DataRole : int {
    COLOR = Qt::ItemDataRole::UserRole + 1,
  };

  QList<GcodeLayerTimeData> data_list_;
};

}  // namespace cxgcode

#endif  // CXGCODE_MODEL_GCODELAYERTIMELISTMODEL_H_
