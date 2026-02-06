cmake_minimum_required(VERSION 3.2)

set(LUA_MODULE_NAME "ndi_mod")

project(lua-extension CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

include(FindPkgConfig)
pkg_check_modules(LIBLUA REQUIRED lua-5.3)
pkg_check_modules(JACK REQUIRED jack)
include_directories(${LIBLUA_INCLUDE_DIRS} ${JACK_INCLUDE_DIRS})

#
# module
#
set(SOURCE_FILES
  ndi_mod.cpp
  ndi_mod_audio.cpp
)

add_library(lua-extension MODULE ${SOURCE_FILES})
set_target_properties(lua-extension PROPERTIES
  PREFIX ""     # strip `lib` prefix
  OUTPUT_NAME "${LUA_MODULE_NAME}"
)

set(NORNS_SOURCE_PATH "${CMAKE_SOURCE_DIR}/dep/norns")

target_include_directories(lua-extension PUBLIC
  "${NORNS_SOURCE_PATH}/matron/src"
  "${NDI_SDK_PATH}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "/usr/include/cairo"
)

target_link_directories(lua-extension PUBLIC
  "${NDI_SDK_PATH}/lib"
  ${JACK_LIBRARY_DIRS}
)

target_link_libraries(lua-extension PUBLIC
  ndi
  ${JACK_LIBRARIES}
)

