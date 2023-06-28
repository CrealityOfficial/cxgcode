#include "slicemodelbuilder.h"
#include "cxgcode/sliceresult.h"
#include "cxgcode/sliceattain.h"

#include "trimesh2/XForm.h"
#include "mmesh/trimesh/trimeshutil.h"
#include "mmesh/trimesh/algrithm3d.h"
#include <math.h>

namespace cxgcode
{
    inline SliceLineType GetLineType(const QString& strLineType)
    {
        if (strLineType.contains(";TYPE:WALL-OUTER"))
        {
            return SliceLineType::OuterWall;
        }
        else if (strLineType.contains(";TYPE:WALL-INNER"))
        {
            return SliceLineType::InnerWall;
        }
        else if (strLineType.contains(";TYPE:SKIN"))
        {
            return SliceLineType::Skin;
        }
        else if (strLineType.contains(";TYPE:SUPPORT-INTERFACE"))
        {
            return SliceLineType::SupportInterface;
        }
        else if (strLineType.contains(";TYPE:SUPPORT-INFILL"))
        {
            return SliceLineType::SupportInfill;
        }
        else if (strLineType.contains(";TYPE:SUPPORT"))
        {
            return SliceLineType::Support;
        }
        else if (strLineType.contains(";TYPE:SKIRT"))
        {
            return SliceLineType::SkirtBrim;
        }
        else if (strLineType.contains(";TYPE:FILL"))
        {
            return SliceLineType::Infill;
        }
        else if (strLineType.contains(";TYPE:PRIME-TOWER"))
        {
            return SliceLineType::PrimeTower;
        }
        else if (strLineType.contains(";TYPE:Slow-Flow-Types"))
        {
            return SliceLineType::FlowTravel;
        }
        else if (strLineType.contains(";TYPE:Flow-In-Advance-Types"))
        {
            return SliceLineType::AdvanceTravel;
        }
        else
        {
            return SliceLineType::NoneType;
        }
    }

    GCodeStruct::GCodeStruct()
        : tempCurrentType(SliceLineType::NoneType)
        , tempNozzleIndex(0)
        , tempCurrentE(0.0f)
        , tempSpeed(0.0f)
        , layerNumberParseSuccess(true)
        , m_tracer(nullptr)
    {
        m_positions.push_back(tempCurrentPos);
    }

    GCodeStruct::~GCodeStruct()
    {

    }

    void GCodeStruct::processLayer(const QString& layerCode, int layer, QList<int>& stepIndexMap)
    {
        QStringList layerLines = layerCode.split("\n");

        int startNumber = (int)m_moves.size();
        if (layerNumberParseSuccess && layerLines.size() > 0)
        {
            QStringList layerNumberStr = layerLines[0].split(":");
            if (layerNumberStr.size() > 1)
            {
                int index = layerNumberStr[1].toInt(&layerNumberParseSuccess);
                if (layerNumberParseSuccess)
                    tempBaseInfo.layerNumbers.push_back(index);
            }
            else
            {
                layerNumberParseSuccess = false;
            }
        }
        else
        {
            layerNumberParseSuccess = false;
        }

        int nIndex = 0;
        for (const QString& layerLine : layerLines)
        {
            QString stepCode = layerLine.trimmed();  //" \n\r\t"
            if (stepCode.isEmpty())
                continue;

            checkoutLayerInfo(layerLine, layer);
            processStep(stepCode, nIndex, stepIndexMap);
            ++nIndex;
        }

        tempBaseInfo.steps.push_back((int)m_moves.size() - startNumber);
    }

    void GCodeStruct::checkoutFan(const QString& stepCode)
    {
        //std::vector <GcodeTemperature> m_temperatures;
        if (stepCode.contains("M106")
            || stepCode.contains("M107"))
        {
            GcodeFan gcodeFan = m_fans.size() > 0 ? m_fans.back() : GcodeFan();
            QStringList strs = stepCode.split(" ");

            float speed = 0.0f;
            int type = -1;
            for (const QString& it3 : strs)
            {
                QString componentStr = it3.trimmed();
                if (componentStr.isEmpty())
                    continue;

                if (componentStr[0] == "S")
                {
                    speed = componentStr.mid(1).toFloat();
                }

                if (!it3.compare("M106"))
                {
                    if (!it3.compare("P1"))
                        type = 1;
                    else if(!it3.compare("P2"))
                        type = 2;
                    else
                        type = 0;
                }
                else if (!it3.compare("M107"))
                {
                    if (!it3.compare("P1"))
                        type = 4;
                    else if (!it3.compare("P2"))
                        type = 5;
                    else
                        type = 3;
                }
            }
            switch (type)
            {
            case 0:
                gcodeFan.fanSpeed = speed;
                break;
            case 1:
                gcodeFan.camberSpeed = speed;
                break;
            case 2:
                gcodeFan.fanSpeed_1 = speed;
                break;
            case 3:
                gcodeFan.fanSpeed = 0.0f;
                break;
            case 4:
                gcodeFan.camberSpeed = 0.0f;
                break;
            case 5:
                gcodeFan.fanSpeed_1 = 0.0f;
                break;
            default:
                break;
            }

            if (type >= 0)
            {
                m_fans.push_back(gcodeFan);
            }
        }

    }

