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

    # Core library.
    add_library(
        CeleriqueEngineCore STATIC
        ${CeleriqueEngineCoreSrc}
    )
    target_include_directories(
        CeleriqueEngineCore PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    option(
        BuildCeleriqueEngineCoreUnitTestingCpp
        "The Switch that enables building the C++ unit testing for the core module."
        OFF
    )
    if (BuildCeleriqueEngineCoreUnitTestingCpp OR PROJECT_IS_TOP_LEVEL)
        enable_testing()
        find_package(GTest QUIET)

        if (NOT GTest_FOUND)
            FetchContent_Declare(
                googletest
                GIT_REPOSITORY      https://github.com/google/googletest.git
                GIT_TAG             v1.14.0
            )
            FetchContent_MakeAvailable(googletest)
        endif()

        # Core Unit Testing CMake Target configurations.
        file(GLOB_RECURSE CeleriqueEngineCoreUnitTestingCppSrc
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.gtest.cpp
        )
        add_executable(
            CeleriqueEngineCoreUnitTestingCpp
            ${CeleriqueEngineCoreUnitTestingCppSrc}
        )
        target_link_libraries(
            CeleriqueEngineCoreUnitTestingCpp PUBLIC
            GTest::gtest_main GTest::gmock_main CeleriqueEngineCore
        )
        target_include_directories(
            CeleriqueEngineCoreUnitTestingCpp PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        gtest_discover_tests(CeleriqueEngineCoreUnitTestingCpp)
    endif()

    option(
        BuildCeleriqueEngineCoreUnitTestingC
        "The Switch that enables building the C unit testing for the core module."
        OFF
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
            LoggingUnitTestC
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/logging.cmocka.c
        )
        if (NOT CMocka_FOUND)
            target_link_libraries(
                LoggingUnitTestC PUBLIC
                CeleriqueEngineCore ${CMOCKA_STATIC_LIBRARY}
            )
        else()
            target_link_libraries(
                LoggingUnitTestC PUBLIC
                CeleriqueEngineCore cmocka
            )
        endif()
        target_include_directories(
            LoggingUnitTestC PRIVATE
            ${cmocka-header_SOURCE_DIR}
        )
        add_test(
            NAME LoggingUnitTestC
            COMMAND LoggingUnitTestC
        )

        add_executable(
            TypeUnitTestC
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/types.cmocka.c
        )
        if (NOT CMocka_FOUND)
            target_link_libraries(
                TypeUnitTestC PUBLIC
                CeleriqueEngineCore ${CMOCKA_STATIC_LIBRARY}
            )
        else()
            target_link_libraries(
                TypeUnitTestC PUBLIC
                CeleriqueEngineCore cmocka
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