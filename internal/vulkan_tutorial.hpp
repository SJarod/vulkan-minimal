#pragma once

// TODO : render in fbo instead of swapchain image directly (google search "bgra vs rgba")
// TODO : swapchain image format : unorm
// TODO : present mode fifo to lock fps to screen refresh rate
// TODO : faire comme une header only lib
// TODO : ne plus utiliser glad
// TODO : glslc automatique


#include <glad/vulkan.h>
#include <GLFW/glfw3.h>


#include <vector>
#include <optional>
#include <set>
#include <limits>
#include <array>


#include <iostream>
#include <fstream>



static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
	std::cerr << "validation layer: " << callbackData->pMessage << std::endl;
	return VK_FALSE;
}



// binding the data to the vertex shader
VkVertexInputBindingDescription get_vertex_binding_description()
{
	// describe the buffer data
	VkVertexInputBindingDescription desc = {
		.binding = 0,
		.stride = sizeof(Vertex),
		// update every vertex (opposed to VK_VERTEX_INPUT_RATE_INSTANCE)
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

	return desc;
}

// 2 attributes descripction
std::array<VkVertexInputAttributeDescription, 2> get_vertex_attribute_description()
{
	// attribute pointer
	std::array<VkVertexInputAttributeDescription, 2> desc;
	desc[0] = {
		.location = 0,
		.binding = 0,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = offsetof(Vertex, position)
	};
	desc[1] = {
		.location = 1,
		.binding = 0,
		.format = VK_FORMAT_R32G32B32A32_SFLOAT,
		.offset = offsetof(Vertex, color)
	};
	return desc;
}





// pipeline
void vulkanImageViews();
void vulkanGraphicsPipeline();
VkShaderModule createShaderModule(const std::vector<char>& code);
void vulkanRenderPass();
void vulkanFramebuffers();
void vulkanCommandPool();
void vulkanCommandBuffer();
void recordCommandBuffer(VkCommandBuffer cb, uint32_t imageIndex);
void drawFrame();
// multithreading
void vulkanMultithreadObjects();
// vertex buffer object
void vulkanVertexBuffer();
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

std::vector<VkImage> swapchainImages;
VkFormat swapchainImageFormat;
VkExtent2D swapchainExtent;
std::vector<VkImageView> swapchainImageViews;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
std::vector<VkFramebuffer> swapchainFramebuffers;
VkCommandPool commandPool;
VkCommandBuffer commandBuffer;
VkSemaphore renderReadySemaphore;
VkSemaphore renderDoneSemaphore;
VkFence renderOnceFence;
VkBuffer vbo;
VkDeviceMemory vboMemory;


void vulkan_load_symbols()
{
	if (!glfwVulkanSupported())
		throw std::exception("GLFW failed to find the Vulkan loader");

	if (!gladLoaderLoadVulkan(nullptr, nullptr, nullptr))
		throw std::exception("Unable to load Vulkan symbols");
}


// rendering instance initialization

VkInstance vulkan_create_instance()
{
	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hello triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3
	};

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> enabledExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifndef NDEBUG
	enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
		.ppEnabledExtensionNames = enabledExtensions.data()
	};

	VkInstance instance;
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::exception("Failed to create Vulkan instance");
	if (!gladLoaderLoadVulkan(instance, nullptr, nullptr))
		throw std::exception("Unable to reload Vulkan symbols with Vulkan instance");

	return instance;
}

void vulkan_destroy_instance(VkInstance& instance)
{
	vkDestroyInstance(instance, nullptr);
}

void vulkan_enumerate_available_instance_layers()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
	std::cout << "available layers : " << layerCount << '\n';
	for (const auto& layer : layers)
		std::cout << '\t' << layer.layerName << '\n';
}

void vulkan_enumerate_available_instance_extensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::cout << "available instance extensions : " << extensionCount << '\n';
	for (const auto& extension : extensions)
		std::cout << '\t' << extension.extensionName << '\n';
}


void vulkan_create_debug_messenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
		,
	.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		,
	.pfnUserCallback = debug_callback,
	.pUserData = nullptr
	};

	VkDebugUtilsMessengerEXT debugMessenger;
	if (vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		throw std::exception("Failed to set up debug messenger");

	return debugMessenger;
}

