cmake_minimum_required(VERSION 2.6)

project(philips-hue)

# Library detection

# Detect GSSDP
find_package(UseGSSDP REQUIRED)

# Detect libcurl
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCURL_PKG libcurl)
if (!LIBCURL_PKG_FOUND)
  message(FATAL_ERROR "libcurl not found")
endif()


include_directories(include/)

# Library libphilipshue.a
set(LIB_SOURCES src/discovery/Discoverer.cpp)
add_library(philipshue STATIC ${LIB_SOURCES})

# Executable ctlhue
add_executable(ctlhue src/ctlhue/main.cpp)
target_link_libraries(ctlhue philipshue ${GSSDP_LIBRARIES} ${LIBCURL_PKG_LIBRARIES})