    void GCodeStruct::checkoutTemperature(const QString& stepCode)
    {
        //std::vector <GcodeTemperature> m_temperatures;
        if (stepCode.contains("M140")
            || stepCode.contains("M190")
            || stepCode.contains("M104")
            || stepCode.contains("M109")
            || stepCode.contains("M141")
            || stepCode.contains("M191")
            || stepCode.contains("EXTRUDER_TEMP")
            || stepCode.contains("BED_TEMP")
            )
        {
            GcodeTemperature gcodeTemperature= m_temperatures.size()>0 ? m_temperatures.back(): GcodeTemperature();
            QStringList strs = stepCode.split(" ");

            float temperature = 0.0f;
            int type = -1;
            for (const QString& it3 : strs)
            {
                QString componentStr = it3.trimmed();
                if (componentStr.isEmpty())
                    continue;

                if (componentStr[0] == "S")
                {
                    temperature = componentStr.mid(1).toFloat();
                }
                else if (componentStr.contains("EXTRUDER_TEMP"))
                {
                    QStringList strs = componentStr.split("=");
                    if (strs.size() >= 2)
                    {
                        temperature = strs[1].toFloat();
                    }
                    gcodeTemperature.temperature = temperature;
                    type = 4;
                }
                else if (componentStr.contains("BED_TEMP"))
                {
                    QStringList strs = componentStr.split("=");
                    if (strs.size() >= 2)
                    {
                        temperature = strs[1].toFloat();
                    }
                    gcodeTemperature.bedTemperature = temperature;
                    type = 4;
                }

                if (!it3.compare("M140")
                    || !it3.compare("M190"))
                {
                    type = 0;
                }
                else if (!it3.compare("M104")
                    || !it3.compare("M109") )
                {
                    if (!it3.compare("T1")
                        || !it3.compare("t1"))
                        type = 1;
                    else
                        type = 2;
                }
                else if (!it3.compare("M141")
                    || !it3.compare("M191"))
                {
                    type = 3;
                }
            }
            switch (type)
            {
            case 0:
                gcodeTemperature.bedTemperature = temperature;
                break;
            case 1:
                //gcodeTemperature.temperature0 = temperature;
                //break;
            case 2:
                gcodeTemperature.temperature = temperature;
                break;
            case 3:
                gcodeTemperature.camberTemperature = temperature;
                break;
            case 4:
            default:
                break;
            }

            if (type >= 0)
            {
                m_temperatures.push_back(gcodeTemperature);
            }
        }

    }

    void GCodeStruct::checkoutLayerInfo(const QString& stepCode, int layer)
    {
        //;TIME_ELAPSED:548.869644
        if (stepCode.contains("TIME_ELAPSED"))
        {
            QStringList strs = stepCode.split(":");
            if (strs.size() >= 2)
            {
                float temp = strs[1].toFloat() - tempCurrentTime;
                float templog = 0.0f;
                //if (temp >0)
                //{
                //    templog = std::log(temp);
                //}
                //m_layerTimeLogs.insert(std::pair<int, float>(layer, templog));
                m_layerTimes.insert(std::pair<int,float>(layer, temp));
                tempCurrentTime = strs[1].toFloat();
            }
        }
    }

    void GCodeStruct::processPrefixCode(const QString& stepCod)
    {
        QStringList layerLines = stepCod.split("\n");

        for (const QString& layerLine : layerLines)
        {
            QString stepCode = layerLine.trimmed();  //" \n\r\t"
            if (stepCode.isEmpty())
                continue;

            if (stepCode.contains("M140")
                || stepCode.contains("M190")
                || stepCode.contains("M104")
                || stepCode.contains("M109")
                || stepCode.contains("M141")
                || stepCode.contains("M191")
                || stepCode.contains("EXTRUDER_TEMP")
                || stepCode.contains("BED_TEMP")
                || stepCode.contains("M106")
                || stepCode.contains("M107"))
            {
                checkoutFan(stepCode);
                checkoutTemperature(stepCode);
            }
        }
    }