void vulkan_destroy_debug_messenger(VkDebugUtilsMessengerEXT& debugMessenger)
{
	vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
}

// devices

std::vector<VkPhysicalDevice> vulkan_get_physical_devices()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::exception("Failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
	std::cout << "available devices : " << deviceCount << '\n';

	for (const auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		std::cout << '\t' << properties.deviceName << '\n';
	}

	return physicalDevices;
}

namespace Client
{

	bool is_device_suitable(VkPhysicalDevice& physicalDevice)
	{
		//checks to tell if the device can do the given tasks

#if false
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;
#else
		VkQueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		bool extensionSupport = checkDeviceExtensionSupport(physicalDevice);

		bool swapchainSupport = false;
		if (extensionSupport)
		{
			VkSwapchainSupportDetails support = querySwapchainSupport(physicalDevice);
			swapchainSupport = !support.formats.empty() && !support.presentModes.empty();
		}

		return indices.isComplete() && extensionSupport && swapchainSupport;
#endif
	}
}

bool is_device_compatible_with_extensions(VkPhysicalDevice pdevice, const std::vector<const char*> deviceExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extensionCount, extensions.data());
	std::cout << "available device extensions : " << extensionCount << '\n';
	for (const auto& extension : extensions)
		std::cout << '\t' << extension.extensionName << '\n';

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const VkExtensionProperties& extension : extensions)
	{
		// is the required extension available?
		requiredExtensions.erase(extension.extensionName);
	}

	// are all required extensions found in the available extension list?
	return requiredExtensions.empty();
}


VkPhysicalDevice vulkan_get_suitable_physical_device()
{
	auto physicalDevices = vulkan_get_physical_devices();
	VkPhysicalDevice physicalDevice;

	//select the last available device (most likely discrete gpu)
	for (const auto& pd : physicalDevices)
	{
		if (Client::is_device_suitable(pd))
		{
			physicalDevice = pd;
			//break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::exception("Failed to find a suitable GPU");
	if (!gladLoaderLoadVulkan(instance, physicalDevice, nullptr))
		throw std::exception("Unable to reload Vulkan symbols with physical device");

	return physicalDevice;
}



std::optional<uint32_t> vulkan_find_queue_family_operation(VkPhysicalDevice pdevice, VkQueueFlags flags)
{
	std::optional<uint32_t> index;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, queueFamilies.data());
	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		// queue family capable of specified flags operations
		if (queueFamilies[i].queueFlags & flags)
			index = i;
	}

	return index;
}

std::optional<uint32_t> vulkan_find_queue_family_presentation(VkPhysicalDevice pdevice, VkSurfaceKHR surface)
{
	std::optional<uint32_t> index;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queueFamilyCount, queueFamilies.data());
	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		// queue family capable of presentation
		VkBool32 bPresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, surface, &bPresentSupport);
		if (bPresentSupport)
			index = i;
	}

	return index;
}

VkDevice vulkan_create_logical_device(VkPhysicalDevice pdevice, VkSurfaceKHR surface)
{
	auto graphicsFamily = vulkan_find_queue_family_operation(pdevice, VK_QUEUE_GRAPHICS_BIT);
	auto presentFamily = vulkan_find_queue_family_presentation(pdevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };

	float queuePriority = 1.f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queueFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = static_cast<uint32_t>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data()
	};

	VkDevice device;
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::exception("Failed to create logical device");
	if (!gladLoaderLoadVulkan(instance, physicalDevice, device))
		throw std::exception("Unable to reload Vulkan symbols with logical device");

	return device;
}

/**
 * get a queue from a specified queue family
 */
VkQueue vulkan_get_device_queue(VkDevice device, uint32_t queueFamilyIndex)
{
	VkQueue queue;
	vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
	return queue;
}

void vulkan_destroy_logical_device(VkDevice device)
{
	vkDestroyDevice(device, nullptr);
}




VkSurfaceKHR vulkan_create_surface(GLFWwindow* window)
{
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::exception("Failed to create window surface");
	return surface;
}

