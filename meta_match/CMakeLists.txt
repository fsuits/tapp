# Copyright 2019, IBM Corporation
# 
# This source code is licensed under the Apache License, Version 2.0 found in
# the LICENSE.md file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.1)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${PROJECT_SOURCE_DIR}" ../ext/tapp_utils)

add_executable(metamatch src/MetaMatch.cpp src/MetaMatchCommand.cpp)

target_link_libraries (metamatch LINK_PUBLIC TAPPLib)

