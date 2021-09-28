SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fno-rtti" )

add_library(DFG MODULE DFGGeneration.cpp)

target_link_libraries(
  DFG
  PRIVATE cgra-me
  )

configure_file(LoopParser.py ${CMAKE_BINARY_DIR}/script/LoopParser.py COPYONLY)

