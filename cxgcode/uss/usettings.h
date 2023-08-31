#ifndef USS_USETTINGS_1693216858115_H
#define USS_USETTINGS_1693216858115_H
#include "cxgcode/uss/usetting.h"

namespace uss
{
	class CXGCODE_API USettings : public QObject
	{
	public:
		USettings(const crcommon::ParameterMetas& metas, QObject* parent = nullptr);
		virtual ~USettings();

	protected:
		const crcommon::ParameterMetas& m_metas;
	};
}

#endif // USS_USETTINGS_1693216858115_H