#ifndef _CXGCODE_US_JSONDEF_1589439749372_H
#define _CXGCODE_US_JSONDEF_1589439749372_H
#include "cxgcode/interface.h"
#include "cxgcode/us/settinggroupdef.h"
#include <QtCore/QMap>

namespace us
{
	class CXGCODE_API JsonDef: public QObject
	{
	public:
		JsonDef(QObject* parent = nullptr);
		virtual ~JsonDef();

		void parse(const QString& file, QMap<QString, SettingGroupDef*>& defs, QObject* parent);
	};
}
#endif // _CXGCODE_US_JSONDEF_1589439749372_H
