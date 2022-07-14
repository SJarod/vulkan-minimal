cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)

project(vulkan-tutorial)

add_subdirectory(externals)

add_executable(vk src/main.cpp)
target_include_directories(vk PRIVATE include)
target_link_libraries(vk
	PUBLIC glad
	PUBLIC glfw
	PUBLIC libraries
)