    void GCodeStruct::processStep(const QString& stepCode, int nIndex, QList<int>& stepIndexMap)
    {
        checkoutTemperature(stepCode);
        checkoutFan(stepCode);

        if (stepCode.contains(";TYPE:") && !stepCode.contains(";TYPE:end"))
        {
            tempCurrentType = GetLineType(stepCode);
        }
        if (stepCode.at(0) == "T")
        {
            QString nozzleIndexStr = stepCode.mid(1);
            if (nozzleIndexStr.size() > 0)
            {
                bool success = false;
                int index = nozzleIndexStr.toInt(&success);
                if (success)
                    tempNozzleIndex = index;
            }

            if (tempBaseInfo.nNozzle < tempNozzleIndex + 1)
                tempBaseInfo.nNozzle = tempNozzleIndex + 1;
        }
        else if (stepCode[0] == 'G' && stepCode.size() > 1)
        {
            if (stepCode[1] == '2' || stepCode[1] == '3')
            {
                processG23(stepCode, nIndex, stepIndexMap);
            }
            else if (stepCode[1] == '0' || stepCode[1] == '1')
            {
                processG01(stepCode, nIndex, stepIndexMap);
            }
            else if (stepCode.contains("G92"))
            {
                tempCurrentE = 0.0f;
            }
        }
    }

    void GCodeStruct::processG01(const QString& G01Str, int nIndex, QList<int>& stepIndexMap)
    {
        QStringList G01Strs = G01Str.split(" ");

        trimesh::vec3 tempEndPos = tempCurrentPos;
        float tempEndE = tempCurrentE;
        SliceLineType tempType = tempCurrentType;
        bool havaXYZ = false;

        GcodeLayerInfo gcodeLayerInfo = m_gcodeLayerInfos.size() > 0 ? m_gcodeLayerInfos.back() : GcodeLayerInfo();
        for (const QString& it3 : G01Strs)
        {
            QString componentStr = it3.trimmed();
            //it4 ==G1 / F4800 / X110.125 / Y106.709 /Z0.6 /E575.62352
            if (componentStr.isEmpty())
                continue;

            if (componentStr[0] == "F")
            {
                tempSpeed = componentStr.mid(1).toFloat();
            }
            else if (componentStr[0] == "E" || componentStr[0] == "P")
            {
                float e = componentStr.mid(1).toFloat();
                if (parseInfo.relativeExtrude)
                    tempEndE += e;
                else
                    tempEndE = e;
            }
            else if (componentStr[0] == "X")
            {
                tempEndPos.at(0) = componentStr.mid(1).toFloat();
                havaXYZ = true;
            }
            else if (componentStr[0] == "Y")
            {
                tempEndPos.at(1) = componentStr.mid(1).toFloat();
                havaXYZ = true;
            }
            else if (componentStr[0] == "Z")
            {
                tempEndPos.at(2) = componentStr.mid(1).toFloat();
                havaXYZ = true;

                //add layerHeight
                GcodeLayerInfo  gcodeLayerInfo = m_gcodeLayerInfos.size() > 0 ? m_gcodeLayerInfos.back() : GcodeLayerInfo();
                float layerHight = tempEndPos[2] - tempCurrentZ ;
                tempCurrentZ = tempEndPos[2];
                if (layerHight > 0 || m_gcodeLayerInfos.empty())
                {
                    if (layerHight < 0.1)
                    {
                        int test = 0;
                    }
                    if (layerHight > 0.3f)
                    {
                        int test = 0;
                    }
                    gcodeLayerInfo.layerHight = layerHight + 0.001f;
                    m_gcodeLayerInfos.push_back(gcodeLayerInfo);
                }
            }
        }

        if (tempEndE == tempCurrentE)
        {
            if (G01Str[1] == '0' || havaXYZ)
            {
                tempType = SliceLineType::Travel;
            }
        }
        else if (tempEndE < tempCurrentE)
        {
            if (havaXYZ)
                tempType = SliceLineType::Travel;
            else
            {
                tempType = SliceLineType::React;
                m_retractions.push_back(m_positions.size()-1);
            }
                
        }

        if (havaXYZ)
        {
            int index = (int)m_positions.size();
            m_positions.push_back(tempEndPos);
            GCodeMove move;
            move.start = index - 1;
            move.speed = tempSpeed;
            move.e = tempEndE - tempCurrentE;
            move.type = tempType;
            if (move.e == 0.0f)
                move.type = SliceLineType::Travel;
            else if (move.e >0.0f && move.type==SliceLineType::OuterWall && m_moves.back().type==SliceLineType::Travel)
            {
                m_zSeams.push_back(move.start);
            }

            move.extruder = tempNozzleIndex;
            m_moves.emplace_back(move);

            //add temperature fan and time ...
            if (m_temperatures.empty())
            {
                m_temperatures.push_back(GcodeTemperature());
            }
            if (m_fans.empty())
            {
                m_fans.push_back(GcodeFan());
            }
            if (m_gcodeLayerInfos.empty())
            {
                m_gcodeLayerInfos.push_back(GcodeLayerInfo());
            }

            //calculate width
            trimesh::vec3 offset = tempCurrentPos - tempEndPos;
            offset.z = 0;
            float len = trimesh::length(offset);
            float len1 = std::sqrt(offset.x * offset.x + offset.y * offset.y);
            float h = m_gcodeLayerInfos.back().layerHight;

            float width = 0.0f;
            if (len !=0 && h != 0 && move.e >0.0f)
            {
                width = move.e*2.405 / len / h;
            }

            if (std::abs(m_gcodeLayerInfos.back().width - width) > 0.001)
            {
                GcodeLayerInfo  gcodeLayerInfo = m_gcodeLayerInfos.size() > 0 ? m_gcodeLayerInfos.back() : GcodeLayerInfo();
                gcodeLayerInfo.width = width;
                m_gcodeLayerInfos.push_back(gcodeLayerInfo);
            }
            //end

            m_temperatureIndex.push_back(m_temperatures.size()-1);
            m_fanIndex.push_back(m_fans.size() - 1);
            m_layerInfoIndex.push_back(m_gcodeLayerInfos.size() - 1);
            //end

            stepIndexMap.append(nIndex);

            tempBaseInfo.gCodeBox += tempEndPos;

            if(tempType != SliceLineType::Travel)
                processSpeed(tempSpeed);
        }

        tempCurrentPos = tempEndPos;
        tempCurrentE = tempEndE;
    }

