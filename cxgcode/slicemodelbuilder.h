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
		float minTimeOfLayer;   //�������ʱ��
		float maxTimeOfLayer;   //�����ʱ��

		float minFlowOfStep;  //������С����������
		float maxFlowOfStep;  //�����������

		float minLineWidth;
		float maxLineWidth;

		float minLayerHeight;
		float maxLayerHeight;

		float minTemperature;
		float maxTemperature;

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
			, minTimeOfLayer(FLT_MAX)
			, maxTimeOfLayer(FLT_MIN)
			, minFlowOfStep(FLT_MAX)
			, maxFlowOfStep(FLT_MIN)
			, minLineWidth(FLT_MAX)
			, maxLineWidth(FLT_MIN)
			, minLayerHeight(FLT_MAX)
			, maxLayerHeight(FLT_MIN)
			, minTemperature(FLT_MAX)
			, maxTemperature(FLT_MIN)
		
		{
		}
	};

	struct GcodeTemperature
	{
		float bedTemperature{ 0.0f }; //ƽ̨�¶�
		float temperature{ 0.0f };  //�����¶�
		float camberTemperature{ 0.0f };  //ǻ���¶�
	};

	struct GcodeFan
	{
		float fanSpeed{ 0.0f }; //�����ٶ�
		float camberSpeed{ 0.0f };  //ǻ������ٶ�
		float fanSpeed_1{ 0.0f };  //�ӷ���
	};

	struct GcodeLayerInfo
	{
		float layerHight{ 0.0f }; //���
		float width{ 0.0f };  //�߿�
		float flow{ 0.0f }; //�������
	};

	struct GCodeMove
	{
		int start;
		float speed;
		SliceLineType type;
		float e;  //����
		int extruder;
	};

	class CXGCODE_API GCodeStruct
	{
	public:
		GCodeStruct();
		~GCodeStruct();

		void buildFromResult(SliceResultPointer result, const GCodeParseInfo& info, GCodeStructBaseInfo& baseInfo, QVector<QList<int>>& stepIndexMaps, ccglobal::Tracer* tracer = nullptr);

		std::vector<trimesh::vec3> m_positions;
		std::vector<GCodeMove> m_moves;  //�������ٶ�..

		std::vector <GcodeTemperature> m_temperatures;//�¶�����ֵ
		std::vector<int> m_temperatureIndex;//�¶Ȳ�������
		std::vector <GcodeFan> m_fans;//��������ֵ
		std::vector<int> m_fanIndex;//���Ȳ�������
		std::vector <GcodeLayerInfo> m_gcodeLayerInfos;  //��ߡ��߿�����ֵ
		std::vector<int> m_layerInfoIndex;  //��ߡ��߿� ��������

		std::map<int,float> m_layerTimes;  //ÿ��ʱ��
		//std::map<int, float> m_layerTimeLogs;  //ÿ��ʱ�����

		std::vector<int> m_zSeams;
		std::vector<int> m_retractions;
	private:
		void processLayer(const QString& layerCode, int layer, QList<int>& stepIndexMap);
		void processStep(const QString& stepCode, int nIndex, QList<int>& stepIndexMap);
		void processG01(const QString& G01Str, int nIndex, QList<int>& stepIndexMap,bool isG2G3 =false);
		void processG23(const QString& G23Str, int nIndex, QList<int>& stepIndexMap);
		void processSpeed(float speed);

		void processPrefixCode(const QString& stepCod);
		void checkoutFan(const QString& stepCod);
		void checkoutTemperature(const QString& stepCode);
		void checkoutLayerInfo(const QString& stepCode,int layer);
		void checkoutLayerHeight(const QStringList& layerLines);
	protected:
		SliceLineType  tempCurrentType;
		int tempNozzleIndex;
		float tempCurrentE;
		float tempCurrentTime{ 0.0f };//time
		float tempCurrentZ{ 0.0f };
		float belowZ{ 0.0f };
		trimesh::vec3 tempCurrentPos;
		float tempSpeed;
		float tempSpeedMax{ 0.0f };//����ٶ�����
		int tempTempIndex{ 0 };; //��ǰ�¶�����
		bool layerNumberParseSuccess;

		GCodeParseInfo parseInfo;
		GCodeStructBaseInfo tempBaseInfo;

		ccglobal::Tracer* m_tracer;
	};
}
#endif // _CXGCODE_NULLSPACE_SLICEMODELBUILDER_1590033290815_H
