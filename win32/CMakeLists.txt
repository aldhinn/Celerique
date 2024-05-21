# File: ./win32/CMakeLists.txt
# Author: Aldhinn Espinas
# Description: This cmake list file configures the win32 plugin module.

# License: Mozilla Public License 2.0. (See ./LICENSE).

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/projects.cmake)

cmake_minimum_required(VERSION ${CELERIQUE_MINIMUM_CMAKE_VERSION})
project(CeleriqueEngineWin32Plugin VERSION ${CELERIQUE_PROJECT_VERSION})

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/language.cmake)

if (NOT TARGET CeleriqueEngineWin32Plugin)
    # Add core as a subdirectory.
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/
        ${CMAKE_CURRENT_BINARY_DIR}/core/
    )

    # Collect source.
    file(GLOB_RECURSE CeleriqueEngineWin32PluginSrc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )

    # Win32 plugin library.
    add_library(
        CeleriqueEngineWin32Plugin STATIC
        ${CeleriqueEngineWin32PluginSrc}
    )
    target_link_libraries(
        CeleriqueEngineWin32Plugin PRIVATE
        CeleriqueEngineCore user32
    )
    target_include_directories(
        CeleriqueEngineWin32Plugin PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    option(
        BuildCeleriqueEngineWin32PluginTesting
        "The Switch that enables building test targets for the win32 plugin"
        OFF
    )
    if (BuildCeleriqueEngineWin32PluginTesting OR PROJECT_IS_TOP_LEVEL)
        add_executable(
            CeleriqueEngineWin32PluginTesting
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/window.cpp
        )
        target_link_libraries(
            CeleriqueEngineWin32PluginTesting PRIVATE
            CeleriqueEngineCore CeleriqueEngineWin32Plugin
        )
    endif()
endif()