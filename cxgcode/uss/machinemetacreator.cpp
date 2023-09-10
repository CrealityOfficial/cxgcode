#include "machinemetacreator.h"
#include "crslice/base/parametermeta.h"
#include "cxgcode/uss/machinemeta.h"
#include <QtCore/QDebug>

namespace uss
{
	MachineMetaCreator::MachineMetaCreator(QObject* parent)
		: QObject(parent)
	{

	}

	MachineMetaCreator::~MachineMetaCreator()
	{

	}

	void MachineMetaCreator::initialize(const QString& path)
	{
		if (m_base)
		{
			qDebug() << QString("MachineMetaCreator::initialize already");
			return;
		}

		m_basePath = path;
		m_base.reset(new crslice::ParameterMetas());
		m_base->initializeBase(path.toStdString());
	}

	MachineMeta* MachineMetaCreator::create(const QString& fileName)
	{
		if (!m_base)
		{
			qWarning() << QString("initialize before MachineMetaCreator::create.");
			return nullptr;
		}

		MachineMeta* meta = new MachineMeta(nullptr);
		return meta;
	}
}