void vulkan_destroy_surface(VkInstance, VkSurfaceKHR surface)
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}



VkSurfaceCapabilitiesKHR get_surface_capabilities(VkPhysicalDevice pdevice, VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, surface, &details.capabilities);
	return capabilities;
}

std::vector<VkSurfaceFormatKHR> get_surface_available_formats(VkPhysicalDevice pdevice, VkSurfaceKHR surface)
{
	std::vector<VkSurfaceFormatKHR> formats;
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, surface, &formatCount, formats.data());
	}
	return formats;
}

std::vector<VkPresentModeKHR> get_surface_available_present_modes(VkPhysicalDevice pdevice, VkSurfaceKHR surface)
{
	std::vector<VkPresentModeKHR> presentModes;
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, surface, &presentModeCount, presentModes.data());
	}
	return presentModes;
}

namespace Client
{

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes)
	{
		for (const VkPresentModeKHR& availableMode : availableModes)
		{
			if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availableMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				.width = static_cast<uint32_t>(width),
				.height = static_cast<uint32_t>(height)
			};

			actualExtent.width = Math::clamp(actualExtent.width,
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width);
			actualExtent.height = Math::clamp(actualExtent.height,
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

}


// surface and swapchain


VkSwapchainKHR vulkan_create_swap_chain(VkPhysicalDevice physicalDevice)
{
	VkSwapchainSupportDetails support = querySwapchainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes);
	VkExtent2D extent = chooseSwapExtent(support.capabilities);

	uint32_t imageCount = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount > 0 && support.capabilities.maxImageCount < imageCount)
		imageCount = support.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = support.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};

	VkQueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	VkSwapchainKHR swapchain;
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
		throw std::exception("Failed to create swapchain");

	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;

	return swapchain;
}

void vulkan_destroy_swap_chain(VkDevice device, VkSwapchainKHR swapchain)
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}



std::vector<VkImageView> vulkan_create_swap_chain_image_views(VkDevice device, VkSwapchainKHR swapchain)
{
	std::vector<VkImageView> swapchainImageViews;
	swapchainImageViews.resize(swapchainImages.size());

	for (size_t i = 0; i < swapchainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchainImageFormat,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
			throw std::exception("Failed to create an image view");
	}

	return swapchainImageViews;
}

void vulkan_destroy_swap_chain_image_views(VkDevice device, std::vector<VkImageView> swapchainImageViews)
{
	for (VkImageView& imageView : swapchainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}
}



VkShaderModule vulkan_create_shader_module(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::exception("Failed to create shader module");
	return shaderModule;
}

void vulkan_destroy_shader_module(VkDevice device, VkShaderModule module)
{
	vkDestroyShaderModule(device, module, nullptr);
}

void vulkan_create_pipeline_layout(VkDevice device)
{

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	.setLayoutCount = 0,
	.pSetLayouts = nullptr,
	.pushConstantRangeCount = 0,
	.pPushConstantRanges = nullptr
	};

	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::exception("Failed to create pipeline layout");
	return pipelineLayout;
}

void vulkan_destroy_pipeline_layout()
{
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

}

