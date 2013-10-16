cmake_minimum_required(VERSION 2.6)

project(philips-hue)

# General stuff
find_package(UseGSSDP REQUIRED)

include_directories(include/)

# Library libphilipshue.a
set(LIB_SOURCES src/discovery/Discoverer.cpp)
add_library(philipshue STATIC ${LIB_SOURCES})

# Executable ctlhue
add_executable(ctlhue src/ctlhue/main.cpp)
target_link_libraries(ctlhue philipshue ${GSSDP_LIBRARIES})

