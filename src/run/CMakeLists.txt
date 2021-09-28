add_executable(cgrame cgrame.cpp)
target_link_libraries(cgrame cgra-me cxxopts mINI)
configure_file(mapper_config.ini ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mapper_config.ini COPYONLY)

