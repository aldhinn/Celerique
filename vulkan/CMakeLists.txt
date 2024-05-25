# File: ./vulkan/CMakeLists.txt
# Author: Aldhinn Espinas
# Description: This cmake list file configures the vulkan plugin module
#   for the Celerique Engine.

# License: Mozilla Public License 2.0. (See ./LICENSE).

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/projects.cmake)

cmake_minimum_required(VERSION ${CELERIQUE_MINIMUM_CMAKE_VERSION})
project(CeleriqueEngineVulkanPlugin VERSION ${CELERIQUE_PROJECT_VERSION})

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/language.cmake)
include(FetchContent)

if (NOT TARGET CeleriqueEngineVulkanPlugin)
    # Dependencies.
    find_package(Vulkan REQUIRED)
    if (UNIX AND NOT ANDROID AND NOT APPLE)
        find_package(X11 REQUIRED)
        find_library(WAYLAND_CLIENT_LIBRARIES NAMES wayland-client)
    endif()

    # Add core as a subdirectory.
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/
        ${CMAKE_CURRENT_BINARY_DIR}/core/
    )

    # Collect source.
    file(GLOB_RECURSE CeleriqueEngineVulkanPluginSrc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )

    # Vulkan plugin library.
    add_library(
        CeleriqueEngineVulkanPlugin STATIC
        ${CeleriqueEngineVulkanPluginSrc}
    )
    target_link_libraries(
        CeleriqueEngineVulkanPlugin PRIVATE
        CeleriqueEngineCore ${Vulkan_LIBRARIES}
    )
    if (UNIX AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(
            CeleriqueEngineVulkanPlugin PRIVATE
            ${X11_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES}
        )
    endif()
    target_include_directories(
        CeleriqueEngineVulkanPlugin PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIR}
    )
    if (UNIX AND NOT ANDROID AND NOT APPLE)
        target_include_directories(
            CeleriqueEngineVulkanPlugin PRIVATE
            ${X11_INCLUDE_DIR}
        )
    endif()

    option(
        BuildCeleriqueEngineVulkanPluginUnitTestingCpp
        "The Switch that enables building the C++ unit testing for the vulkan plugin module."
        OFF
    )
    if (BuildCeleriqueEngineVulkanPluginUnitTestingCpp OR PROJECT_IS_TOP_LEVEL)
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

        if (WIN32)
            add_subdirectory(
                ${CMAKE_CURRENT_SOURCE_DIR}/../win32
                ${CMAKE_CURRENT_BINARY_DIR}/win32
            )
        else()
            add_subdirectory(
                ${CMAKE_CURRENT_SOURCE_DIR}/../x11
                ${CMAKE_CURRENT_BINARY_DIR}/x11
            )
        endif()

        # Vulkan Plugin Unit Testing CMake Target configurations.
        file(GLOB_RECURSE CeleriqueEngineVulkanPluginUnitTestingCppSrc
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.gtest.cpp
        )
        add_executable(
            CeleriqueEngineVulkanPluginUnitTestingCpp
            ${CeleriqueEngineVulkanPluginUnitTestingCppSrc}
        )
        target_link_libraries(
            CeleriqueEngineVulkanPluginUnitTestingCpp PUBLIC
            GTest::gtest_main GTest::gmock_main CeleriqueEngineCore
            CeleriqueEngineVulkanPlugin
        )
        if (WIN32)
            target_link_libraries(
                CeleriqueEngineVulkanPluginUnitTestingCpp PUBLIC
                CeleriqueEngineWin32Plugin
            )
        else()
            target_link_libraries(
                CeleriqueEngineVulkanPluginUnitTestingCpp PUBLIC
                CeleriqueEngineX11Plugin
            )
        endif()
        target_include_directories(
            CeleriqueEngineVulkanPluginUnitTestingCpp PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIR}
        )
        gtest_discover_tests(CeleriqueEngineVulkanPluginUnitTestingCpp)
    endif()
endif()