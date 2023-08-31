#ifndef USS_MACHINEMETA_1693295990409_H
#define USS_MACHINEMETA_1693295990409_H
#include "cxgcode/uss/usettings.h"
#include "crcommon/base/parametermeta.h"

namespace uss
{
	class CXGCODE_API MachineMeta : public QObject
	{
		Q_OBJECT
	public:
		MachineMeta(crcommon::ParameterMetas* metas, QObject* parent = nullptr);
		virtual ~MachineMeta();

		uss::USettings* create();
	protected:
		crcommon::ParameterMetas* m_metas;  //own it
	};
}

#endif // USS_MACHINEMETA_1693295990409_H