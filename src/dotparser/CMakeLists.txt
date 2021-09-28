add_library(
    dotparser
    OBJECT
    dfgdot_driver.cc
    dfgdot.tab.cc
    dfgdot.yy.cc
    dotparser.cc
)

target_include_directories(
    dotparser
    PRIVATE ${CGRAME_INCLUDE_DIR}
)

set_target_properties(
    dotparser PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
