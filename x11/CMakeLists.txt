# File: ./x11/CMakeLists.txt
# Author: Aldhinn Espinas
# Description: This cmake list file configures the x11 plugin module.

# License: Mozilla Public License 2.0. (See ./LICENSE).

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/projects.cmake)

cmake_minimum_required(VERSION ${CELERIQUE_MINIMUM_CMAKE_VERSION})
project(CeleriqueEngineX11Plugin VERSION ${CELERIQUE_PROJECT_VERSION})

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/language.cmake)

if (NOT TARGET CeleriqueEngineX11Plugin)
    find_package(X11 REQUIRED)

    # Add core as a subdirectory.
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/
        ${CMAKE_CURRENT_BINARY_DIR}/core/
    )

    # Collect source.
    file(GLOB_RECURSE CeleriqueEngineX11PluginSrc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )

    # x11 plugin library.
    add_library(
        CeleriqueEngineX11Plugin STATIC
        ${CeleriqueEngineX11PluginSrc}
    )
    target_link_libraries(
        CeleriqueEngineX11Plugin PRIVATE
        CeleriqueEngineCore ${X11_LIBRARIES}
    )
    target_include_directories(
        CeleriqueEngineX11Plugin PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include ${X11_INCLUDE_DIR}
    )

    option(
        BuildCeleriqueEngineX11PluginTesting
        "The Switch that enables building test targets for the x11 plugin"
        OFF
    )
    if (BuildCeleriqueEngineX11PluginTesting OR PROJECT_IS_TOP_LEVEL)
        add_executable(
            CeleriqueEngineX11PluginTesting
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/window.cpp
        )
        target_link_libraries(
            CeleriqueEngineX11PluginTesting PRIVATE
            CeleriqueEngineCore CeleriqueEngineX11Plugin
        )
    endif()
endif()