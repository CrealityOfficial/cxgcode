#pragma once

#ifndef CXGCODE_MODEL_GCODEFANSPEEDLISTMODEL_H_
#define CXGCODE_MODEL_GCODEFANSPEEDLISTMODEL_H_

#include <QtCore/QList>
#include <QtGui/QColor>

#include "cxgcode/interface.h"
#include "cxgcode/model/gcodepreviewlistmodel.h"

namespace cxgcode {

struct GcodeFanSpeedData{
  QColor color;
};

class CXGCODE_API GcodeFanSpeedListModel : public GcodePreviewListModel {
  Q_OBJECT;

public:
  explicit GcodeFanSpeedListModel(QObject* parent = nullptr);
  virtual ~GcodeFanSpeedListModel() = default;

public:
  void setDataList(const QList<GcodeFanSpeedData>& data_list);

protected:
  int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::ItemDataRole::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

private:
  enum DataRole : int {
    COLOR = Qt::ItemDataRole::UserRole + 1,
  };

  QList<GcodeFanSpeedData> data_list_;
};

}  // namespace cxgcode

#endif  // CXGCODE_MODEL_GCODEFANSPEEDLISTMODEL_H_
