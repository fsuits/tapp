# Copyright 2019, IBM Corporation
# 
# This source code is licensed under the Apache License, Version 2.0 found in
# the LICENSE.md file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.1)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${PROJECT_SOURCE_DIR}" ../ext/tapp_utils)

add_executable(warp2d src/Warp2D.cpp src/Warp2DCommand.cpp src/PeakMatchDB.cpp src/PeakWarpDB.cpp)

target_link_libraries (warp2d LINK_PUBLIC TAPPLib)

