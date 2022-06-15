#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

VkInstance instance;

int vulkanInit()
{
    if (!glfwVulkanSupported())
    {
        std::cerr << "GLFW failed to find the Vulkan loader.\nExiting ...\n";
        fflush(stdout);
        return -1;
    }

    if (!gladLoaderLoadVulkan(nullptr, nullptr, nullptr))
    {
        std::cerr << "Unable to load Vulkan symbols!\ngladLoad Failure\n";
        return -2;
    }
}

int vulkanCreate()
{
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions
    };

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        std::cerr << "Failed to create vulkan instance\n";
        return -1;
    }
}

void vulkanExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    std::cout << "available extensions : " << extensionCount << '\n';
    for (const auto& extension : extensions)
        std::cout << '\t' << extension.extensionName << '\n';
}

int main()
{
    glfwInit();
    if (vulkanInit() < 0) return -1;
    if (vulkanCreate() < 0) return -2;
    vulkanExtensions();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}