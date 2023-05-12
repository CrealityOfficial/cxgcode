#include "gcodespeedlistmodel.h"
#include <QtGui/QColor>

namespace cxgcode
{
    enum {
        NameRole = Qt::UserRole + 1,
        ValueRole
    };

    GCodeSpeedListModel::GCodeSpeedListModel(QObject* parent)
        : QAbstractListModel(parent)
        , m_minSpeed(0.0f)
        , m_maxSpeed(1.0f)
    {
    }

    GCodeSpeedListModel::~GCodeSpeedListModel()
    {
    }

    void GCodeSpeedListModel::setColors(const QList<QColor>& colors)
    {
        m_baseColors = colors;
    }

    void GCodeSpeedListModel::setRange(float minSpeed, float maxSpeed)
    {
        beginResetModel();
        m_maxSpeed = maxSpeed;
        m_minSpeed = minSpeed;
        endResetModel();
    }

    int GCodeSpeedListModel::rowCount(const QModelIndex& parent) const
    {
        return m_baseColors.count();
    }

    QHash<int, QByteArray> GCodeSpeedListModel::roleNames() const
    {
        QHash<int, QByteArray> roles;
        roles[NameRole] = "modelColorData";
        roles[ValueRole] = "modelTextData";

        return roles;
    }

    QVariant GCodeSpeedListModel::data(const QModelIndex& index, int role) const
    {
        int i = index.row();
        if (i < 0 || i >= rowCount() || rowCount() < 2)
            return QVariant(QVariant::Invalid);

        i = rowCount() - i - 1;

        if (role == NameRole)
        {
            QString name = m_baseColors.at(i).name(QColor::HexRgb);
            return QVariant::fromValue(name);
        }
        if (role == ValueRole)
        {
            float speedDiff = m_maxSpeed - m_minSpeed;
            float speedDelta = (speedDiff > 0 ? speedDiff : m_maxSpeed) / (rowCount() - 1);
            float speed = (m_minSpeed + speedDelta * (float)i) / 60.0f;
            QString value = QString::number(speed, 10, 1);
            return QVariant::fromValue(value);
        }

        return QVariant(QVariant::Invalid);
    }
}