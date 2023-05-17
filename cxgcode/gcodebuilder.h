#ifndef CREATIVE_KERNEL_GCODEBUILDER_1677068549648_H
#define CREATIVE_KERNEL_GCODEBUILDER_1677068549648_H

#include "cxgcode/define.h"
#include "cxgcode/sliceresult.h"
#include "cxgcode/slicemodelbuilder.h"
#include "qtuser3d/geometry/attribute.h"
#include "trimesh2/Box.h"

#include "ccglobal/tracer.h"

#include <Qt3DRender/QGeometry>
#define PI 3.1415926535

namespace cxgcode
{
	class CXGCODE_API GCodeBuilder
	{
	public:
		GCodeBuilder();
		virtual ~GCodeBuilder();

		void build(SliceResultPointer result, ccglobal::Tracer* tracer = nullptr);

		virtual float traitSpeed(int layer, int step);
		virtual trimesh::vec3 traitPosition(int layer, int step);
		virtual Qt3DRender::QGeometry* buildGeometry();
		virtual void updateFlagAttribute(Qt3DRender::QAttribute* attribute, GCodeVisualType type);
	protected:
		virtual void implBuild(SliceResultPointer result);
		void parseGCodeInfo(SliceResultPointer result);
	public:
		GCodeParseInfo parseInfo;
		GCodeStructBaseInfo baseInfo;
		QVector<QList<int>> m_stepGCodesMaps;

		ccglobal::Tracer* m_tracer;
	};
}

#endif // CREATIVE_KERNEL_GCODEBUILDER_1677068549648_H