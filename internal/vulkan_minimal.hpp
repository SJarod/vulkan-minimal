#pragma once

// TODO : render in fbo instead of swapchain image directly (google search "bgra vs rgba")

#include <iostream>

#include <volk.h>

#include <array>
#include <limits>
#include <optional>
#include <set>
#include <vector>

#include "utils.hpp"
#include "vertex.hpp"

#include <glm/glm.hpp>

namespace RHI
{
/**
 * @brief load Vulkan symbols
 *
 */
inline void load_symbols()
{
    volkInitialize();
}

namespace Instance
{
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback_instance(VkDebugReportFlagsEXT flags,
                                                                     VkDebugReportObjectTypeEXT objectType,
                                                                     uint64_t object, size_t location,
                                                                     int32_t messageCode, const char *pLayerPrefix,
                                                                     const char *pMessage, void *pUserData)
{
    std::cerr << "[Debug Report Callback Instance, ";
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        std::cerr << "INFORMATION";
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        std::cerr << "WARNING";
    else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        std::cerr << "PERFORMANCE WARNING";
    else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        std::cerr << "ERROR";
    else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        std::cerr << "DEBUG";

    std::cerr << "] : " << pMessage << std::endl;
    return VK_FALSE;
};

inline VkInstance create_instance(std::vector<const char *> layers, std::vector<const char *> instanceExtensions,
                                  bool bDebugReportCallback = true)
{
    VkApplicationInfo appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                 .pApplicationName = "Vulkan Minimal",
                                 .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
                                 .pEngineName = "Vulkan Renderer",
                                 .engineVersion = VK_MAKE_VERSION(0, 0, 0),
                                 .apiVersion = VK_API_VERSION_1_3};

    VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT |
                 VK_DEBUG_REPORT_DEBUG_BIT_EXT,
        .pfnCallback = debug_report_callback_instance,
        .pUserData = nullptr,
    };
    VkInstanceCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                       .pNext = bDebugReportCallback ? &debugReportCreateInfo : nullptr,
                                       .pApplicationInfo = &appInfo,
                                       .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                                       .ppEnabledLayerNames = layers.data(),
                                       .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
                                       .ppEnabledExtensionNames = instanceExtensions.data()};

    VkInstance instance;
    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create Vulkan instance : " << res << std::endl;

    volkLoadInstance(instance);

    return instance;
}
inline void destroy_instance(VkInstance &instance)
{
    vkDestroyInstance(instance, nullptr);
}
inline std::vector<std::string> enumerate_available_layers()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    std::vector<std::string> layerNames(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    std::cout << "available layers : " << layerCount << '\n';
    for (const auto &layer : layers)
    {
        std::cout << '\t' << layer.layerName << '\n';
        layerNames.emplace_back(layer.layerName);
    }

    return layerNames;
}
inline void enumerate_available_instance_extensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    std::cout << "available instance extensions : " << extensionCount << '\n';
    for (const auto &extension : extensions)
        std::cout << '\t' << extension.extensionName << '\n';
}

namespace Debug
{
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                               VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                               const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                               void *userData)
{
    std::cerr << "[Validation Layer, ";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        std::cerr << "VERBOSE, ";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        std::cerr << "INFO, ";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cerr << "WARNING, ";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        std::cerr << "ERROR, ";

    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        std::cerr << "GENERAL] : ";
    else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        std::cerr << "VALIDATION] : ";
    else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        std::cerr << "PERFORMANCE] : ";

    std::cerr << callbackData->pMessage << std::endl;
    return VK_FALSE;
}

inline VkDebugUtilsMessengerEXT create_debug_messenger(
    VkInstance instance,
    VkDebugUtilsMessageSeverityFlagsEXT messageType = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                                                     .messageSeverity = messageType,
                                                     .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                                                     .pfnUserCallback = debug_messenger_callback,
                                                     .pUserData = nullptr};

    VkDebugUtilsMessengerEXT messenger;
    VkResult res = vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &messenger);
    if (res != VK_SUCCESS)
        std::cerr << "Failed create debug messenger : " << res << std::endl;

    return messenger;
}
inline void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
    vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags,
                                                            VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                                            size_t location, int32_t messageCode,
                                                            const char *pLayerPrefix, const char *pMessage,
                                                            void *pUserData)
{
    std::cerr << "[Debug Report Callback, ";
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        std::cerr << "INFORMATION, ";
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        std::cerr << "WARNING, ";
    else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        std::cerr << "PERFORMANCE WARNING, ";
    else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        std::cerr << "ERROR, ";
    else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        std::cerr << "DEBUG, ";

    std::cerr << objectType << ", " << object << ", " << messageCode << "] : ";

    std::cerr << pMessage << std::endl;
    return VK_FALSE;
};

