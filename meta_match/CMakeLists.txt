cmake_minimum_required(VERSION 3.1)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${PROJECT_SOURCE_DIR}" ../ext/tapp_utils)

add_executable(metamatch src/MetaMatch.cpp src/MetaMatchCommand.cpp)

target_link_libraries (metamatch LINK_PUBLIC TAPPLib)

