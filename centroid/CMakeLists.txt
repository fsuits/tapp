cmake_minimum_required(VERSION 3.1)

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories("${PROJECT_SOURCE_DIR}" ../ext/tapp_utils)

set(LOAD_MIDAS 0)

if(LOAD_MIDAS)
add_definitions(-DLOAD_MIDAS)
add_executable(centroid src/centroid.cpp ../ext/tapp_utils/external/Midas/Midas.cpp
../ext/tapp_utils/MassSpectrometry/Isotope/IsotopicClusterDetector.cpp)
else()
add_executable(centroid src/centroid.cpp)
endif()

target_link_libraries (centroid LINK_PUBLIC TAPPLib)