inline VkDebugReportCallbackEXT create_debug_report_callback(
    VkInstance instance,
    VkDebugReportFlagsEXT messageType = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                        VK_DEBUG_REPORT_DEBUG_BIT_EXT)
{
    VkDebugReportCallbackCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = messageType,
        .pfnCallback = debug_report_callback,
        .pUserData = nullptr,
    };

    VkDebugReportCallbackEXT callback;
    VkResult res = vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create debug report callback : " << res << std::endl;

    return callback;
}
inline void destroy_debug_report_callback(VkInstance instance, VkDebugReportCallbackEXT callback)
{
    vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
}
} // namespace Debug
} // namespace Instance

namespace Device
{
inline void enumerate_available_physical_devices(VkInstance instance)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(count);
    vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());
    std::cout << "available devices : " << count << '\n';
    for (const auto &physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        std::cout << '\t' << properties.deviceName << '\n';
    }
}
inline std::vector<VkPhysicalDevice> get_physical_devices(VkInstance instance)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(count);
    vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());

    return physicalDevices;
}

inline void enumerate_available_device_extensions(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data());
    std::cout << "available device extensions for " << properties.deviceName << " : " << count << '\n';
    for (const auto &extension : extensions)
        std::cout << '\t' << extension.extensionName << '\n';
}
inline bool is_device_compatible_with_extensions(VkPhysicalDevice physicalDevice,
                                                 const std::vector<const char *> deviceExtensions)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
    std::cout << "available device extensions : " << extensionCount << '\n';
    for (const auto &extension : extensions)
        std::cout << '\t' << extension.extensionName << '\n';

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const VkExtensionProperties &extension : extensions)
    {
        // is the required extension available?
        requiredExtensions.erase(extension.extensionName);
    }

    // are all required extensions found in the available extension list?
    return requiredExtensions.empty();
}

namespace Memory
{
inline std::optional<uint32_t> find_memory_type_index(VkPhysicalDevice physicalDevice,
                                                      VkMemoryRequirements requirements,
                                                      VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProp);

    for (uint32_t i = 0; i < memProp.memoryTypeCount; ++i)
    {
        bool rightType = requirements.memoryTypeBits & (1 << i);
        bool rightFlag = (memProp.memoryTypes[i].propertyFlags & properties) == properties;
        if (rightType && rightFlag)
            return std::optional<uint32_t>(i);
    }

    std::cerr << "Failed to find suitable memory type" << std::endl;
    return std::optional<uint32_t>();
}
} // namespace Memory

namespace Queue
{
inline std::optional<uint32_t> find_queue_family_index(VkPhysicalDevice physicalDevice, VkQueueFlags flags)
{
    std::optional<uint32_t> index;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        // queue family capable of specified flags operations
        if (queueFamilies[i].queueFlags & flags)
            index = i;
    }

    return index;
}
inline std::optional<uint32_t> find_present_queue_family_index(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    std::optional<uint32_t> index;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        // queue family capable of presentation
        VkBool32 bPresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &bPresentSupport);
        if (bPresentSupport)
            index = i;
    }

    return index;
}

/**
 * get a queue from a specified queue family
 */
inline VkQueue get_device_queue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex)
{
    VkQueue queue;
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &queue);
    return queue;
}
} // namespace Queue

inline VkDevice create_logical_device(VkInstance instance, VkPhysicalDevice physicalDevice, VkSurfaceKHR *surface,
                                      std::vector<const char *> layers, std::vector<const char *> deviceExtensions)
{
    std::optional<uint32_t> graphicsFamilyIndex = Queue::find_queue_family_index(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
    std::optional<uint32_t> presentFamilyIndex;
    if (surface)
        presentFamilyIndex = Queue::find_present_queue_family_index(physicalDevice, *surface);

    std::set<uint32_t> queueFamilyIndices;
    if (graphicsFamilyIndex.has_value())
        queueFamilyIndices.insert(graphicsFamilyIndex.value());
    if (presentFamilyIndex.has_value())
        queueFamilyIndices.insert(presentFamilyIndex.value());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.f;
    for (uint32_t queueFamilyIndex : queueFamilyIndices)
    {
        queueCreateInfos.emplace_back(VkDeviceQueueCreateInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                              .queueFamilyIndex = queueFamilyIndex,
                                                              .queueCount = 1,
                                                              .pQueuePriorities = &queuePriority});
    }

    VkDeviceCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                     .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                                     .pQueueCreateInfos = queueCreateInfos.data(),
                                     .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                                     .ppEnabledLayerNames = layers.data(),
                                     .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                                     .ppEnabledExtensionNames = deviceExtensions.data()};

    VkDevice device;
    VkResult res = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create logical device : " << res << std::endl;

    volkLoadDevice(device);

    return device;
}
inline void destroy_logical_device(VkDevice device)
{
    vkDestroyDevice(device, nullptr);
}

} // namespace Device

