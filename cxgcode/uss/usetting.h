#ifndef USS_USETTING_1693216858123_H
#define USS_USETTING_1693216858123_H
#include "cxgcode/interface.h"
#include "crcommon/base/parametermeta.h"
#include <QtCore/QObject>

namespace uss
{
	class CXGCODE_API USetting : public QObject
	{
	public:
		USetting(const crcommon::ParameterMeta& meta, QObject* parent = nullptr);
		virtual ~USetting();

	protected:
		const crcommon::ParameterMeta& m_meta;
	};
}

#endif // USS_USETTING_1693216858123_H