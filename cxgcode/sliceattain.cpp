#include "sliceattain.h"
#include "cxgcode/gcodehelper.h"

#include "qtusercore/string/resourcesfinder.h"
#include "qtusercore/module/systemutil.h"

#include <Qt3DRender/QAttribute>
#include <QtCore/QUuid>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

using namespace qtuser_3d;
namespace cxgcode
{
	SliceAttain::SliceAttain(const QString& slicePath, const QString& tempPath, SliceResultPointer result, SliceAttainType type, QObject* parent)
		:QObject(parent)
		, m_cache(nullptr)
		, m_attribute(nullptr)
		, m_type(type)
		, m_result(result)
		, m_slicePath(slicePath)
		, m_tempPath(tempPath)
	{
		QString path = QString("%1/%2").arg(m_tempPath).arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
		mkMutiDir(path);

		QString name = m_result->sliceName();
		m_tempGCodeFileName = QString("%1/%2").arg(path).arg(name);
		int maxPath = qtuser_core::getSystemMaxPath() - 7;
		if (m_tempGCodeFileName.length() > maxPath)
		{
			m_tempGCodeFileName = m_tempGCodeFileName.left(maxPath);
		}
		m_tempGCodeImageFileName = QString("%1_image").arg(m_tempGCodeFileName);
		if (m_tempGCodeImageFileName.length() > maxPath)
		{
			m_tempGCodeImageFileName = QString("%1_image").arg(m_tempGCodeImageFileName.left(maxPath - 6));
		}
		m_tempGCodeFileName += ".gcode";
		m_tempGCodeImageFileName += ".gcode";
	}

	SliceAttain::~SliceAttain()
	{
		QFileInfo info(m_tempGCodeFileName);
		QString path = info.absolutePath();
		
		clearPath(path);
		QDir dir;
		dir.setPath(path);
		dir.removeRecursively();
	}

	void SliceAttain::build(ccglobal::Tracer* tracer)
	{
		builder.build(m_result, tracer);
	}

	QString SliceAttain::sliceName()
	{
		return m_result->sliceName();
	}

	QString SliceAttain::sourceFileName()
	{
		return m_result->fileName();
	}

	bool SliceAttain::isFromFile()
	{
		return m_type == SliceAttainType::sat_file;
	}

	int SliceAttain::printTime()
	{
		return builder.parseInfo.printTime;
	}

	QString SliceAttain::material_weight()
	{
		return QString::number(builder.parseInfo.materialLenth 
			* builder.parseInfo.materialDensity, 'f', 2);
	}

	QString SliceAttain::printing_time()
	{
		int printTime = builder.parseInfo.printTime;
		QString str = QString("%1h%2m%3s").arg(printTime / 3600)
			.arg(printTime / 60 % 60)
			.arg(printTime % 60);
		return str;
	}

	QString SliceAttain::material_money()
	{
		int nTemp = (builder.parseInfo.materialLenth + 0.005) * 100;
		float materialLength = nTemp / 100.0f;
		return QString::number(materialLength * builder.parseInfo.unitPrice, 'f', 2);
	}

	QString SliceAttain::material_length()
	{
		return QString::number(builder.parseInfo.materialLenth);
	}

	trimesh::box3 SliceAttain::box()
	{
		return m_result->inputBox;
	}

	int SliceAttain::layers()
	{
		return builder.baseInfo.layers;
	}

	int SliceAttain::steps(int layer)
	{
		int _layer = layer - INDEX_START_AT_ONE;

		if (_layer < 0 || _layer >= (int)builder.baseInfo.steps.size())
			return 0;

		return builder.baseInfo.steps.at(_layer);
	}

	int SliceAttain::totalSteps()
	{
		return builder.baseInfo.totalSteps;
	}

	int SliceAttain::nozzle()
	{
		return builder.baseInfo.nNozzle;
	}

	float SliceAttain::minSpeed()
	{
		return builder.baseInfo.speedMin;
	}

	float SliceAttain::maxSpeed()
	{
		return builder.baseInfo.speedMax;
	}

	float SliceAttain::layerHeight()
	{
		return builder.parseInfo.layerHeight;
	}

	float SliceAttain::lineWidth()
	{
		return builder.parseInfo.lineWidth;
	}

	float SliceAttain::traitSpeed(int layer, int step)
	{
		return builder.traitSpeed(layer, step);
	}

	trimesh::vec3 SliceAttain::traitPosition(int layer, int step)
	{
		return builder.traitPosition(layer, step);
	}

	trimesh::fxform SliceAttain::modelMatrix()
	{
		return builder.parseInfo.xf4;
	}

	int SliceAttain::findViewIndexFromStep(int layer, int nStep)
	{
		int _layer = layer - INDEX_START_AT_ONE;
		int _nStep = nStep - INDEX_START_AT_ONE;

		if (_layer >= 0 && _layer < layers())
		{
			const QList<int>& maps = builder.m_stepGCodesMaps.at(_layer);
			if (_nStep >= 0 && _nStep < maps.size())
				return maps.at(_nStep);
		}
		return -1;
	}