namespace Presentation
{
namespace Surface
{
typedef VkResult (*PFN_CreateSurfacePredicate)(VkInstance instance, void *window, VkAllocationCallbacks *allocator,
                                               VkSurfaceKHR *surface);

inline VkSurfaceKHR create_surface(PFN_CreateSurfacePredicate predicate, VkInstance instance, void *window,
                                   VkAllocationCallbacks *allocator)
{
    VkSurfaceKHR surface;
    predicate(instance, window, allocator, &surface);
    return surface;
}
inline void destroy_surface(VkInstance instance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

inline VkSurfaceCapabilitiesKHR get_surface_capabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
    return capabilities;
}
inline std::vector<VkSurfaceFormatKHR> get_surface_available_formats(VkPhysicalDevice physicalDevice,
                                                                     VkSurfaceKHR surface)
{
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());
    return formats;
}
inline std::vector<VkPresentModeKHR> get_surface_available_present_modes(VkPhysicalDevice physicalDevice,
                                                                         VkSurfaceKHR surface)
{
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> presentModes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, presentModes.data());
    return presentModes;
}

inline std::optional<VkSurfaceFormatKHR> find_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats,
                                                             const VkFormat &targetFormat,
                                                             const VkColorSpaceKHR &targetColorSpace)
{
    for (const VkSurfaceFormatKHR &availableFormat : availableFormats)
    {
        if (availableFormat.format == targetFormat && availableFormat.colorSpace == targetColorSpace)
            return std::optional<VkSurfaceFormatKHR>(availableFormat);
    }
    return std::optional<VkSurfaceFormatKHR>();
}
inline std::optional<VkPresentModeKHR> find_surface_present_mode(const std::vector<VkPresentModeKHR> &availableModes,
                                                                 const VkPresentModeKHR &targetPresentMode)
{
    for (const VkPresentModeKHR &availableMode : availableModes)
    {
        if (availableMode == targetPresentMode)
            return std::optional<VkPresentModeKHR>(availableMode);
    }
    return std::optional<VkPresentModeKHR>();
}
inline VkExtent2D find_extent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t width, uint32_t height)
{
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
    {
        return capabilities.currentExtent;
    }
    else
    {
        return VkExtent2D{
            .width = glm::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = glm::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
        };
    }
}
} // namespace Surface

namespace SwapChain
{
inline VkSwapchainKHR create_swap_chain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface,
                                        VkSurfaceFormatKHR surfaceFormat, uint32_t width, uint32_t height)
{
    VkSurfaceCapabilitiesKHR capabilities = Surface::get_surface_capabilities(physicalDevice, surface);

    std::vector<VkSurfaceFormatKHR> formats = Surface::get_surface_available_formats(physicalDevice, surface);

    std::vector<VkPresentModeKHR> presentModes = Surface::get_surface_available_present_modes(physicalDevice, surface);
    std::optional<VkPresentModeKHR> presentMode =
        Surface::find_surface_present_mode(presentModes, VK_PRESENT_MODE_FIFO_KHR);

    VkExtent2D extent = Surface::find_extent(Surface::get_surface_capabilities(physicalDevice, surface), width, height);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                           .surface = surface,
                                           .minImageCount = imageCount,
                                           .imageFormat = surfaceFormat.format,
                                           .imageColorSpace = surfaceFormat.colorSpace,
                                           .imageExtent = extent,
                                           .imageArrayLayers = 1,
                                           .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                           .preTransform = capabilities.currentTransform,
                                           .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                           .presentMode = presentMode.value(),
                                           .clipped = VK_TRUE,
                                           .oldSwapchain = VK_NULL_HANDLE};

    std::optional<uint32_t> graphicsFamilyIndex =
        Device::Queue::find_queue_family_index(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
    std::optional<uint32_t> presentFamilyIndex;
    if (surface)
        Device::Queue::find_present_queue_family_index(physicalDevice, surface);

    std::vector<uint32_t> queueFamilyIndices;
    if (graphicsFamilyIndex.has_value())
        queueFamilyIndices.emplace_back(graphicsFamilyIndex.value());
    if (presentFamilyIndex.has_value())
        queueFamilyIndices.emplace_back(presentFamilyIndex.value());

    if ((graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value()) &&
        graphicsFamilyIndex.value() != presentFamilyIndex.value())
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    VkSwapchainKHR swapchain;
    VkResult res = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create swapchain : " << res << std::endl;

    return swapchain;
}
inline void destroy_swap_chain(VkDevice device, VkSwapchainKHR swapchain)
{
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

inline std::vector<VkImage> get_swap_chain_images(VkDevice device, VkSwapchainKHR swapchain)
{
    uint32_t count;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
    return images;
}
} // namespace SwapChain
} // namespace Presentation

