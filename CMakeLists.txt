#
#   libcdim - a library for manipulation CBM imagefiles (mainly d64)
# 
#   Copyright (C) [2015]  [Thomas Martens]
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 3 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
#   for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, see <http://www.gnu.org/licenses/>.
#

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
