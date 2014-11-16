cmake_minimum_required(VERSION 2.6)
project(cdim)

add_executable(cdim main.cpp)

install(TARGETS cdim RUNTIME DESTINATION bin)