void vulkan_create_pipeline(VkDevice device)
{
	std::vector<char> vs = read_binary_file("shaders/triangle.vert.spv");
	std::vector<char> fs = read_binary_file("shaders/triangle.frag.spv");

	VkShaderModule vsModule = vulkan_create_shader_module(vs);
	VkShaderModule fsModule = vulkan_create_shader_module(fs);

	VkPipelineShaderStageCreateInfo vsStageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vsModule,
		.pName = "main",
		//for shader constants values
		.pSpecializationInfo = nullptr
	};

	VkPipelineShaderStageCreateInfo fsStageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fsModule,
		.pName = "main",
		.pSpecializationInfo = nullptr
	};

	VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[] = {
		vsStageCreateInfo,
		fsStageCreateInfo
	};

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	//vertex buffer (enabling the binding for our Vertex structure)
	auto binding = get_vertex_binding_description();
	auto attribs = get_vertex_attribute_description();
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size()),
		.pVertexAttributeDescriptions = attribs.data()
	};

	//draw mode
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	//viewport
	VkViewport viewport = {
		.x = 0.f,
		.y = 0.f,
		.width = static_cast<float>(swapchainExtent.width),
		.height = static_cast<float>(swapchainExtent.height),
		.minDepth = 0.f,
		.maxDepth = 1.f
	};

	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchainExtent
	};

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};

	//rasterizer
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
		.lineWidth = 1.f
	};

	//multisampling, anti-aliasing
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE
	};

	//color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = { 0.f, 0.f, 0.f, 0.f }
	};

	// shader variables
	VkPipelineLayout pipelineLayout = vulkan_create_pipeline_layout();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		//shader stage
		.stageCount = 2,
		.pStages = shaderStagesCreateInfo,
		//fixed function stage
		.pVertexInputState = &vertexInputCreateInfo,
		.pInputAssemblyState = &inputAssemblyCreateInfo,
		.pViewportState = &viewportStateCreateInfo,
		.pRasterizationState = &rasterizerCreateInfo,
		.pMultisampleState = &multisamplingCreateInfo,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlendCreateInfo,
		.pDynamicState = &dynamicStateCreateInfo,
		//pipeline layout
		.layout = pipelineLayout,
		//render pass
		.renderPass = renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	VkPipeline pipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::exception("Failed to create graphics pipeline");

	vulkan_destroy_shader_module(device, vsModule);
	vulkan_destroy_shader_module(device, fsModule);

	return pipeline;
}

void vulkan_destroy_pipeline(VkDevice device, VkPipeline pipeline)
{
	vkDestroyPipeline(device, pipeline, nullptr);

}




VkRenderPass vulkan_create_render_pass(VkDevice device)
{
	VkAttachmentDescription colorAttachment = {
		.format = swapchainImageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,		//load : what to do with the already existing image on the framebuffer
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,	//store : what to do with the newly rendered image on the framebuffer
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,	//colorAttachment is index 0
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef
	};

	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	};

	VkRenderPassCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};

	VkRenderPass renderPass;
	if (vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::exception("Failed to create render pass");
	return renderPass;
}

void vulkan_destroy_render_pass(VkDevice device, VkRenderPass renderPass)
{
	vkDestroyRenderPass(device, renderPass, nullptr);

}



std::vector<VkFramebuffer> vulkan_create_framebuffers(VkDevice device, std::vector<VkImageView> swapchainImageViews)
{
	std::vector<VkFramebuffer> framebuffers;
	framebuffers.resize(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainImageViews.size(); ++i)
	{
		VkFramebufferCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = &swapchainImageViews[i],
			.width = swapchainExtent.width,
			.height = swapchainExtent.height,
			.layers = 1
		};

		if (vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			throw std::exception("Failed to create framebuffer");
	}
	return framebuffers;
}

void vulkan_destroy_framebuffers(VkDevice device, std::vector<VkFramebuffer> framebuffers)
{
	for (VkFramebuffer& framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

VkCommandPool vulkan_create_command_pool(VkPhysicalDevice physicalDevice, VkDevice device)
{
	VkCommandPoolCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = vulkan_find_queue_family_operation(physicalDevicce, VK_QUEUE_GRAPHICS_BIT).value(),
	};

	VkCommandPool commandPool;
	if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::exception("Failed to create command pool");
	return commandPool;
}

void vulkan_destroy_command_pool(VkDevice device, VkCommandPool commandPool)
{
	vkDestroyCommandPool(device, commandPool, nullptr);

}



VkCommandBuffer vulkan_allocate_command_buffer(VkDevice device)
{
	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
		throw std::exception("Failed to allocate command buffers");
	return commandBuffer;
}



void vulkan_record_command_buffer(VkCommandBuffer cb, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};

	if (vkBeginCommandBuffer(cb, &commandBufferBeginInfo) != VK_SUCCESS)
		throw std::exception("Failed to begin recording command buffer");

	VkClearValue clearColor = {
		.color = { 0.2f, 0.2f, 0.2f, 1.f }
	};

	VkRenderPassBeginInfo renderPassBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPass,
		.framebuffer = swapchainFramebuffers[imageIndex],
		.renderArea = {
			.offset = { 0, 0 },
			.extent = swapchainExtent
			},
		.clearValueCount = 1,
		.pClearValues = &clearColor
	};

	vkCmdBeginRenderPass(cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkBuffer vbos[] = { vbo };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cb, 0, 1, vbos, offsets);

	VkViewport viewport = {
		.x = 0.f,
		.y = 0.f,
		.width = static_cast<float>(swapchainExtent.width),
		.height = static_cast<float>(swapchainExtent.height),
		.minDepth = 0.f,
		.maxDepth = 1.f
	};

	vkCmdSetViewport(cb, 0, 1, &viewport);

	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchainExtent
	};

	vkCmdSetScissor(cb, 0, 1, &scissor);

	vkCmdDraw(cb, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

	vkCmdEndRenderPass(cb);

	if (vkEndCommandBuffer(cb) != VK_SUCCESS)
		throw std::exception("Failed to record command buffer");
}

