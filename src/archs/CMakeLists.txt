file(GLOB ARCHS_SRC
    "*.cpp"
)

add_library(archs OBJECT ${ARCHS_SRC})

target_include_directories(archs
    PRIVATE ${CGRAME_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(
    archs PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

