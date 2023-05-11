#ifndef SLICE_RESULT_H
#define SLICE_RESULT_H
#include "cxgcode/interface.h"
#include "cxgcode/us/usettings.h"
#include "ccglobal/tracer.h"
#include "trimesh2/Box.h"
#include <QtCore/QFile>
#include <QtGui/QImage>

namespace cxgcode
{
    class CXGCODE_API SliceResult
    {
    public:
        SliceResult();
        ~SliceResult();

        const QString& prefixCode();
        const QStringList& layerCode();
        const QString& tailCode();
        const QString& layer(int index);

        SettingsPointer G;
        QList<SettingsPointer> ES;
        trimesh::box3 inputBox;

        void clear();
        void load(const QString& fileName, ccglobal::Tracer* tracer = nullptr);
        QString fileName();

        QString sliceName();
        void setSliceName(const QString& sliceName);

        std::vector<QImage> previews;
    private:
        QString m_emptyString;
        QStringList m_data_gcodelayer;//GCodeLayer
        QStringList m_data_gcodeprefix; //GCodePrefix.
        QStringList m_data_gcodetail; //GCodetail.

        QString m_fileName;
        QString m_sliceName;
    };
}

typedef QSharedPointer<cxgcode::SliceResult> SliceResultPointer;
#endif // SLICE_RESULT_H
