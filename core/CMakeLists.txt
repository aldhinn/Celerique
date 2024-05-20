# File: ./core/CMakeLists.txt
# Author: Aldhinn Espinas
# Description: This cmake list file configures the core module.

# License: Mozilla Public License 2.0. (See ./LICENSE).

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/projects.cmake)

cmake_minimum_required(VERSION ${CELERIQUE_MINIMUM_CMAKE_VERSION})
project(CeleriqueEngineCore VERSION ${CELERIQUE_PROJECT_VERSION})

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/language.cmake)
include(FetchContent)

if (NOT TARGET CeleriqueEngineCore)
    file(GLOB_RECURSE CeleriqueEngineCoreSrc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )

    if (BuildCeleriqueEngineCoreUnitTestingC OR PROJECT_IS_TOP_LEVEL)
        enable_testing()
        find_package(CMocka QUIET)

        if (NOT CMocka_FOUND)
            set(WITH_STATIC_LIB ON)
            set(UNIT_TESTING OFF)
            set(WITH_EXAMPLES OFF)
            FetchContent_Declare(
                cmocka
                GIT_REPOSITORY      https://github.com/clibs/cmocka.git
                GIT_TAG             cmocka-1.1.5
            )
            FetchContent_MakeAvailable(cmocka)
        endif()

        add_executable(
            TypeUnitTestC
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/types.cmocka.c
        )
        if (NOT CMocka_FOUND)
            target_link_libraries(
                TypeUnitTestC PUBLIC
                ${CMOCKA_STATIC_LIBRARY}
            )
        else()
            target_link_libraries(
                TypeUnitTestC PUBLIC
                cmocka
            )
        endif()
        target_include_directories(
            TypeUnitTestC PRIVATE
            ${cmocka-header_SOURCE_DIR}
        )
        add_test(
            NAME TypeUnitTestC
            COMMAND TypeUnitTestC
        )
    endif()
endif()