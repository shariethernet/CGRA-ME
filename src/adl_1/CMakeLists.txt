add_library(adl_1 OBJECT ADLParser.cpp ADLStructs.cpp SyntacticSugar.cpp)

target_include_directories(adl_1
    PRIVATE ${CGRAME_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${PUGIXML_INCLUDE_DIR}
)

set_target_properties(
    adl_1 PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

configure_file(module_templates.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/module_templates.xml COPYONLY)