namespace RenderPass
{
inline VkRenderPass create_render_pass(VkDevice device, VkFormat imageFormat)
{
    VkAttachmentDescription colorAttachment = {.format = imageFormat,
                                               .samples = VK_SAMPLE_COUNT_1_BIT,
                                               // load : what to do with the already existing image on the framebuffer
                                               .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                               // store : what to do with the newly rendered image on the framebuffer
                                               .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                               .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                               .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                               .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    VkAttachmentReference colorAttachmentRef = {.attachment = 0, // colorAttachment is index 0
                                                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    .colorAttachmentCount = 1,
                                    .pColorAttachments = &colorAttachmentRef};

    VkSubpassDependency dependency = {.srcSubpass = VK_SUBPASS_EXTERNAL,
                                      .dstSubpass = 0,
                                      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      .srcAccessMask = 0,
                                      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

    VkRenderPassCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                         .attachmentCount = 1,
                                         .pAttachments = &colorAttachment,
                                         .subpassCount = 1,
                                         .pSubpasses = &subpass,
                                         .dependencyCount = 1,
                                         .pDependencies = &dependency};

    VkRenderPass renderPass;
    VkResult res = vkCreateRenderPass(device, &createInfo, nullptr, &renderPass);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create render pass : " << res << std::endl;

    return renderPass;
}
inline void destroy_render_pass(VkDevice device, VkRenderPass renderPass)
{
    vkDestroyRenderPass(device, renderPass, nullptr);
}

inline std::vector<VkFramebuffer> create_framebuffers(VkDevice device, VkRenderPass renderPass,
                                                      std::vector<VkImageView> swapchainImageViews, VkExtent2D extent)
{
    std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        VkFramebufferCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                              .renderPass = renderPass,
                                              .attachmentCount = 1,
                                              .pAttachments = &swapchainImageViews[i],
                                              .width = extent.width,
                                              .height = extent.height,
                                              .layers = 1};

        VkResult res = vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffers[i]);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to create framebuffer : " << res << std::endl;
    }
    return framebuffers;
}
inline void destroy_framebuffers(VkDevice device, std::vector<VkFramebuffer> framebuffers)
{
    for (VkFramebuffer &framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}
} // namespace RenderPass

namespace Pipeline
{
namespace Shader
{
struct UniformBufferObjectT
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

inline VkShaderModule create_shader_module(VkDevice device, const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                           .codeSize = code.size(),
                                           .pCode = reinterpret_cast<const uint32_t *>(code.data())};

    VkShaderModule module;
    VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &module);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create shader module : " << res << std::endl;

    return module;
}
inline void destroy_shader_module(VkDevice device, VkShaderModule module)
{
    vkDestroyShaderModule(device, module, nullptr);
}

// binding the data to the vertex shader
inline VkVertexInputBindingDescription get_vertex_binding_description()
{
    // describe the buffer data
    VkVertexInputBindingDescription desc = {.binding = 0,
                                            .stride = sizeof(Vertex),
                                            // update every vertex (opposed to VK_VERTEX_INPUT_RATE_INSTANCE)
                                            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    return desc;
}
// 2 attributes descripction
inline std::array<VkVertexInputAttributeDescription, 3> get_vertex_attribute_description()
{
    // attribute pointer
    std::array<VkVertexInputAttributeDescription, 3> desc;
    desc[0] = {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)};
    desc[1] = {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, color)};
    desc[2] = {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv)};
    return desc;
}

inline VkDescriptorSetLayout create_descriptor_set_layout(VkDevice device,
                                                          std::vector<VkDescriptorSetLayoutBinding> layoutBindings)
{
    VkDescriptorSetLayoutCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
        .pBindings = layoutBindings.data(),
    };

    VkDescriptorSetLayout setLayout;
    VkResult res = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create descriptor set layout : " << res << std::endl;

    return setLayout;
}
inline void destroy_descriptor_set_layout(VkDevice device, VkDescriptorSetLayout setLayout)
{
    vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
}

