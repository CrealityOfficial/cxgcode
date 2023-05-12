#ifndef CXGCODE_CREATIVE_KERNEL_GCODETYPELISTMODEL_1677312656658_H
#define CXGCODE_CREATIVE_KERNEL_GCODETYPELISTMODEL_1677312656658_H
#include "cxgcode/interface.h"
#include <QtCore/QAbstractListModel>

namespace cxgcode
{
    struct GCodeTypeItemData
    {
        QString modelColorData;
        QString modelTextData;
        QString modelColorKey;
        bool modelChecked;

        GCodeTypeItemData() {
            modelChecked = true;
        }

        GCodeTypeItemData(const QString& str1, const QString& str2, const QString& str3, bool check)
        {
            modelColorData = str1;
            modelTextData = str2;
            modelColorKey = str3;
            modelChecked = check;
        }
    };

    class CXGCODE_API GCodeTypeListModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        explicit GCodeTypeListModel(QObject* parent = nullptr);
        ~GCodeTypeListModel();

        Q_INVOKABLE void checkItem(int index, bool check);
        void generateDatas();

    signals:
        void itemCheckStateChanged(int type, bool check);
    protected:
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        virtual QHash<int, QByteArray> roleNames() const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    protected:
        QList<GCodeTypeItemData> m_datas;
    };
}

#endif // CREATIVE_KERNEL_GCODETYPELISTMODEL_1677312656658_H