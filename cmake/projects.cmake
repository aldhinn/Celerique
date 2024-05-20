# File: ./cmake/projects.cmake
# Author: Aldhinn Espinas
# Description: This provides standard configurations for all the
#   cmake projects in this repository.

# License: Mozilla Public License 2.0. (See ./LICENSE).

if (NOT DEFINED CELERIQUE_PROJECT_VERSION)
    # The minimum version of cmake we will be working on for all projects.
    set(CELERIQUE_MINIMUM_CMAKE_VERSION 3.25)
    # The version that is currently being developed.
    set(CELERIQUE_PROJECT_VERSION 0.0.1)

    # All projects will see the header files in this directory.
    include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/../include
    )

    # Extract the value of the repo's directory by getting
    # the parent of this list's directory.
    cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH RepoDir)
    # Add the value of the root repo directory as a define.
    # To be used to assess where the source is relative to the repository's root.
    add_compile_definitions(
        CELERIQUE_REPO_ROOT_DIR="${RepoDir}"
    )

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_definitions(CELERIQUE_DEBUG_MODE)
    endif()

    # If targeting a posix compliant system.
    if (UNIX)
        add_compile_definitions(CELERIQUE_FOR_POSIX_SYSTEMS)
    endif()
    # If targeting windows.
    if (WIN32)
        add_compile_definitions(CELERIQUE_FOR_WINDOWS)
    endif()
    # If targeting android.
    if (ANDROID)
        add_compile_definitions(CELERIQUE_FOR_ANDROID)
    endif()
    # If targeting macos.
    if (APPLE AND NOT IOS)
        add_compile_definitions(CELERIQUE_FOR_MACOS)
    endif()
    # If targeting ios.
    if (APPLE AND IOS)
        add_compile_definitions(CELERIQUE_FOR_IOS)
    endif()
endif()