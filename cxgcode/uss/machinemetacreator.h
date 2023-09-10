#ifndef USS_MACHINEMETACREATOR_1693295990408_H
#define USS_MACHINEMETACREATOR_1693295990408_H
#include "cxgcode/interface.h"
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QHash>

#include "cxgcode/uss/usettings.h"

namespace crcommon
{
	class ParameterMetas;
}

namespace uss
{
	class MachineMeta;
	class CXGCODE_API MachineMetaCreator : public QObject
	{
		Q_OBJECT
	public:
		MachineMetaCreator(QObject* parent = nullptr);
		virtual ~MachineMetaCreator();

		void initialize(const QString& path);  // base
		MachineMeta* create(const QString& fileName);
	protected:
		QString m_basePath;
		QSharedPointer<crslice::ParameterMetas> m_base;
	};
}

#endif // USS_MACHINEMETACREATOR_1693295990408_H