inline VkPipelineLayout create_pipeline_layout(VkDevice device, const std::vector<VkDescriptorSetLayout> &setLayouts)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                           .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                                           .pSetLayouts = setLayouts.data(),
                                                           .pushConstantRangeCount = 0,
                                                           .pPushConstantRanges = nullptr};

    VkPipelineLayout pipelineLayout;
    VkResult res = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create pipeline layout : " << res << std::endl;

    return pipelineLayout;
}
inline void destroy_pipeline_layout(VkDevice device, VkPipelineLayout pipelineLayout)
{
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

inline VkDescriptorPool create_descriptor_pool(VkDevice device, std::vector<VkDescriptorPoolSize> poolSizes,
                                               uint32_t frameInFlightCount)
{
    VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = frameInFlightCount,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    VkDescriptorPool descriptorPool;
    VkResult res = vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create descriptor pool : " << res << std::endl;

    return descriptorPool;
}
inline void destroy_descriptor_pool(VkDevice device, VkDescriptorPool descriptorPool)
{
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

inline std::vector<VkDescriptorSet> allocate_desriptor_sets(VkDevice device, VkDescriptorPool descriptorPool,
                                                            uint32_t frameInFlightCount,
                                                            const std::vector<VkDescriptorSetLayout> &setLayouts)
{
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = frameInFlightCount,
        .pSetLayouts = setLayouts.data(),
    };

    std::vector<VkDescriptorSet> descriptorSets(frameInFlightCount);
    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
    if (res != VK_SUCCESS)
        std::cerr << "Failed to allocate descriptor sets : " << res << std::endl;

    return descriptorSets;
}

inline void write_descriptor_sets(VkDevice device, std::vector<VkWriteDescriptorSet> writes)
{
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
} // namespace Shader

inline VkPipeline create_pipeline(VkDevice device, VkRenderPass renderPass, const char *shaderName, VkExtent2D extent,
                                  VkPipelineLayout pipelineLayout)
{
    std::vector<char> vs;
    if (!read_binary_file("shaders/" + std::string(shaderName) + ".vert.spv", vs))
        return VK_NULL_HANDLE;
    std::vector<char> fs;
    if (!read_binary_file("shaders/" + std::string(shaderName) + ".frag.spv", fs))
        return VK_NULL_HANDLE;

    VkShaderModule vsModule = Shader::create_shader_module(device, vs);
    VkShaderModule fsModule = Shader::create_shader_module(device, fs);

    VkPipelineShaderStageCreateInfo vsStageCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                         .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                                         .module = vsModule,
                                                         .pName = "main",
                                                         // for shader constants values
                                                         .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo fsStageCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                         .module = fsModule,
                                                         .pName = "main",
                                                         .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[] = {vsStageCreateInfo, fsStageCreateInfo};

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};

    // vertex (enabling the binding for the Vertex structure)
    auto binding = Shader::get_vertex_binding_description();
    auto attribs = Shader::get_vertex_attribute_description();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size()),
        .pVertexAttributeDescriptions = attribs.data()};

    // draw mode
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    // viewport
    VkViewport viewport = {.x = 0.f,
                           .y = 0.f,
                           .width = static_cast<float>(extent.width),
                           .height = static_cast<float>(extent.height),
                           .minDepth = 0.f,
                           .maxDepth = 1.f};

    VkRect2D scissor = {.offset = {0, 0}, .extent = extent};

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.f,
        .depthBiasClamp = 0.f,
        .depthBiasSlopeFactor = 0.f,
        .lineWidth = 1.f};

    // multisampling, anti-aliasing
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    // color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.f, 0.f, 0.f, 0.f}};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                       // shader stage
                                                       .stageCount = 2,
                                                       .pStages = shaderStagesCreateInfo,
                                                       // fixed function stage
                                                       .pVertexInputState = &vertexInputCreateInfo,
                                                       .pInputAssemblyState = &inputAssemblyCreateInfo,
                                                       .pViewportState = &viewportStateCreateInfo,
                                                       .pRasterizationState = &rasterizerCreateInfo,
                                                       .pMultisampleState = &multisamplingCreateInfo,
                                                       .pDepthStencilState = nullptr,
                                                       .pColorBlendState = &colorBlendCreateInfo,
                                                       .pDynamicState = &dynamicStateCreateInfo,
                                                       // pipeline layout
                                                       .layout = pipelineLayout,
                                                       // render pass
                                                       .renderPass = renderPass,
                                                       .subpass = 0,
                                                       .basePipelineHandle = VK_NULL_HANDLE,
                                                       .basePipelineIndex = -1};

    VkPipeline pipeline;
    VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create graphics pipeline : " << res << std::endl;

    Shader::destroy_shader_module(device, vsModule);
    Shader::destroy_shader_module(device, fsModule);

    return pipeline;
}
inline void destroy_pipeline(VkDevice device, VkPipeline pipeline)
{
    vkDestroyPipeline(device, pipeline, nullptr);
}
} // namespace Pipeline

namespace Command
{
inline VkCommandPool create_command_pool(VkDevice device, uint32_t queueFamilyIndex, bool bTransient = false)
{
    VkCommandPoolCreateFlags flags =
        bTransient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCommandPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queueFamilyIndex,
    };

    VkCommandPool commandPool;
    VkResult res = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create command pool : " << res << std::endl;

