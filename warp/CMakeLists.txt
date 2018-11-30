cmake_minimum_required(VERSION 3.1)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${PROJECT_SOURCE_DIR}" ../ext/tapp_utils)

add_executable(warp2d src/Warp2D.cpp src/Warp2DCommand.cpp src/PeakMatchDB.cpp src/PeakWarpDB.cpp)

target_link_libraries (warp2d LINK_PUBLIC TAPPLib)

