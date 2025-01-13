# **Vulkan-Minimal**

Rendering a simple triangle with Vulkan.

## Summary
- [Getting started](#getting-started)
    - [Installation](#installation)
    - [Shaders](#shaders)
- [Third-parties](#third-parties)

# Getting started

## Installation
This is a CMake-based project so make sure to install CMake ([portable version](https://github.com/Kitware/CMake/releases/download/v3.26.0-rc5/cmake-3.26.0-rc5-windows-x86_64.zip) : do not forget to add bin directory to PATH).

[Install Python](https://www.python.org/ftp/python/3.11.2/python-3.11.2-amd64.exe) since it is used to compile shaderc.

Download the [latest Vulkan SDK](https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe) from [LunarG's website](https://vulkan.lunarg.com/sdk/home#), it is used to make the Vulkan validations layers available.

## Shaders
Compile the shaders running the command (it should be done automatically in a post-build event) :
```
glslc shader.vert -o shader.vert.spv
glslc shader.frag -o shader.frag.spv
```

# Third-parties
- GLFW
    - https://www.glfw.org/
- Python
    - https://www.python.org/
- CMake
    - https://cmake.org/
- Vulkan SDK
    - https://vulkan.lunarg.com/
- shaderc
    - https://github.com/google/shaderc
- volk
    - https://github.com/zeux/volk
- glm
    - https://github.com/g-truc/glm