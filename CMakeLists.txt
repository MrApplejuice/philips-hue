cmake_minimum_required(VERSION 2.6)

project(philips-hue)

find_package(UseGSSDP REQUIRED)

include_directories(include/)
set(LIB_SOURCES src/discovery/Discoverer.cpp)
add_library(philipshue STATIC ${LIB_SOURCES})
