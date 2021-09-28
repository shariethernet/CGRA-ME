add_library(adl_0 OBJECT adl.cpp  adl_parser.cpp  grid.cpp  macro.cpp  module.cpp)

target_include_directories(adl_0
    PRIVATE ${CGRAME_INCLUDE_DIR}
    PRIAVTE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${PUGIXML_INCLUDE_DIR}
)

set_target_properties(
    adl_0 PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

