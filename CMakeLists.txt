cmake_minimum_required(VERSION 2.6)
project(cdim)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
add_executable(cdim cdim.cpp)

set(CDIM_VERSION_MAJOR 0)
set(CDIM_VERSION_MINOR 1)
set(CDIM_VERSION_PATCH 1)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)

# link libcdim
find_package(CDIM REQUIRED)
include_directories(${CDIM_INCLUDE_DIR})
target_link_libraries (cdim ${CDIM_LIBRARIES})

install(TARGETS cdim RUNTIME DESTINATION bin)
