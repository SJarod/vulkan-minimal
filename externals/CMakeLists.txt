# glad
add_subdirectory(glad/cmake)
# glad settings for vulkan
glad_add_library(glad_vk_13
	LOADER
	API vulkan=1.3
	EXTENSIONS VK_EXT_debug_utils,VK_KHR_surface,VK_KHR_swapchain
)

# glfw
add_subdirectory(glfw)

# mathematics
add_subdirectory(mathematics)

# glslc
add_subdirectory(glslang)

# spirv
set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
set(SPIRV_HEADERS_SKIP_INSTALL ON)
add_subdirectory(spirv-headers)
add_subdirectory(spirv-tools)

# shaderc
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_SKIP_TESTS ON)
add_subdirectory(shaderc)