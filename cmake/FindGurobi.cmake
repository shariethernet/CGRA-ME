#When GUROBI_HOME is defined
if(DEFINED ENV{GUROBI_HOME})
    message(STATUS "Found Environmental Variable $GUROBI_HOME")

    execute_process(
        COMMAND find -name "libgurobi.so*" -type f -printf "%f"
        WORKING_DIRECTORY $ENV{GUROBI_HOME}
        OUTPUT_VARIABLE GUROBI_SHARED_LIB_NAME
    )

    if(NOT GUROBI_SHARED_LIB_NAME)
        message(STATUS "Gurobi Not Found In $GUROBI_HOME, Using Default Solver Instead")
    else()
        find_path(
            GUROBI_INCLUDE_DIR
            NAMES gurobi_c++.h
            PATHS "$ENV{GUROBI_HOME}/linux64/include"
        )

        find_library(
            GUROBI_4_2_LIBRARY
            NAMES gurobi_g++4.2 libgurobi_g++4.2
            PATHS "$ENV{GUROBI_HOME}/linux64/lib"
        )

        find_library(
            GUROBI_5_2_LIBRARY
            NAMES gurobi_g++5.2 libgurobi_g++5.2
            PATHS "$ENV{GUROBI_HOME}/linux64/lib"
        )

        find_library(
            GUROBI_MAIN_LIBRARY
            NAMES ${GUROBI_SHARED_LIB_NAME}
            PATHS "$ENV{GUROBI_HOME}/linux64/lib"
        )
    endif()

#When GUROBI_HOME is not defined
else(DEFINED ENV{GUROBI_HOME})
    if(EXISTS "${CGRAME_DIR}/../gurobi751")
        message(STATUS "Found Gurobi v7.5.1")
        set(GUROBI_DIR_NAME "gurobi751")
        set(GUROBI_VERSION_NUM "7.5.1")
    elseif(EXISTS "${CGRAME_DIR}/../gurobi702")
        message(STATUS "Found Gurobi v7.0.2")
        set(GUROBI_DIR_NAME "gurobi702")
        set(GUROBI_VERSION_NUM "7.0.2")
    else()
        message(STATUS "Gurobi Not Found, Using Default Solver Instead")
    endif()

    find_path(
        GUROBI_INCLUDE_DIR
        NAMES gurobi_c++.h
        PATHS "${CGRAME_DIR}/../${GUROBI_DIR_NAME}/linux64/include"
    )

    find_library(
        GUROBI_4_2_LIBRARY
        NAMES gurobi_g++4.2 libgurobi_g++4.2
        PATHS "${CGRAME_DIR}/../${GUROBI_DIR_NAME}/linux64/lib"
    )

    find_library(
        GUROBI_5_2_LIBRARY
        NAMES gurobi_g++5.2 libgurobi_g++5.2
        PATHS "${CGRAME_DIR}/../${GUROBI_DIR_NAME}/linux64/lib"
    )

    find_library(
        GUROBI_MAIN_LIBRARY
        NAMES libgurobi.so.${GUROBI_VERSION_NUM}
        PATHS "${CGRAME_DIR}/../${GUROBI_DIR_NAME}/linux64/lib"
    )
endif()

    if(GUROBI_INCLUDE_DIR AND GUROBI_MAIN_LIBRARY)
        add_library(gurobi::main UNKNOWN IMPORTED)

        set_target_properties(
            gurobi::main PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GUROBI_INCLUDE_DIR}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${GUROBI_MAIN_LIBRARY}"
        )

        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_library(gurobi::cxx UNKNOWN IMPORTED)

            if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.2")
                set_target_properties(
                gurobi::cxx PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                IMPORTED_LOCATION "${GUROBI_4_2_LIBRARY}"
                )
            else()
                set_target_properties(
                gurobi::cxx PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                IMPORTED_LOCATION "${GUROBI_5_2_LIBRARY}"
                )
            endif()

            set_target_properties(
                gurobi::cxx PROPERTIES
                INTERFACE_LINK_LIBRARIES gurobi::main
            )
        endif()
    endif()

include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
        GUROBI_LIBRARY
        REQUIRED_VARS GUROBI_MAIN_LIBRARY GUROBI_INCLUDE_DIR
    )

