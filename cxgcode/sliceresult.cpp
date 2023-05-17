#include "sliceresult.h"
#include "cxgcode/gcodehelper.h"
#include <QtCore/QDebug>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

#include "qtusercore/string/resourcesfinder.h"

namespace cxgcode
{
    SliceResult::SliceResult() 
    {
        G.reset(new USettings());
        G->loadCompleted();
    }

    SliceResult::~SliceResult()
    {
    }

    const QString& SliceResult::prefixCode()
    {
        if (m_data_gcodeprefix.size() > 0)
            return m_data_gcodeprefix.at(0);
        return m_emptyString;
    }

    const QStringList& SliceResult::layerCode()
    {
        return m_data_gcodelayer;
    }

    const QString& SliceResult::layer(int index)
    {
        if (index < 0 || index >= (int)m_data_gcodelayer.size())
            return m_emptyString;

        return m_data_gcodelayer.at(index);
    }

    const QString& SliceResult::tailCode()
    {
        if (m_data_gcodetail.size() > 0)
            return m_data_gcodetail.at(0);
        return m_emptyString;
    }

    void SliceResult::clear()
    {
        m_data_gcodelayer.clear();
        m_data_gcodeprefix.clear();
    }

	void SliceResult::load(const QString& fileName, ccglobal::Tracer* tracer)
    {
        cxsw::cxLoadGCode(fileName, m_data_gcodelayer, m_data_gcodeprefix, m_data_gcodetail, tracer);
        m_fileName = fileName;
    }

    QString SliceResult::fileName()
    {
        return m_fileName;
    }

    QString SliceResult::sliceName()
    {
        return m_sliceName;
    }

    void SliceResult::setSliceName(const QString& _sliceName)
    {
        m_sliceName = _sliceName;
    }
}
