cmake_minimum_required(VERSION 3.2)

set(LUA_MODULE_NAME "ndi_mod")

project(lua-extension CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

include(FindPkgConfig)
pkg_check_modules(LIBLUA REQUIRED lua-5.3)
include_directories(${LIBLUA_INCLUDE_DIRS})

#
# module
#
set(SOURCE_FILES
  ndi_mod.cpp
)

add_library(lua-extension MODULE ${SOURCE_FILES})
set_target_properties(lua-extension PROPERTIES
  PREFIX ""     # strip `lib` prefix
  OUTPUT_NAME "${LUA_MODULE_NAME}"
)

set(NORNS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/dep/norns")
target_include_directories(lua-extension PUBLIC "${NORNS_SOURCE_DIR}/matron/src")

target_include_directories(lua-extension PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")