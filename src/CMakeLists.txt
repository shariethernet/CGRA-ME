find_package(Gurobi)

add_subdirectory(archs)
add_subdirectory(core)
add_subdirectory(dotparser)
add_subdirectory(modules)
add_subdirectory(visual)

add_subdirectory(run)
add_subdirectory(adl_0)
add_subdirectory(adl_1)

add_library(cgra-me
    $<TARGET_OBJECTS:adl_0>
    $<TARGET_OBJECTS:adl_1>
    $<TARGET_OBJECTS:archs>
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:dotparser>
    $<TARGET_OBJECTS:modules>
    $<TARGET_OBJECTS:visual>
)

add_library(cgra-me_static STATIC
    $<TARGET_OBJECTS:adl_0>
    $<TARGET_OBJECTS:adl_1>
    $<TARGET_OBJECTS:archs>
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:dotparser>
    $<TARGET_OBJECTS:modules>
    $<TARGET_OBJECTS:visual>
)

target_include_directories(cgra-me
    PUBLIC ${CGRAME_INCLUDE_DIR}
)

target_include_directories(cgra-me_static
    PUBLIC ${CGRAME_INCLUDE_DIR}
)

if(GUROBI_LIBRARY_FOUND)
    target_link_libraries(cgra-me
        PRIVATE gurobi::cxx
        PRIVATE scip
        PRIVATE pugixml
    )
    target_link_libraries(cgra-me_static
        PRIVATE gurobi::cxx
        PRIVATE scip
        PRIVATE pugixml
    )
else()
    target_link_libraries(cgra-me
        PRIVATE scip
        PRIVATE pugixml
    )
    target_link_libraries(cgra-me_static
        PRIVATE scip
        PRIVATE pugixml
    )
endif()

