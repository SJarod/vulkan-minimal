add_subdirectory(glfw)

add_subdirectory(volk)

add_subdirectory(glm)

set(SPIRV_HEADERS_ENABLE_TESTS OFF)
add_subdirectory(SPIRV-Headers)
set(SPIRV_SKIP_EXECUTABLES ON)
set(SPIRV_SKIP_TESTS ON)
add_subdirectory(SPIRV-Tools)
set(GLSLANG_TESTS OFF)
add_subdirectory(glslang)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
if (MSVC)
    set(SHADERC_ENABLE_SHARED_CRT ON)
endif(MSVC)
add_subdirectory(shaderc)