    void GCodeStruct::processG23(const QString& G23Code, int nIndex, QList<int>& stepIndexMap)
    {
        QStringList G23Strs = G23Code.split(" ");//G1 Fxxx Xxxx Yxxx Exxx
        //G3 F1500 X118.701 Y105.96 I9.55 J1.115 E7.96039
        bool isG2 = true;
        if (G23Code[1] == '3')
            isG2 = false;

        bool bIsTravel = true;
        float f = 0.0f;
        float e = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float i = 0.0f;
        float j = 0.0f;
        bool  bcircles = false;
        for (const QString& it3 : G23Strs)
        {
            //it4 ==G1 / F4800 / X110.125 / Y106.709 /Z0.6 /E575.62352
            QString G23Str = it3.trimmed();
            if (G23Str.isEmpty())
                continue;

            if (G23Str[0] == "E")
            {
                e = G23Str.mid(1).toFloat();
                if (e > 0)
                {
                    if(!parseInfo.relativeExtrude)
                        e = e - tempCurrentE;
                    bIsTravel = false;
                }
            }
            else if (G23Str[0] == "F")
            {
                f = G23Str.mid(1).toFloat();
            }
            else if (G23Str[0] == "X")
            {
                x = G23Str.mid(1).toFloat();
            }
            else if (G23Str[0] == "Y")
            {
                y = G23Str.mid(1).toFloat();
            }
            else if (G23Str[0] == "I")
            {
                i = G23Str.mid(1).toFloat();
            }
            else if (G23Str[0] == "J")
            {
                j = G23Str.mid(1).toFloat();
            }
            else if (G23Str[0] == "P")
            {
                bcircles = true;
                bIsTravel = true;
                if (x == 0 && y == 0)
                {
                    x = tempCurrentPos.x;
                    y = tempCurrentPos.y;
                }
            }
        }

        //for test
        //if (qFuzzyCompare(x, 103.334f) && qFuzzyCompare(y, 105.998f))
        //    int test = 0;

        trimesh::vec3 circlePos = tempCurrentPos;
        circlePos.x += i;
        circlePos.y += j;
        trimesh::vec3 circleEndPos = tempCurrentPos;
        circleEndPos.x = x;
        circleEndPos.y = y;

        float theta = 0.0f;
        std::vector<trimesh::vec> out;
        if (isG2)
        {
            theta = mmesh::getAngelOfTwoVector(tempCurrentPos, circleEndPos, circlePos);
        }
        else
        {
            theta = mmesh::getAngelOfTwoVector(circleEndPos, tempCurrentPos, circlePos);
        }
        if (bcircles)
        {
            theta = 360;
        }
        mmesh::getDevidePoint(circlePos, tempCurrentPos, out, theta, isG2);
        out.push_back(circleEndPos);
        float devideE = e;
        if (out.size() > 0)
        {
            devideE = e / out.size();
        }

        QStringList G23toG1s;
        for (size_t ii = 0; ii < out.size(); ii++)
        {
            std::string devideTemp = "";
            if (bIsTravel)
            {
                devideTemp += "G0 ";
            }
            else
            {
                devideTemp += "G1 ";
            }
            if (f)
            {
                char itc[20];
                sprintf(itc, "F%0.1f ", f);
                devideTemp += itc;
            }
            char itcx[20];
            sprintf(itcx, "X%0.3f ", out[ii].x);
            devideTemp += itcx;
            char itcy[20];
            sprintf(itcy, "Y%0.3f ", out[ii].y);
            devideTemp += itcy;
            if (!bIsTravel)
            {
                char itce[20];
                double ce = parseInfo.relativeExtrude ? devideE : tempCurrentE + devideE * (ii + 1);
                sprintf(itce, "E%0.5f", ce);
                devideTemp += itce;
            }

            G23toG1s.push_back(devideTemp.c_str());
        }

        for (const QString& G23toG01 : G23toG1s)
        {
            processG01(G23toG01, nIndex, stepIndexMap);
        }
    }

