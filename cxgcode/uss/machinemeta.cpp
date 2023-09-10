#include "machinemeta.h"

namespace uss
{
	MachineMeta::MachineMeta(crslice::ParameterMetas* metas, QObject* parent)
		: QObject(parent)
		, m_metas(metas)
	{

	}

	MachineMeta::~MachineMeta()
	{
		delete m_metas;
	}

	uss::USettings* MachineMeta::create()
	{
		return nullptr;
	}
}