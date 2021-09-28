include(ProcessorCount)
ProcessorCount(CORE_NUMBER)
if(NOT CORE_NUMBER EQUAL 0)
    SET(SCIP_MAKE_FLAG -j${CORE_NUMBER})
endif()

add_custom_target(scipoptsuite
    COMMAND make scipoptlib SHARED=true ZLIB=false GMP=false READLINE=false ${SCIP_MAKE_FLAG}
    COMMAND ln -nfL lib/libscipopt.so ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libscipopt.so
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(scip INTERFACE)

target_link_libraries(
    scip
    INTERFACE -L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l:libscipopt.so
)

