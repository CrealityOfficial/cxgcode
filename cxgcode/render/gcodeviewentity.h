#ifndef CXGCODE_GCODEVIEWENTITY_1683805686175_H
#define CXGCODE_GCODEVIEWENTITY_1683805686175_H
#include "cxgcode/interface.h"
#include "qtuser3d/refactor/xentity.h"
#include "cxgcode/define.h"

#include <QtGui/QVector4D>
#include <QtGui/QVector2D>
#include <QtGui/QMatrix4x4>
#include <QtGui/QColor>

namespace cxgcode
{
	class CXGCODE_API GCodeViewEntity : public qtuser_3d::XEntity
	{
	public:
		GCodeViewEntity(Qt3DCore::QNode* parent = nullptr);
		virtual ~GCodeViewEntity();

		void setClipValue(const QVector4D& value);
		QList<QColor> getSpeedColorValues();

		void setGCodeVisualType(GCodeVisualType type);
		void setAnimation(int animation);

		void setTypeShow(int typepos, bool needshow);

		void setLayerShowScope(int startlayer, int endlayer);
		void clearLayerShowScope();

		void setLayerStartFlagEnable(bool enable);
		void setModelMatrix(const QMatrix4x4& matrix);

		void setLayerHeight(float height);
		void setLineWidth(float width);
	protected:
		Qt3DRender::QParameter* m_clipValue;
		Qt3DRender::QParameter* m_typeColors;
		Qt3DRender::QParameter* m_speedColors;
		Qt3DRender::QParameter* m_nozzleColors;
		Qt3DRender::QParameter* m_showType;
		Qt3DRender::QParameter* m_animation;
		Qt3DRender::QParameter* m_modelMatrix;

		Qt3DRender::QParameter* m_typecolorsshow;

		Qt3DRender::QParameter* m_layerShow;
		Qt3DRender::QParameter* m_layerStartFlagShow;

		Qt3DRender::QParameter* m_layerHeight;
		Qt3DRender::QParameter* m_lineWidth;

		QVariantList m_showTypeValues;
		QVariantList m_speedColorValues;
	};
}

#endif // CXGCODE_GCODEVIEWENTITY_1683805686175_H