__cc_find(RapidJson)
__files_group(${CMAKE_CURRENT_SOURCE_DIR}/cxgcode/ SRCS)

set(LIBS
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  Qt5::Quick

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
