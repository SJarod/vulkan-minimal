#pragma once

#include <iostream>
#include <vector>

#include <volk.h>
#include <GLFW/glfw3.h>

// Window System Integration
namespace WSI
{

inline void init()
{
    glfwInit();
}
inline void terminate()
{
    glfwTerminate();
}

// Window creation

inline GLFWwindow *create_window(int width, int height)
{
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // no api specified to create vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window = glfwCreateWindow(width, height, "Vulkan Minimal", nullptr, nullptr);
    return window;
}
inline void destroy_window(GLFWwindow *window)
{
    glfwDestroyWindow(window);
}

inline void is_vulkan_supported()
{
    if (!glfwVulkanSupported())
        std::cerr << "Vulkan is not supported by the window API" << std::endl;
}

inline const std::vector<const char *> get_required_extensions()
{
    uint32_t count = 0;
    const char **extensions;

    extensions = glfwGetRequiredInstanceExtensions(&count);

    return std::vector<const char *>(extensions, extensions + count);
}

inline void make_context_current(GLFWwindow *window)
{
    glfwMakeContextCurrent(window);
}

inline void swap_buffers(GLFWwindow *window)
{
    glfwSwapBuffers(window);
}
inline void poll_events()
{
    glfwPollEvents();
}

inline bool should_close(GLFWwindow *window)
{
    return glfwWindowShouldClose(window);
}

// Window surface

/**
 * return success
 */
inline VkResult create_presentation_surface(VkInstance instance, void *window, VkAllocationCallbacks *allocator,
                                            VkSurfaceKHR *surface)
{
    VkResult res = glfwCreateWindowSurface(instance, (GLFWwindow *)window, allocator, surface);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create window surface : " << res << std::endl;
    return res;
}
} // namespace WSI