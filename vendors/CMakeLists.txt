add_library(glad INTERFACE)
target_sources(glad INTERFACE
	glad/include/vk_platform.h
	glad/include/glad/vulkan.h
	glad/src/vulkan.c
)
target_include_directories(glad INTERFACE glad/include)

add_subdirectory(glfw)