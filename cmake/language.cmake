# File: ./cmake/language.cmake
# Author: Aldhinn Espinas
# Description: This sets the language standard for C and C++.

# License: Mozilla Public License 2.0. (See ./LICENSE).

# Set to C++ 17.
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()