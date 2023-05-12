#include "gcodetypelistmodel.h"

namespace cxgcode
{
    enum {
        NameRole = Qt::UserRole + 1,
        NameRole1,
        NameRole2,
        NameRole3
    };

    GCodeTypeListModel::GCodeTypeListModel(QObject* parent)
        : QAbstractListModel(parent)
    {
        generateDatas();
    }

    GCodeTypeListModel::~GCodeTypeListModel()
    {
    }

    void GCodeTypeListModel::checkItem(int index, bool check)
    {
        if (index < 0 || index >= rowCount())
            return;

        GCodeTypeItemData& data = m_datas[index];
        data.modelChecked = check;

        bool success = false;
        int type = data.modelColorKey.toInt(&success);
        if (success)
            emit itemCheckStateChanged(type, check);
    }

    void GCodeTypeListModel::generateDatas()
    {
        beginResetModel();
        m_datas.clear();
        m_datas << GCodeTypeItemData({ "#772D28", "Outer Perimeter" ,"1" , true });
        m_datas << GCodeTypeItemData({ "#028C05", "Inner Perimeter" ,"2" , true });
        m_datas << GCodeTypeItemData({ "#FFB27F", "Skin"            ,"3" , true });
        m_datas << GCodeTypeItemData({ "#058C8C", "Support"         ,"4" , true });
        m_datas << GCodeTypeItemData({ "#511E54", "SkirtBrim"       ,"5" , true });
        m_datas << GCodeTypeItemData({ "#E5DB33", "Infill"          ,"6" , true });
        m_datas << GCodeTypeItemData({ "#B5BC38", "SupportInfill"   ,"7" , true });
        //m_datas << GCodeTypeItemData({ "#FF6B44", "MoveCombing"     ,"8" , true });
        //m_datas << GCodeTypeItemData({ "#66334C", "MoveRetraction"  ,"9" , true });
        m_datas << GCodeTypeItemData({ "#D63A11", "SupportInterface","10", true });
        m_datas << GCodeTypeItemData({ "#339919", "PrimeTower"      ,"11", true });
        m_datas << GCodeTypeItemData({ "#60595F", "Travel"          ,"13", false});
        //m_datas << GCodeTypeItemData({ "#0000FF", "FlowTravel"      ,"15", true });
        //m_datas << GCodeTypeItemData({ "#FF0000", "AdvanceTravel"   ,"16", true });
		m_datas << GCodeTypeItemData({ "#FFFFFF", "Zseam"      ,"17", true });
		m_datas << GCodeTypeItemData({ "#FF00FF", "Retraction"   ,"18", false });

        endResetModel();
    }

    int GCodeTypeListModel::rowCount(const QModelIndex& parent) const
    {
        return m_datas.count();
    }

    QHash<int, QByteArray> GCodeTypeListModel::roleNames() const
    {
        QHash<int, QByteArray> roles;
        roles[NameRole] = "modelColorData";
        roles[NameRole1] = "modelTextData";
        roles[NameRole2] = "modelColorKey";
        roles[NameRole3] = "modelChecked";

        return roles;
    }

    QVariant GCodeTypeListModel::data(const QModelIndex& index, int role) const
    {
        int i = index.row();
        if (i < 0 || i >= rowCount() || rowCount() < 2)
            return QVariant(QVariant::Invalid);

        const GCodeTypeItemData& data = m_datas.at(i);

        if (role == NameRole)
        {
            return QVariant::fromValue(data.modelColorData);
        }
        if (role == NameRole1)
        {
            return QVariant::fromValue(data.modelTextData);
        }
        if (role == NameRole2)
        {
            return QVariant::fromValue(data.modelColorKey);
        }
        if (role == NameRole3)
        {
            return QVariant::fromValue(data.modelChecked);
        }
        return QVariant(QVariant::Invalid);
    }
}