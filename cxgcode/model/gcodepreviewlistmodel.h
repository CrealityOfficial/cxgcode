#pragma once

#ifndef CXGCODE_MODEL_GCODEPREVIEWLISTMODEL_H_
#define CXGCODE_MODEL_GCODEPREVIEWLISTMODEL_H_

#include <QtCore/QAbstractListModel>

#include "cxgcode/define.h"
#include "cxgcode/interface.h"

namespace cxgcode {

class CXGCODE_API GcodePreviewListModel : public QAbstractListModel {
  Q_OBJECT;

public:
  explicit GcodePreviewListModel(GCodeVisualType type, QObject* parent = nullptr);
  virtual ~GcodePreviewListModel() = default;

public:
  GCodeVisualType getVisualType() const;
  int getVisualTypeIndex() const;
  Q_PROPERTY(int visualType READ getVisualTypeIndex CONSTANT);

private:
  const GCodeVisualType type_;
};

}  // namespace cxgcode

#endif  // CXGCODE_MODEL_GCODEPREVIEWLISTMODEL_H_
