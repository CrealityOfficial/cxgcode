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
        std::string temp1 = ".*" + key + ":([0-9]{0,8}).*";
        std::string temp2 = ".*" + key + ":([-]{0,1}[0-9]{0,8}\\.[0-9]{0,8}).*";
        if (std::regex_match(gcodeStr, sm, std::regex(temp1.c_str()))||
            std::regex_match(gcodeStr, sm, std::regex(temp2.c_str())))
        {
            return true;
        }
        return false;
    }

    void getImage(std::string p, SliceResult* result)
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
            QImage image;
            image.loadFromData(&decodeData[0], decodeData.size());
            result->previews.push_back(image);
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
		std::string gcodeStr = result->prefixCode().toStdString();

		std::replace(gcodeStr.begin(), gcodeStr.end(), '\n', ' ');
		std::replace(gcodeStr.begin(), gcodeStr.end(), '\r', ' ');

		std::smatch sm;

        result->previews.clear();
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

		int index1 = result->prefixCode().lastIndexOf("M83");
		int index2 = result->prefixCode().lastIndexOf("M82");
		if (index1 > index2)
			parseInfo.relativeExtrude = true;

		SettingsPointer ExtrudeSettings = result->ES.empty() ? result->G : result->ES.back();
		float material_diameter = ExtrudeSettings->value("material_diameter", "1.75").toFloat();
        //单位面积密度
		parseInfo.materialDensity = PI * (material_diameter * 0.5) * (material_diameter * 0.5) * ExtrudeSettings->value("material_density", "1.24").toFloat();

        float filament_cost = ExtrudeSettings->value("filament_cost", "0.0").toFloat();
        float filament_weight = ExtrudeSettings->value("filament_weight", "0.0").toFloat();

        float filament_length = filament_weight /  parseInfo.materialDensity;
        parseInfo.unitPrice = filament_cost / filament_length;

		parseInfo.lineWidth = result->G->value("line_width", "0.1").toFloat();
		parseInfo.exportFormat = result->G->value("preview_img_type", "jpg");
		parseInfo.layerHeight = result->G->value("layer_height", "0.1").toFloat();
		parseInfo.screenSize = result->G->value("screen_size", "Sermoon D3");

		const QString& preStr = result->prefixCode();
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

	void GCodeBuilder::updateFlagAttribute(Qt3DRender::QAttribute* attribute, GCodeVisualType type)
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