    void GCodeStruct::processSpeed(float speed)
    {
        if (tempBaseInfo.speedMax < speed)
            tempBaseInfo.speedMax = speed;
        if (tempBaseInfo.speedMin > speed)
            tempBaseInfo.speedMin = speed;
    }

    void GCodeStruct::buildFromResult(SliceResultPointer result, const GCodeParseInfo& info,
        GCodeStructBaseInfo& baseInfo, QVector<QList<int>>& stepIndexMaps, ccglobal::Tracer* tracer)
    {
        m_tracer = tracer;

        //get temperature and fan
        processPrefixCode(result->prefixCode());

        parseInfo = info;
        tempBaseInfo.nNozzle = 1;
        int layer = 0;
        int layerCount = (int)result->layerCode().size();
        for (const QString& it : result->layerCode())
        {
            QList<int> stepIndexMap;
            QString layerCode = it.trimmed();
            processLayer(layerCode, layer, stepIndexMap);
            stepIndexMaps.push_back(stepIndexMap);
            ++layer;

            if (m_tracer)
            {
                m_tracer->progress((float)layer / (float)layerCount);
                if (m_tracer->interrupt())
                {
                    m_tracer->failed("GCodeStruct::buildFromResult interrupt.");
                    layerNumberParseSuccess = false;
                    break;
                }
            }
        }

        tempBaseInfo.totalSteps = (int)m_moves.size();
        tempBaseInfo.layers = (int)tempBaseInfo.layerNumbers.size();
        if (!layerNumberParseSuccess)
        {
            tempBaseInfo.layerNumbers.clear();
            tempBaseInfo.layers = 1;
            tempBaseInfo.steps.clear();
            tempBaseInfo.steps.push_back(tempBaseInfo.totalSteps);
            if (stepIndexMaps.size() > 0)
            {
                stepIndexMaps.resize(1);
            }
        }

        float minFlow = FLT_MAX, maxFlow = FLT_MIN;
        float d = tempBaseInfo.speedMax - tempBaseInfo.speedMin;
        for (GCodeMove& move : m_moves) 
        {
            move.speed = d > 0 ? (move.speed - tempBaseInfo.speedMin) / d : 0;

            if (move.e > 0)
            {
                minFlow = fminf(move.e, minFlow);
                maxFlow = fmaxf(move.e, maxFlow);
            }
        }
        tempBaseInfo.minFlowOfStep = minFlow;
        tempBaseInfo.maxFlowOfStep = maxFlow;

        float minTime = FLT_MAX, maxTime = FLT_MIN;
        for each (auto t in m_layerTimes)
        {
            float time = t.second;
            minTime = fminf(time, minTime);
            maxTime = fmaxf(time, maxTime);
        }
        tempBaseInfo.minTimeOfLayer = minTime;
        tempBaseInfo.maxTimeOfLayer = maxTime;

        baseInfo = tempBaseInfo;
    }
}

