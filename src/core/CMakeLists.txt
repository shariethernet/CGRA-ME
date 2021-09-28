add_library(
  core
  OBJECT
  AnnealMapper.cpp
  BitSetting.cpp
  BitStream.cpp
  CGRA.cpp
  ILPMapper.cpp
  Mapper.cpp
  Mapping.cpp
  OpGraph.cpp
  MRRG.cpp
  Module.cpp
  ModuleRoutingStructures.cpp
  ModuleComposites.cpp
  ModuleFuncUnit.cpp
)

add_dependencies(
    core
    scipoptsuite
)

set_target_properties(
    core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

if(GUROBI_LIBRARY_FOUND)
    target_compile_definitions(
        core
        PRIVATE USE_GUROBI
        )
    target_include_directories(
        core
        PRIVATE ${CGRAME_INCLUDE_DIR}
        PRIVATE ${SCIP_INCLUDE_DIR}
        PRIVATE ${GUROBI_INCLUDE_DIR}
    )
else()
    target_include_directories(
        core
        PRIVATE ${CGRAME_INCLUDE_DIR}
        PRIVATE ${SCIP_INCLUDE_DIR}
    )
endif()

