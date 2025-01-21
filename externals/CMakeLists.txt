add_subdirectory(glfw)

add_subdirectory(Vulkan-Headers)

add_subdirectory(volk)

add_subdirectory(glm)

add_library(stb INTERFACE)
target_sources(stb INTERFACE
    stb/stb_image.h
)
target_include_directories(stb INTERFACE ./stb/)

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

set(UPDATE_DEPS ON)
add_subdirectory(Vulkan-Utility-Libraries)

set(BUILD_TESTS OFF)
add_subdirectory(Vulkan-ValidationLayers)

get_target_property(VkLayer_utils_BINARY_DIR VkLayer_utils BINARY_DIR)
set(VkLayer_utils_dir "${VkLayer_utils_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
add_custom_command(OUTPUT "${VkLayer_utils_dir}/VkLayer_khronos_validation.dll"
    COMMAND echo Validation Layers will be built to ${VkLayer_utils_dir}
    COMMAND echo Add this path to the VK_LAYER_PATH environment variable to enable it
    DEPENDS VkLayer_utils
)
add_custom_target(show_validation_layer_path ALL
    DEPENDS "${VkLayer_utils_dir}/VkLayer_khronos_validation.dll"
)