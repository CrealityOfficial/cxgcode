#pragma once

#ifndef CXGCODE_MODEL_GCODEPREVIEWLISTMODEL_H_
#define CXGCODE_MODEL_GCODEPREVIEWLISTMODEL_H_

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtGui/QColor>

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



struct GcodeRangeDivideData {
	QColor color;
	double value;
};

class CXGCODE_API GcodePreviewRangeDivideModel : public GcodePreviewListModel {
	Q_OBJECT;

public:
	explicit GcodePreviewRangeDivideModel(GCodeVisualType type, QObject* parent = nullptr);
	virtual ~GcodePreviewRangeDivideModel() = default;

public:
	void setColors(const QList<QColor>& colors);
	virtual void setRange(double min, double max);

protected:
	int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
	QVariant data(const QModelIndex& index, int role = Qt::ItemDataRole::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;

protected:
	void resetData();

protected:
	enum DataRole : int {
		COLOR = Qt::ItemDataRole::UserRole + 1,
		VALUE,
	};

	QList<GcodeRangeDivideData> m_dataList;
	QList<QColor> m_colors;
	double m_min;
	double m_max;
};




}  // namespace cxgcode

#endif  // CXGCODE_MODEL_GCODEPREVIEWLISTMODEL_H_
