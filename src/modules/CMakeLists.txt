file(GLOB MODULES_SRC
    "*.cpp"
)

add_library(modules OBJECT ${MODULES_SRC})

target_include_directories(modules
    PRIVATE ${CGRAME_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(
    modules PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

