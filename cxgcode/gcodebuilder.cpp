#include "gcodebuilder.h"
#include <regex>
#include "thumbnail/thumbnail.h"
#include "gcode/gcodedata.h"

namespace cxgcode
{
	GCodeBuilder::GCodeBuilder()
		:m_tracer(nullptr)
	{

	}

	GCodeBuilder::~GCodeBuilder()
	{

	}

	void GCodeBuilder::build(SliceResultPointer result, ccglobal::Tracer* tracer)
	{
		if (!result)
			return;

		m_tracer = tracer;
		cxsw::parseGCodeInfo(result.get(), parseInfo);
		implBuild(result);
		m_tracer = nullptr;
	}

    bool  regex_match(std::string& gcodeStr,std::string key, std::smatch& sm)
    {
        std::string temp2 = ".*" + key + ":([0-9]{0,8}).*";
        std::string temp1 = ".*" + key + ":([-]{0,1}[0-9]{0,8}\\.[0-9]{0,8}).*";
        if (std::regex_match(gcodeStr, sm, std::regex(temp1.c_str()))||
            std::regex_match(gcodeStr, sm, std::regex(temp2.c_str())))
        {
            return true;
        }
        return false;
    }

    bool  regex_match_time(std::string& gcodeStr, std::smatch& sm, gcode::GCodeParseInfo& parseInfo)
    {
        if (std::regex_match(gcodeStr, sm, std::regex(".*OuterWall Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.OuterWall = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*InnerWall Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.InnerWall = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*Skin Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.Skin = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*Support Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.Support = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*SkirtBrim Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.SkirtBrim = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*Infill Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.Infill = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*Support Infills Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.SupportInfill = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*Combing Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.MoveCombing = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*Retraction Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.MoveRetraction = tmp;
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*PrimeTower Time:([0-9]{0,8}).*")))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.timeParts.PrimeTower = tmp;
        }

        return true;
    }

    void getImage(std::string p, gcode::SliceResult* result)
    {
        std::string tail1 = p;

        size_t pos = p.find(";");
        if (pos > 0 && pos < p.size())
        {
            tail1 = p.substr(pos, p.size());
            tail1.erase(std::remove(tail1.begin(), tail1.end(), ' '), tail1.end());
            tail1.erase(std::remove(tail1.begin(), tail1.end(), ';'), tail1.end());
        }

        std::vector<std::string> prevData;
        prevData.push_back(tail1);
        std::vector<unsigned char> decodeData;
        thumbnail_base2image(prevData, decodeData);

		if (decodeData.size())
		{
			result->previewsData.push_back(decodeData);
		}
    }

    std::string trim(const std::string& str) {
        std::string::const_iterator it = str.begin();
        while (it != str.end() && std::isspace(*it))
            ++it;

        std::string::const_reverse_iterator rit = str.rbegin();
        while (rit.base() != it && std::isspace(*rit))
            ++rit;

        return std::string(it, rit.base());
    }
    

	void GCodeBuilder::implBuild(SliceResultPointer result)
	{

	}

	float GCodeBuilder::traitSpeed(int layer, int step)
	{
		return 0.0f;
	}

	trimesh::vec3 GCodeBuilder::traitPosition(int layer, int step)
	{
		return trimesh::vec3();
	}

	Qt3DRender::QGeometry* GCodeBuilder::buildGeometry()
	{
		return nullptr;
	}

	void GCodeBuilder::updateFlagAttribute(Qt3DRender::QAttribute* attribute, gcode::GCodeVisualType type)
	{
	}
}

//if (preStr.contains(";machine belt offset:"))
//std::string tag2 = ";machine belt offset:";
//float offset = 10.0;
//int pos2 = prevCode.find(tag2);
//if (pos2 >= 0)
//{
//	std::string t = prevCode.substr(pos2 + tag2.length());
//	pos2 = t.find("\n");
//	if (pos2 > 0)
//	{
//		t = t.substr(0, pos2);
//		offset = qtuser_core::StrToFloat(t);
//	}
//}
//
//tag2 = ";machine belt offset Y:";
//float offsetY = 0.0;
//pos2 = prevCode.find(tag2);
//if (pos2 >= 0)
//{
//	std::string t = prevCode.substr(pos2 + tag2.length());
//	pos2 = t.find("\n");
//	if (pos2 > 0)
//	{
//		t = t.substr(0, pos2);
//		offsetY = qtuser_core::StrToFloat(t);
//	}
//}