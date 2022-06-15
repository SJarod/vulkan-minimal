#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>

int main()
{
    glfwInit();

    if (!glfwVulkanSupported())
    {
        std::cerr << "GLFW failed to find the Vulkan loader.\nExiting ...\n";
        fflush(stdout);
        exit(1);
    }

    if (!gladLoaderLoadVulkan(nullptr, nullptr, nullptr))
    {
        std::cerr << "Unable to load Vulkan symbols!\ngladLoad Failure\n";
        exit(2);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}