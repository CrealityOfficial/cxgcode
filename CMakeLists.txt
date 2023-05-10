set(SRCS
  cxgcode/define.h
  cxgcode/interface.h
)

set(LIBS
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  Qt5::Quick
)

set(ILIBS
)

__add_common_library(cxgcode)
