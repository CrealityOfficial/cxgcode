__cc_find(RapidJson)
__cc_find(thumbnail)

__assert_target(thumbnail)

set(SRCS
  cxgcode/define.h
  cxgcode/interface.h
  cxgcode/sliceline.h
  cxgcode/gcodeloadjob.h
  cxgcode/gcodeloadjob.cpp
  cxgcode/sliceresult.h
  cxgcode/sliceresult.cpp
  cxgcode/gcodehelper.h
  cxgcode/gcodehelper.cpp
  cxgcode/sliceattain.h
  cxgcode/sliceattain.cpp
  cxgcode/simplegcodebuilder.h
  cxgcode/simplegcodebuilder.cpp
  cxgcode/gcodebuilder.h
  cxgcode/gcodebuilder.cpp
  cxgcode/slicemodelbuilder.h
  cxgcode/slicemodelbuilder.cpp
  
  cxgcode/us/defaultloader.h
  cxgcode/us/defaultloader.cpp
  cxgcode/us/defenumsmodel.h
  cxgcode/us/defenumsmodel.cpp
  cxgcode/us/jsondef.h
  cxgcode/us/jsondef.cpp
  cxgcode/us/metatype.h
  cxgcode/us/metatype.cpp
  cxgcode/us/settingdef.h
  cxgcode/us/settingdef.cpp
  cxgcode/us/settinggroupdef.h
  cxgcode/us/settinggroupdef.cpp
  cxgcode/us/settingitemdef.h
  cxgcode/us/settingitemdef.cpp
  cxgcode/us/usetting.h
  cxgcode/us/usetting.cpp
  cxgcode/us/usettings.h
  cxgcode/us/usettings.cpp
  
  cxgcode/render/gcodevieweffect.h
  cxgcode/render/gcodevieweffect.cpp
  cxgcode/render/gcodeviewentity.h
  cxgcode/render/gcodeviewentity.cpp
)

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
	qcxutil
	
	trimesh2
	rapidjson
)

__add_common_library(cxgcode)