	int SliceAttain::findStepFromViewIndex(int layer, int nViewIndex)
	{
		int _layer = layer - INDEX_START_AT_ONE;
		if (_layer >= 0 && _layer < layers())
		{
			const QList<int>& maps = builder.m_stepGCodesMaps.at(_layer);
			return maps.indexOf(nViewIndex);
		}
		return -1;
	}

	void SliceAttain::updateStepIndexMap(int layer)
	{
	}

	const QString& SliceAttain::layerGcode(int layer)
	{
		int _layer = layer - INDEX_START_AT_ONE;
		return m_result->layer(_layer);
	}

	void SliceAttain::setGCodeVisualType(GCodeVisualType type)
	{
		builder.updateFlagAttribute(m_attribute, type);
	}

	Qt3DRender::QGeometry* SliceAttain::createGeometry()
	{
		if (!m_cache)
		{
			m_cache = builder.buildGeometry();
			m_attribute = new Qt3DRender::QAttribute(m_cache);
			m_cache->addAttribute(m_attribute);
		}

		return m_cache;
	}

    float SliceAttain::getMachineHeight()
    {
        return builder.parseInfo.machine_height;
    }

    float SliceAttain::getMachineWidth()
    {
        return builder.parseInfo.machine_width;
    }

    float SliceAttain::getMachineDepth()
    {
        return builder.parseInfo.machine_depth;
    }

    int SliceAttain::getBeltType()
    {
        return builder.parseInfo.beltType;
    }

	Qt3DRender::QGeometry* SliceAttain::createRetractionGeometry()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildRetractionGeometry();
#else
		return nullptr;
#endif
	}

	Qt3DRender::QGeometry* SliceAttain::createZSeamsGeometry()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildZSeamsGeometry();
#else
		return nullptr;
#endif
	}

	Qt3DRender::QGeometryRenderer* SliceAttain::createRetractionGeometryRenderer()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildRetractionGeometryRenderer();
#else
		return nullptr;
#endif
	}

	Qt3DRender::QGeometryRenderer* SliceAttain::createZSeamsGeometryRenderer()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildZSeamsGeometryRenderer();
#else
		return nullptr;
#endif
	}

	void SliceAttain::saveGCode(const QString& fileName, QImage* previewImage)
	{
		QString imageStr;
		if (previewImage)
		{
			float layerHeight = builder.parseInfo.layerHeight;
			QString screenSize = builder.parseInfo.screenSize;
            QString exportFormat = builder.parseInfo.exportFormat;

			previewImage = cxsw::resizeModule(previewImage);
			if (exportFormat == "bmp")
			{
				cxsw::image2String(*previewImage, 64, 64, true, imageStr);
				cxsw::image2String(*previewImage, 400, 400, false, imageStr);
			}
			else
			{
				QImage minPreImg;
				QImage maxPreImg;
				if (screenSize == "Sermoon D3")
				{
					minPreImg = previewImage->scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					maxPreImg = previewImage->scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				else if (screenSize == "CR-10 Inspire")
				{
					minPreImg = previewImage->scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					maxPreImg = previewImage->scaled(600, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				else
				{
					minPreImg = previewImage->scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					maxPreImg = previewImage->scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				cxsw::getImageStr(imageStr, &minPreImg, builder.baseInfo.layers, exportFormat, layerHeight);
				cxsw::getImageStr(imageStr, &maxPreImg, builder.baseInfo.layers, exportFormat, layerHeight, exportFormat == "png");
			}
		}

		cxsw::cxSaveGCode(fileName, imageStr, m_result->layerCode(), m_result->prefixCode(), m_result->tailCode());
	}

	void SliceAttain::saveTempGCode()
	{
		saveGCode(m_tempGCodeFileName, nullptr);
	}

	void SliceAttain::saveTempGCodeWithImage(QImage& image)
	{
		saveGCode(m_tempGCodeImageFileName, &image);
	}

	QString SliceAttain::tempGCodeFileName()
	{
        return m_tempGCodeFileName;
    }

    QString SliceAttain::tempGcodeThumbnail()
    {
        QString exportFormat = builder.parseInfo.exportFormat;
        return QString("%1/imgPreview.%2").arg(m_slicePath).arg(exportFormat);
    }

	QString SliceAttain::tempGCodeImageFileName()
	{
		if (m_tempGCodeImageFileName.isEmpty())
			return m_tempGCodeFileName;
		return m_tempGCodeImageFileName;
	}

	QString SliceAttain::tempImageFileName()
	{
		return QString("%1/slice_flow_gcode_preview.png").arg(m_slicePath);
	}

}