    return commandPool;
}
inline void destroy_command_pool(VkDevice device, VkCommandPool commandPool)
{
    vkDestroyCommandPool(device, commandPool, nullptr);
}

inline std::vector<VkCommandBuffer> allocate_command_buffers(VkDevice device, VkCommandPool commandPool,
                                                             uint32_t commandBufferCount)
{
    VkCommandBufferAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                             .commandPool = commandPool,
                                             .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                             .commandBufferCount = commandBufferCount};

    std::vector<VkCommandBuffer> commandBuffer(commandBufferCount);
    VkResult res = vkAllocateCommandBuffers(device, &allocInfo, commandBuffer.data());
    if (res != VK_SUCCESS)
        std::cerr << "Failed to allocate command buffers : " << res << std::endl;

    return commandBuffer;
}

inline VkCommandBuffer command_buffer_begin_one_time_submit(VkDevice device, VkCommandPool commandPoolTransient)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPoolTransient,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VkResult res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to begin one time submit command buffer : " << res << std::endl;

    return commandBuffer;
}
inline void command_buffer_end_one_time_submit(VkCommandBuffer commandBuffer, VkDevice device, VkQueue queue,
                                               VkCommandPool commandPoolTransient)
{
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    VkResult res = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to submit one time command buffer : " << res << std::endl;

    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, commandPoolTransient, 1, &commandBuffer);
}
} // namespace Command

namespace Parallel
{
inline VkSemaphore create_semaphore(VkDevice device)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    VkSemaphore semaphore;
    VkResult res = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create semaphore : " << res << std::endl;

    return semaphore;
}
inline void destroy_semaphore(VkDevice device, VkSemaphore semaphore)
{
    vkDestroySemaphore(device, semaphore, nullptr);
}

inline VkFence create_fence(VkDevice device)
{
    VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                         .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    VkFence fence;
    VkResult res = vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create fence : " << res << std::endl;

    return fence;
}
inline void destroy_fence(VkDevice device, VkFence fence)
{
    vkDestroyFence(device, fence, nullptr);
}
} // namespace Parallel

namespace Memory
{
inline VkDeviceMemory allocate_memory(VkDevice device, VkDeviceSize requiredSize, uint32_t memoryTypeIndex)
{
    // VRAM heap
    VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                      .allocationSize = requiredSize,
                                      .memoryTypeIndex = memoryTypeIndex};

    VkDeviceMemory memory;
    VkResult res = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to allocate memory : " << res << std::endl;

    return memory;
}
inline void free_memory(VkDevice device, VkDeviceMemory memory)
{
    vkFreeMemory(device, memory, nullptr);
}

inline void copy_data_to_memory(VkDevice device, VkDeviceMemory memory, const void *srcData, size_t size)
{
    // filling the VBO (bind and unbind CPU accessible memory)
    void *data;
    vkMapMemory(device, memory, 0, size, 0, &data);
    // TODO : flush memory
    memcpy(data, srcData, size);
    // TODO : invalidate memory before reading in the pipeline
    vkUnmapMemory(device, memory);
}

inline void transfer_buffer(VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size,
                            VkCommandPool commandPoolTransient, VkQueue queue)
{
    VkCommandBuffer commandBuffer = Command::command_buffer_begin_one_time_submit(device, commandPoolTransient);
    VkBufferCopy copyRegion = {
        .size = size,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    Command::command_buffer_end_one_time_submit(commandBuffer, device, queue, commandPoolTransient);
}

namespace Buffer
{
/**
 * @brief Create a buffer object
 *
 * @param device
 * @param size
 * @param usage
 * @return VkBuffer
 */
inline VkBuffer create_buffer(VkDevice device, size_t size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                     .flags = 0,
                                     .size = size,
                                     .usage = usage,
                                     .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VkBuffer buffer;
    VkResult res = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create buffer : " << res << std::endl;

    return buffer;
}
inline void destroy_buffer(VkDevice device, VkBuffer buffer)
{
    vkDestroyBuffer(device, buffer, nullptr);
}

inline void bind_memory_to_buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory memory)
{
    vkBindBufferMemory(device, buffer, memory, 0);
}

inline std::pair<VkBuffer, VkDeviceMemory> create_allocated_buffer(
    VkDevice device, VkPhysicalDevice physicalDevice, size_t size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    VkBuffer buffer = create_buffer(device, size, usage);
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, buffer, &memReq);
    std::optional<uint32_t> memoryTypeIndex =
        Device::Memory::find_memory_type_index(physicalDevice, memReq, properties);

    VkDeviceMemory memory = allocate_memory(device, memReq.size, memoryTypeIndex.value());
    bind_memory_to_buffer(device, buffer, memory);

    return {buffer, memory};
}

