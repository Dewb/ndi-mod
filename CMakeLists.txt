cmake_minimum_required(VERSION 3.2)

project(ndi-mod)

# packaging for the mod (this could be much improved)

set(MOD_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")
set(NDI_SDK_PATH "${CMAKE_SOURCE_DIR}/dep/ndi")
set(MOD_INSTALL_PATH "/home/we/dust/code/${CMAKE_PROJECT_NAME}")

set(CMAKE_BUILD_RPATH "${MOD_INSTALL_PATH}/lib")
set(CMAKE_INSTALL_RPATH "${MOD_INSTALL_PATH}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH FALSE)

add_subdirectory(src)

add_custom_target(copy-mod ALL COMMAND
    ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/mod
        ${MOD_OUTPUT_PATH}
)

add_custom_target(copy-extension ALL COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lua-extension> "${MOD_OUTPUT_PATH}/lib/" &&
    ${CMAKE_COMMAND} -E copy "${NDI_SDK_PATH}/lib/libndi.so" "${MOD_OUTPUT_PATH}/lib/libndi.so.5"
)
add_dependencies(copy-extension copy-mod)

add_custom_target(install-zip COMMAND
    ${CMAKE_COMMAND} -E tar "cfv" "${MOD_OUTPUT_PATH}.zip" --format=zip
        "${MOD_OUTPUT_PATH}"
)
add_dependencies(install-zip copy-extension copy-mod)


add_custom_target(install-dust COMMAND
    ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}
        ${MOD_INSTALL_PATH}
)
add_dependencies(install-dust copy-extension copy-mod)