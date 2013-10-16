# First include glib...
find_package(PkgConfig REQUIRED)

# Detect GLIB
pkg_check_modules(GLIB_PKG glib-2.0)
if (!GLIB_PKG_FOUND)
  message(FATAL_ERROR "glib-2.0 not found")
endif()

# Detect GObject
pkg_check_modules(GOBJECT_PKG gobject-2.0)
if (!GOBJECT_PKG_FOUND)
  message(FATAL_ERROR "gobject-2.0 not found")
endif()

include_directories(${GLIB_PKG_INCLUDE_DIRS})


# Then look for gssdp and include it
find_path(GSSDP_INCLUDE_PATH gssdp-1.0 /usr/include/ /usr/local/include/)
find_library(GSSDP_LIBRARIES gssdp-1.0)

if (GSSDP_INCLUDE_PATH-NOTFOUND)
  message(FATAL_ERROR "gssdp include directory not found")
endif()

include_directories("${GSSDP_INCLUDE_PATH}/gssdp-1.0")

message(INFO "glib libraries! ${GLIB_PKG_LIBRARIES}")

set(GSSDP_LIBRARIES ${GSSDP_LIBRARIES} ${GOBJECT_PKG_LIBRARIES} ${GLIB_PKG_LIBRARIES})
