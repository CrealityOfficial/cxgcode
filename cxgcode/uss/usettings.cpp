#include "usettings.h"

namespace uss
{
	USettings::USettings(const crslice::ParameterMetas& metas, QObject* parent)
		: QObject(parent)
		, m_metas(metas)
	{

	}

	USettings::~USettings()
	{

	}
}