/**
 * @brief Create a optimal buffer from data object by using a staging buffer
 *
 * @param device
 * @param physicalDevice
 * @param size
 * @param data
 * @param commandPoolTransient
 * @param graphicsQueue
 * @return std::pair<VkBuffer, VkDeviceMemory>
 */
inline std::pair<VkBuffer, VkDeviceMemory> create_optimal_buffer_from_data(
    VkDevice device, VkPhysicalDevice physicalDevice, size_t size, const void *data, VkCommandPool commandPoolTransient,
    VkQueue graphicsQueue, VkBufferUsageFlags usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    // staging buffer

    auto stagingBuffer = create_allocated_buffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    copy_data_to_memory(device, stagingBuffer.second, data, size);

    // buffer

    auto buffer = create_allocated_buffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    transfer_buffer(device, stagingBuffer.first, buffer.first, size, commandPoolTransient, graphicsQueue);

    free_memory(device, stagingBuffer.second);
    destroy_buffer(device, stagingBuffer.first);

    return buffer;
}
} // namespace Buffer

namespace Image
{
inline VkImage create_image(VkDevice device, uint32_t width, uint32_t height,
                            VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
                            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL)
{
    VkImage image;

    VkImageCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent =
            {
                .width = width,
                .height = height,
                .depth = 1U,
            },
        .mipLevels = 1U,
        .arrayLayers = 1U,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkResult res = vkCreateImage(device, &createInfo, nullptr, &image);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create image : " << res << std::endl;
        return VK_NULL_HANDLE;
    }

    return image;
}
inline void destroy_image(VkDevice device, VkImage image)
{
    vkDestroyImage(device, image, nullptr);
}

inline void bind_memory_to_image(VkDevice device, VkImage image, VkDeviceMemory memory)
{
    vkBindImageMemory(device, image, memory, 0);
}

inline std::pair<VkImage, VkDeviceMemory> create_allocated_image(
    VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    VkImage image = create_image(device, width, height, usage, format, tiling);
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, image, &memReq);
    std::optional<uint32_t> memoryTypeIndex =
        Device::Memory::find_memory_type_index(physicalDevice, memReq, properties);

    VkDeviceMemory memory = allocate_memory(device, memReq.size, memoryTypeIndex.value());
    bind_memory_to_image(device, image, memory);

    return {image, memory};
}

inline void transition_image_layout(VkDevice device, VkCommandPool commandPoolTransient, VkQueue queue,
                                    VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image,
                                    VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                    VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    VkCommandBuffer commandBuffer = Command::command_buffer_begin_one_time_submit(device, commandPoolTransient);

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    Command::command_buffer_end_one_time_submit(commandBuffer, device, queue, commandPoolTransient);
}

void copy_buffer_to_image(VkDevice device, VkCommandPool commandPoolTransient, uint32_t width, uint32_t height,
                          VkBuffer buffer, VkImage image, VkQueue queue)
{
    VkCommandBuffer commandBuffer = Command::command_buffer_begin_one_time_submit(device, commandPoolTransient);

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset =
            {
                .x = 0,
                .y = 0,
                .z = 0,
            },
        .imageExtent =
            {
                .width = width,
                .height = height,
                .depth = 1,
            },
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    Command::command_buffer_end_one_time_submit(commandBuffer, device, queue, commandPoolTransient);
}

inline std::pair<VkImage, VkDeviceMemory> create_image_texture_from_data(
    VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, void *data,
    VkCommandPool commandPoolTransient, VkQueue graphicsQueue, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB)
{
    size_t imageSize = width * height * 4;

    auto stagingBuffer =
        Buffer::create_allocated_buffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    copy_data_to_memory(device, stagingBuffer.second, data, imageSize);

    auto image = create_allocated_image(device, physicalDevice, width, height,
                                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, imageFormat,
                                        VK_IMAGE_TILING_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    transition_image_layout(device, commandPoolTransient, graphicsQueue, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image.first, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    copy_buffer_to_image(device, commandPoolTransient, width, height, stagingBuffer.first, image.first, graphicsQueue);
    transition_image_layout(device, commandPoolTransient, graphicsQueue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, image.first, VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    free_memory(device, stagingBuffer.second);
    Buffer::destroy_buffer(device, stagingBuffer.first);

    return image;
}

inline VkImageView create_image_view(VkDevice device, VkImage image, VkFormat imageFormat)
{
    VkImageViewCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                        .image = image,
                                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                        .format = imageFormat,
                                        .components =
                                            {
                                                .r = VK_COMPONENT_SWIZZLE_R,
                                                .g = VK_COMPONENT_SWIZZLE_G,
                                                .b = VK_COMPONENT_SWIZZLE_B,
                                                .a = VK_COMPONENT_SWIZZLE_A,
                                            },
                                        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                             .baseMipLevel = 0,
                                                             .levelCount = 1,
                                                             .baseArrayLayer = 0,
                                                             .layerCount = 1}};

    VkImageView imageView;
    VkResult res = vkCreateImageView(device, &createInfo, nullptr, &imageView);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create image view : " << res << std::endl;

    return imageView;
}
inline void destroy_image_view(VkDevice device, VkImageView imageView)
{
    vkDestroyImageView(device, imageView, nullptr);
}

inline VkSampler create_image_sampler(VkDevice device, VkFilter filter, bool bEnableAnisotropy = false,
                                      float maxAnisotropy = 1.f)
{
    VkSamplerCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = filter,
        .minFilter = filter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.f,
        .anisotropyEnable = bEnableAnisotropy,
        .maxAnisotropy = maxAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.f,
        .maxLod = 0.f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VkSampler sampler;
    VkResult res = vkCreateSampler(device, &createInfo, nullptr, &sampler);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to create image sampler : " << res << std::endl;

    return sampler;
}
inline void destroy_image_sampler(VkDevice device, VkSampler sampler)
{
    vkDestroySampler(device, sampler, nullptr);
}
} // namespace Image
} // namespace Memory

