#ifndef _CXGCODE_NULLSPACE_SLICEMODELBUILDER_1590033290815_H
#define _CXGCODE_NULLSPACE_SLICEMODELBUILDER_1590033290815_H
#include "cxgcode/interface.h"
#include <float.h>

#include <QtCore/QObject>
#include "cxgcode/sliceline.h"
#include "cxgcode/sliceresult.h"
#include "cxgcode/define.h"
#include "ccglobal/tracer.h"
#include "trimesh2/Box.h"

namespace cxgcode
{
	struct GCodeParseInfo;
	struct GCodeStructBaseInfo
	{
		std::vector<int> layerNumbers;
		std::vector<int> steps;
		int totalSteps;
		int layers;

		trimesh::box3 gCodeBox;
		int nNozzle;
		float speedMin;
		float speedMax;

		GCodeStructBaseInfo()
			: speedMin(FLT_MAX)
			, speedMax(FLT_MIN)
			, layers(0)
			, nNozzle(0)
			, totalSteps(0)
		{
		}
	};

	struct GcodeTemperature
	{
		float bedTemperature{ 0.0f }; //平台温度
		float temperature{ 0.0f };  //喷嘴温度
		float camberTemperature{ 0.0f };  //腔体温度
	};

	struct GcodeFan
	{
		float fanSpeed{ 0.0f }; //风扇速度
		float camberSpeed{ 0.0f };  //腔体风扇速度
		float fanSpeed_1{ 0.0f };  //子风扇
	};

	struct GCodeMove
	{
		int start;
		float speed;
		SliceLineType type;
		float e;  //流量
		int extruder;
	};

	class CXGCODE_API GCodeStruct
	{
	public:
		GCodeStruct();
		~GCodeStruct();

		void buildFromResult(SliceResultPointer result, const GCodeParseInfo& info, GCodeStructBaseInfo& baseInfo, QVector<QList<int>>& stepIndexMaps, ccglobal::Tracer* tracer = nullptr);

		std::vector<trimesh::vec3> m_positions;
		std::vector<GCodeMove> m_moves;

		std::vector <GcodeTemperature> m_temperatures;
		std::vector <GcodeFan> m_fans;
		std::vector<int> m_temperatureIndex;
		std::vector<int> m_fanIndex;

		std::vector<int> m_zSeams;
		std::vector<int> m_retractions;
	private:
		void processLayer(const QString& layerCode, int layer, QList<int>& stepIndexMap);
		void processStep(const QString& stepCode, int nIndex, QList<int>& stepIndexMap);
		void processG01(const QString& G01Str, int nIndex, QList<int>& stepIndexMap);
		void processG23(const QString& G23Str, int nIndex, QList<int>& stepIndexMap);
		void processSpeed(float speed);

		void processPrefixCode(const QString& stepCod);
		void checkoutFan(const QString& stepCod);
		void checkoutTemperature(const QString& stepCode);
	protected:
		SliceLineType  tempCurrentType;
		int tempNozzleIndex;
		float tempCurrentE;
		trimesh::vec3 tempCurrentPos;
		float tempSpeed;
		bool layerNumberParseSuccess;

		GCodeParseInfo parseInfo;
		GCodeStructBaseInfo tempBaseInfo;

		ccglobal::Tracer* m_tracer;
	};
}
#endif // _CXGCODE_NULLSPACE_SLICEMODELBUILDER_1590033290815_H
