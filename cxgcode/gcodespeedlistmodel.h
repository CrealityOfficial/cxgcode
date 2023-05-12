#ifndef CXGCODE_CREATIVE_KERNEL_GCODESPEEDLISTMODEL_1677063873803_H
#define CXGCODE_CREATIVE_KERNEL_GCODESPEEDLISTMODEL_1677063873803_H
#include "cxgcode/interface.h"
#include <QtCore/QAbstractListModel>

namespace cxgcode
{
    class CXGCODE_API GCodeSpeedListModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        explicit GCodeSpeedListModel(QObject* parent = nullptr);
        ~GCodeSpeedListModel();

        void setColors(const QList<QColor>& colors);
        void setRange(float minSpeed, float maxSpeed);
    protected:
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        virtual QHash<int, QByteArray> roleNames() const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    protected:
        QList<QColor> m_baseColors;
        float m_minSpeed;
        float m_maxSpeed;
    };
}

#endif // CREATIVE_KERNEL_GCODESPEEDLISTMODEL_1677063873803_H