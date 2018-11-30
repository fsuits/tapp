cmake_minimum_required(VERSION 3.1)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${PROJECT_SOURCE_DIR}" ../ext/tapp_utils)

add_executable(grid src/Grid.cpp)

target_link_libraries (grid LINK_PUBLIC TAPPLib)