namespace Render
{
inline uint32_t acquire_back_buffer(VkDevice device, VkSwapchainKHR swapchain, VkSemaphore &acquireSemaphore,
                                    VkFence &backBufferFence)
{
    vkWaitForFences(device, 1, &backBufferFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &backBufferFence);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, acquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to acquire next image : " << res << std::endl;
        return -1;
    }

    return imageIndex;
}

inline void record_back_buffer_begin_render_pass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,
                                                 VkFramebuffer framebuffer, VkExtent2D extent, VkPipeline pipeline)
{
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = 0, .pInheritanceInfo = nullptr};
    VkResult res = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to begin recording command buffer : " << res << std::endl;
        return;
    }

    VkClearValue clearColor = {.color = {0.2f, 0.2f, 0.2f, 1.f}};
    VkRenderPassBeginInfo renderPassBeginInfo = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                                 .renderPass = renderPass,
                                                 .framebuffer = framebuffer,
                                                 .renderArea = {.offset = {0, 0}, .extent = extent},
                                                 .clearValueCount = 1,
                                                 .pClearValues = &clearColor};
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport = {.x = 0.f,
                           .y = 0.f,
                           .width = static_cast<float>(extent.width),
                           .height = static_cast<float>(extent.height),
                           .minDepth = 0.f,
                           .maxDepth = 1.f};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor = {.offset = {0, 0}, .extent = extent};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
inline void record_back_buffer_descriptor_sets_commands(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                                                        VkDescriptorSet descriptorSet)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0,
                            nullptr);
}
inline void record_back_buffer_draw_object_commands(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer,
                                                    uint32_t vertexCount)
{
    VkBuffer vbos[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vbos, offsets);
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}
inline void record_back_buffer_draw_indexed_object_commands(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer,
                                                            VkBuffer indexBuffer, uint32_t indexCount)
{
    VkBuffer vbos[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vbos, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}
inline void record_back_buffer_end_render_pass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);

    VkResult res = vkEndCommandBuffer(commandBuffer);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to record command buffer : " << res << std::endl;
}

inline void submit_back_buffer(VkQueue graphicsQueue, VkCommandBuffer commandBuffer, VkSemaphore &acquireSemaphore,
                               VkSemaphore &renderSemaphore, VkFence &inFlightFence)
{
    VkSemaphore waitSemaphores[] = {acquireSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderSemaphore};
    VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                               .waitSemaphoreCount = 1,
                               .pWaitSemaphores = waitSemaphores,
                               .pWaitDstStageMask = waitStages,
                               .commandBufferCount = 1,
                               .pCommandBuffers = &commandBuffer,
                               .signalSemaphoreCount = 1,
                               .pSignalSemaphores = signalSemaphores};

    VkResult res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to submit draw command buffer : " << res << std::endl;
}

inline void present_back_buffer(VkQueue presentQueue, VkSwapchainKHR swapchain, uint32_t imageIndex,
                                VkSemaphore &renderSemaphore)
{
    VkSwapchainKHR swapchains[] = {swapchain};
    VkSemaphore waitSemaphores[] = {renderSemaphore};
    VkPresentInfoKHR presentInfo = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                    .waitSemaphoreCount = 1,
                                    .pWaitSemaphores = waitSemaphores,
                                    .swapchainCount = 1,
                                    .pSwapchains = swapchains,
                                    .pImageIndices = &imageIndex,
                                    .pResults = nullptr};

    VkResult res = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (res != VK_SUCCESS)
        std::cerr << "Failed to present : " << res << std::endl;
}
} // namespace Render
} // namespace RHI