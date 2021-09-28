add_library(
    visual
    OBJECT
    Visual.cpp
)

set_target_properties(
    visual PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(
    visual
    PRIVATE ${CGRAME_INCLUDE_DIR}
)

configure_file(CGRA.html ${CGRAME_DIR}/output/CGRA.html COPYONLY)