void draw_frame()
{
	vkWaitForFences(device, 1, &renderOnceFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &renderOnceFence);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, renderReadySemaphore, VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(commandBuffer, 0);
	recordCommandBuffer(commandBuffer, imageIndex);

	VkSemaphore waitSemaphores[] = { renderReadySemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { renderDoneSemaphore };
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores
	};

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderOnceFence) != VK_SUCCESS)
		throw std::exception("Failed to submit draw command buffer");

	VkSwapchainKHR swapchains[] = { swapchain };
	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &imageIndex,
		.pResults = nullptr
	};

	vkQueuePresentKHR(presentQueue, &presentInfo);
}


VkSemaphore vulkan_create_semaphore(VkDevice device)
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {
	.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkSemaphore semaphore;
	if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS)
		throw std::exception("Failed to create semaphore");
	return semaphore;
}

void vulkan_destroy_semaphore(VkDevice device, VkSemaphore semaphore)
{
	vkDestroySemaphore(device, semaphore, nullptr);

}

VkFence vulkan_create_fence(VkDevice device)
{
	VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VkFence fence;
	if (vkCreateFence(device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		throw std::exception("Failed to create fence");
	return fence;
}

void vulkan_destroy_fence(VkDevice device, VkFence fence)
{
	vkDestroyFence(device, fence, nullptr);

}

VkBuffer vulkan_create_buffer()
{
	// TODO : staging buffers for better performance (https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer)
	VkBufferCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = sizeof(Vertex) * vertices.size(),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	VkBuffer buffer;
	if (vkCreateBuffer(device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::exception("Failed to create buffer");
	return buffer;
}

void vulkan_destroy_buffer()
{
	vkDestroyBuffer(device, vbo, nullptr);

}

VkDeviceMemory vulkan_allocate_memory()
{
	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(device, vbo, &memReq);

	// VRAM heap
	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReq.size,
		.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};

	VkDeviceMemory memory;
	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		throw std::exception("Failed to allocate memory");
	return memory;

}

void vulkan_free_memory()
{
	vkFreeMemory(device, vboMemory, nullptr);

}

void vulkan_bind_memory_to_buffer()
{
	vkBindBufferMemory(device, vbo, vboMemory, 0);

}

void vulkan_copy_data_to_memory()
{


	// filling the VBO (bind and unbind CPU accessible memory)
	void* data;
	vkMapMemory(device, vboMemory, 0, createInfo.size, 0, &data);
	// TODO : flush memory
	memcpy(data, vertices.data(), (size_t)createInfo.size);
	// TODO : invalidate memory before reading in the pipeline
	vkUnmapMemory(device, vboMemory);
}

uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProp;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProp);

	for (uint32_t i = 0; i < memProp.memoryTypeCount; ++i)
	{
		bool rightType = typeFilter & (1 << i);
		bool rightFlag = (memProp.memoryTypes[i].propertyFlags & properties) == properties;
		if (rightType && rightFlag)
			return i;
	}

	throw std::exception("Failed to find suitable memory type");
}