#include "gcodebuilder.h"
#include <regex>
#include "thumbnail/thumbnail.h"

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
		parseGCodeInfo(result);
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
    
	void GCodeBuilder::parseGCodeInfo(SliceResultPointer result)
	{
		std::string gcodeStr = result->prefixCode();

		std::replace(gcodeStr.begin(), gcodeStr.end(), '\n', ' ');
		std::replace(gcodeStr.begin(), gcodeStr.end(), '\r', ' ');

		std::smatch sm;

        result->previewsData.clear();
        if (std::regex_match(gcodeStr, sm, std::regex(".*jpg begin(.*)jpg end.*jpg begin(.*)jpg end.*"))) //jpg
        {
            getImage(sm[1], result.get());
            if (sm.size() > 2)
            {
                getImage(sm[2], result.get());
            }
        }
        else  if (std::regex_match(gcodeStr, sm, std::regex(".*png begin(.*)png end.*png begin(.*)png end.*"))) //png
        {
            getImage(sm[1], result.get());
            if (sm.size() > 2)
            {
                getImage(sm[2], result.get());
            }
        }
        else  if (std::regex_match(gcodeStr, sm, std::regex(".*bmp begin(.*)bmp end.*bmp begin(.*)bmp end.*"))) //bmp
        {
            getImage(sm[1], result.get());
            if (sm.size() > 2)
            {
                getImage(sm[2], result.get());
            }
        }
        if (std::regex_match(gcodeStr, sm, std::regex(".*thumbnail begin(.*)thumbnail end.*"))) //thumbnail
        {
            getImage(sm[1], result.get());
        }     

		if (std::regex_match(gcodeStr, sm, std::regex(".*TIME:([0-9]{0,8}).*"))) ////get print time
		{
			std::string tStr = sm[1];
			int tmp = atoi(tStr.c_str());
			parseInfo.printTime = tmp;

            regex_match_time(gcodeStr, sm, parseInfo);
		}
		if (std::regex_match(gcodeStr, sm, std::regex(".*Filament used:([0-9]{0,8}\\.[0-9]{0,8}).*"))) ////get print time
		{
			std::string tStr = sm[1];
			float tmp = atof(tStr.c_str());
			parseInfo.materialLenth = tmp;
		}

        if (regex_match(gcodeStr, "machine belt offset", sm))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.beltOffset = tmp;
        }

        if (regex_match(gcodeStr, "machine belt offset Y", sm))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.beltOffsetY = tmp;
        }

        bool hasMachineSpaceBox = false;
        if (regex_match(gcodeStr, "Machine Height", sm))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.machine_height = tmp;
        }
        else
            hasMachineSpaceBox = true;

        if (regex_match(gcodeStr, "Machine Width", sm))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.machine_width = tmp;
        }
        else
            hasMachineSpaceBox = true;

        if (regex_match(gcodeStr, "Machine Depth", sm))
        {
            std::string tStr = sm[1];
            float tmp = atof(tStr.c_str());
            parseInfo.machine_depth = tmp;
        }
        else
            hasMachineSpaceBox = true;

        if (hasMachineSpaceBox)
        {//获取模型尺寸信息
            if (regex_match(gcodeStr, "MAXX", sm))
            {
                std::string tStr = sm[1];
                parseInfo.machine_width = atof(tStr.c_str()) + 20; //gap
            }

            if (regex_match(gcodeStr, "MAXY", sm))
            {
                std::string tStr = sm[1];
                parseInfo.machine_depth = atof(tStr.c_str()) + 20; //gap
            }

            if (regex_match(gcodeStr, "MAXZ", sm))
            {
                std::string tStr = sm[1];
                parseInfo.machine_height = atof(tStr.c_str()) + 20; //gap
            }
        }

        if (gcodeStr.find("M83") != std::string::npos)
        {
            bool relativeExtrude = true;
        }

		float material_diameter = 1.75;
		float material_density = 1.24;
		if (regex_match(gcodeStr, "Material Diameter", sm))
		{
			std::string tStr = sm[1];
            parseInfo.material_diameter = atof(tStr.c_str()); //gap
		}
		if (regex_match(gcodeStr, "Material Density", sm))
		{
			std::string tStr = sm[1];
            parseInfo.material_density = atof(tStr.c_str()); //gap
		}

        //单位面积密度
		parseInfo.materialDensity = PI * (parseInfo.material_diameter * 0.5) * (parseInfo.material_diameter * 0.5) * parseInfo.material_density;

		float filament_cost = 0.0;
		if (regex_match(gcodeStr, "Filament Cost", sm))
		{
			std::string tStr = sm[1];
			filament_cost = atof(tStr.c_str()); //gap
		}
		float filament_weight = 0.0;
		if (regex_match(gcodeStr, "Filament Weight", sm))
		{
			std::string tStr = sm[1];
			filament_weight = atof(tStr.c_str()); //gap
		}

        float filament_length = filament_weight /  parseInfo.materialDensity;
        parseInfo.unitPrice = filament_cost / filament_length;

		parseInfo.lineWidth = 0.4;
		if (regex_match(gcodeStr, "Out Wall Line Width", sm))
		{
			std::string tStr = sm[1];
			parseInfo.lineWidth = atof(tStr.c_str()); //gap
		}

		parseInfo.exportFormat = "jpg";
		int ipos = gcodeStr.find("Preview Img Type");
		if (ipos != std::string::npos)
		{
			parseInfo.exportFormat = gcodeStr.substr(ipos+17, 3);
		}


		parseInfo.layerHeight = 0.1;
		if (regex_match(gcodeStr, "Layer Height", sm))
		{
			std::string tStr = sm[1];
			parseInfo.layerHeight =atof(tStr.c_str()); //gap
            //兼容老的
            if (parseInfo.layerHeight >= 50)
                parseInfo.layerHeight = parseInfo.layerHeight / 1000.0f;
		}
		parseInfo.screenSize = "Sermoon D3";
		if (gcodeStr.find("Screen Size:CR-200B Pro") != std::string::npos)
		{
			parseInfo.screenSize = "CR - 200B Pro";
		}
		else if (gcodeStr.find("Screen Size:CR-10 Inspire") != std::string::npos)
		{
			parseInfo.screenSize = "CR-10 Inspire";
		}

		const QString& preStr = result->prefixCode().c_str();
		parseInfo.spiralMode = preStr.contains(";Vase Model:true");
		if (preStr.contains(";machine is belt:true"))
			parseInfo.beltType = 1;
		if (preStr.contains("Crealitybelt"))
			parseInfo.beltType = 2;
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