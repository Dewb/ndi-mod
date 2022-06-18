cmake_minimum_required(VERSION 3.2)

project(ndi-mod)

add_subdirectory(src)

# packaging for the mod (this could be much improved)

set(MOD_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")
set(NDI_SDK_PATH "${CMAKE_SOURCE_DIR}/dep/ndi")

add_custom_target(copy-mod ALL COMMAND
    ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/mod
        ${MOD_OUTPUT_PATH}
)

add_custom_target(copy-extension ALL COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lua-extension> "${MOD_OUTPUT_PATH}/lib/" &&
    ${CMAKE_COMMAND} -E copy "${NDI_SDK_PATH}/lib/*" "${MOD_OUTPUT_PATH}/lib/"
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
        "/home/we/dust/code/${CMAKE_PROJECT_NAME}"
)
add_dependencies(install-dust copy-extension copy-mod)