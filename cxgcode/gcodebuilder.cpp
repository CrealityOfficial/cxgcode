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