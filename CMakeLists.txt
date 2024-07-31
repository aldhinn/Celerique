# File: ./CMakeLists.txt
# Author: Aldhinn Espinas
# Description: This cmake list file configures engine library.

# License: Mozilla Public License 2.0. (See ./LICENSE).

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/projects.cmake)

cmake_minimum_required(VERSION ${CELERIQUE_MINIMUM_CMAKE_VERSION})
project(CeleriqueEngine VERSION ${CELERIQUE_PROJECT_VERSION})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/language.cmake)

if (NOT TARGET celerique)
    # Collecting the source files for the engine library.
    file(GLOB_RECURSE CeleriqueSrc ${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.cpp)

    # GUI window wrappers.
    if (WIN32)
        file(GLOB_RECURSE CeleriqueSrc
            ${CeleriqueSrc} ${CMAKE_CURRENT_SOURCE_DIR}/win32/src/*.cpp
        )
    elseif(UNIX AND NOT ANDROID AND NOT APPLE)
        file(GLOB_RECURSE CeleriqueSrc
            ${CeleriqueSrc} ${CMAKE_CURRENT_SOURCE_DIR}/x11/src/*.cpp
        )
    endif()

    option(
        CeleriqueWrappingVulkan
        "The Switch that enables building wrappers for the vulkan library."
        ON
    )
    # Not wrapping vulkan for web assembly.
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten" AND CeleriqueWrappingVulkan)
        file(GLOB_RECURSE CeleriqueSrc
            ${CeleriqueSrc} ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/src/*.cpp
        )
    endif()

    # Static library.
    add_library(celerique STATIC ${CeleriqueSrc})
    target_include_directories(celerique PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
    if (WIN32)
        target_include_directories(celerique PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/win32/include)
        target_link_libraries(celerique PRIVATE user32)
    elseif(UNIX AND NOT ANDROID AND NOT APPLE)
        find_package(X11 REQUIRED)
        find_library(WAYLAND_CLIENT_LIBRARIES NAMES wayland-client)

        target_include_directories(celerique PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/x11/include ${X11_INCLUDE_DIR})
        target_link_libraries(celerique PRIVATE ${X11_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES})
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten" AND CeleriqueWrappingVulkan)
        find_package(Vulkan REQUIRED)

        target_include_directories(celerique PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include ${Vulkan_INCLUDE_DIR})
        target_link_libraries(celerique PRIVATE ${Vulkan_LIBRARIES})
    endif()

    # Shared library.
    add_library(celerique-shared SHARED ${CeleriqueSrc})
    target_compile_definitions(
        celerique-shared
        PUBLIC CELERIQUE_ENGINE_LINKED_SHARED
        PRIVATE CELERIQUE_ENGINE_BUILDING_SHARED
    )
    target_include_directories(celerique-shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
    if (WIN32)
        target_include_directories(celerique-shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/win32/include)
        target_link_libraries(celerique-shared PRIVATE user32)
    elseif(UNIX AND NOT ANDROID AND NOT APPLE)
        target_include_directories(celerique-shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/x11/include ${X11_INCLUDE_DIR})
        target_link_libraries(celerique-shared PRIVATE ${X11_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES})
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten" AND CeleriqueWrappingVulkan)
        target_include_directories(celerique-shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include ${Vulkan_INCLUDE_DIR})
        target_link_libraries(celerique-shared PRIVATE ${Vulkan_LIBRARIES})
    endif()
endif()