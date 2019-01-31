# Copyright 2019, IBM Corporation
# 
# This source code is licensed under the Apache License, Version 2.0 found in
# the LICENSE.md file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.1)
project (TAPP)

# Set project properties
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory (ext)
add_subdirectory (centroid)
add_subdirectory (grid)
add_subdirectory (meta_match)
add_subdirectory (warp)

