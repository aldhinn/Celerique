# File: ./CMakeLists.txt
# Author: Aldhinn Espinas
# Description: This cmake list file configures engine library.

# License: Mozilla Public License 2.0. (See ./LICENSE).

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/projects.cmake)

cmake_minimum_required(VERSION ${CELERIQUE_MINIMUM_CMAKE_VERSION})
project(CeleriqueEngine VERSION ${CELERIQUE_PROJECT_VERSION})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/language.cmake)

if (NOT TARGET celerique)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_BINARY_DIR}/core
    )

    # GUI window wrappers.
    if (WIN32)
        add_subdirectory(
            ${CMAKE_CURRENT_SOURCE_DIR}/win32
            ${CMAKE_CURRENT_BINARY_DIR}/win32
        )
    elseif(UNIX AND NOT ANDROID AND NOT APPLE)
        add_subdirectory(
            ${CMAKE_CURRENT_SOURCE_DIR}/x11
            ${CMAKE_CURRENT_BINARY_DIR}/x11
        )
    endif()

    option(
        CeleriqueWrappingVulkan
        "The Switch that enables building wrappers for the vulkan library."
        ON
    )
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten" AND CeleriqueWrappingVulkan)
        add_subdirectory(
            ${CMAKE_CURRENT_SOURCE_DIR}/vulkan
            ${CMAKE_CURRENT_BINARY_DIR}/vulkan
        )
    endif()
endif()