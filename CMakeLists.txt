__cc_find(RapidJson)
__cc_find(thumbnail)

__assert_target(thumbnail)

__files_group(${CMAKE_CURRENT_SOURCE_DIR}/cxgcode/ SRCS)
#set(SRCS
#  cxgcode/interface.h
#  cxgcode/gcodeloadjob.h
#  cxgcode/gcodeloadjob.cpp
#  cxgcode/gcodehelper.h
#  cxgcode/gcodehelper.cpp
#  cxgcode/sliceattain.h
#  cxgcode/sliceattain.cpp
#  cxgcode/simplegcodebuilder.h
#  cxgcode/simplegcodebuilder.cpp
#  cxgcode/gcodebuilder.h
#  cxgcode/gcodebuilder.cpp
#
#  cxgcode/model/gcodepreviewlistmodel.h
#  cxgcode/model/gcodepreviewlistmodel.cpp
#  cxgcode/model/gcodespeedlistmodel.h
#  cxgcode/model/gcodespeedlistmodel.cpp
#  cxgcode/model/gcodestructurelistmodel.h
#  cxgcode/model/gcodestructurelistmodel.cpp
#  cxgcode/model/gcodeextruderlistmodel.h
#  cxgcode/model/gcodeextruderlistmodel.cpp
#  cxgcode/model/gcodelayerhightlistmodel.h
#  cxgcode/model/gcodelayerhightlistmodel.cpp
#  cxgcode/model/gcodelinewidthlistmodel.h
#  cxgcode/model/gcodelinewidthlistmodel.cpp
#  cxgcode/model/gcodeflowlistmodel.h
#  cxgcode/model/gcodeflowlistmodel.cpp
#  cxgcode/model/gcodelayertimelistmodel.h
#  cxgcode/model/gcodelayertimelistmodel.cpp
#  cxgcode/model/gcodefanspeedlistmodel.h
#  cxgcode/model/gcodefanspeedlistmodel.cpp
#  cxgcode/model/gcodetemperaturelistmodel.h
#  cxgcode/model/gcodetemperaturelistmodel.cpp
#
#  cxgcode/us/defaultloader.h
#  cxgcode/us/defaultloader.cpp
#  cxgcode/us/defenumsmodel.h
#  cxgcode/us/defenumsmodel.cpp
#  cxgcode/us/jsondef.h
#  cxgcode/us/jsondef.cpp
#  cxgcode/us/metatype.h
#  cxgcode/us/metatype.cpp
#  cxgcode/us/settingdef.h
#  cxgcode/us/settingdef.cpp
#  cxgcode/us/settinggroupdef.h
#  cxgcode/us/settinggroupdef.cpp
#  cxgcode/us/settingitemdef.h
#  cxgcode/us/settingitemdef.cpp
#  cxgcode/us/usetting.h
#  cxgcode/us/usetting.cpp
#  cxgcode/us/usettings.h
#  cxgcode/us/usettings.cpp
#
#  cxgcode/render/gcodevieweffect.h
#  cxgcode/render/gcodevieweffect.cpp
#  cxgcode/render/gcodeviewentity.h
#  cxgcode/render/gcodeviewentity.cpp
#)

set(LIBS
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  Qt5::Quick

  thumbnail
  mmesh
)

set(ILIBS
	qtuser_core
	qtuser_3d
	gcode
	trimesh2
	rapidjson
	crslice
)

__add_common_library(cxgcode)
