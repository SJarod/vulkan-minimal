#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "wsi.hpp"

#include "uniform_desc.hpp"
#include "vertex_desc.hpp"
#include "vulkan_minimal.hpp"

int main()
{
    // TODO : better pipeline creation

    WSI::init();

    int width = 1366, height = 768;

    GLFWwindow *window = WSI::create_window(width, height, "Vulkan Minimal");
    WSI::make_context_current(window);

    RHI::load_symbols();
    std::vector<std::string> availableLayers = RHI::Instance::enumerate_available_layers();
    RHI::Instance::enumerate_available_instance_extensions();

    std::vector<const char *> layers;
    if (std::find(availableLayers.begin(), availableLayers.end(), "VK_LAYER_KHRONOS_validation") !=
        availableLayers.end())
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    if (std::find(availableLayers.begin(), availableLayers.end(), "VK_LAYER_LUNARG_monitor") != availableLayers.end())
        layers.emplace_back("VK_LAYER_LUNARG_monitor");

    std::vector<const char *> instanceExtensions = WSI::get_required_extensions();
    instanceExtensions.push_back("VK_EXT_debug_utils");
    instanceExtensions.push_back("VK_EXT_debug_report");
    VkInstance instance = RHI::Instance::create_instance(layers, instanceExtensions, false);

    VkDebugUtilsMessengerEXT debugMessenger =
        RHI::Instance::Debug::create_debug_messenger(instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
    VkDebugReportCallbackEXT debugReport =
        RHI::Instance::Debug::create_debug_report_callback(instance, VK_DEBUG_REPORT_ERROR_BIT_EXT);

    std::vector<VkPhysicalDevice> physicalDevices = RHI::Device::get_physical_devices(instance);
    for (auto physicalDevice : physicalDevices)
        RHI::Device::enumerate_available_device_extensions(physicalDevice);

    VkSurfaceKHR surface =
        RHI::Presentation::Surface::create_surface(&WSI::create_presentation_surface, instance, window, nullptr);

    VkPhysicalDevice physicalDevice = physicalDevices[0];
    std::optional<uint32_t> graphicsFamilyIndex =
        RHI::Device::Queue::find_queue_family_index(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
    std::optional<uint32_t> presentFamilyIndex =
        RHI::Device::Queue::find_present_queue_family_index(physicalDevice, surface);
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDevice device = RHI::Device::create_logical_device(instance, physicalDevice, &surface, layers, deviceExtensions);
    VkQueue graphicsQueue = RHI::Device::Queue::get_device_queue(device, graphicsFamilyIndex.value(), 0);
    VkQueue presentQueue = RHI::Device::Queue::get_device_queue(device, presentFamilyIndex.value(), 0);

    std::optional<VkSurfaceFormatKHR> surfaceFormat = RHI::Presentation::Surface::find_adequate_surface_format(
        physicalDevice, surface, VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    VkSwapchainKHR swapchain =
        RHI::Presentation::SwapChain::create_swap_chain(physicalDevice, device, surface, surfaceFormat.value(),
                                                        static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    std::vector<VkImage> swapchainImages = RHI::Presentation::SwapChain::get_swap_chain_images(device, swapchain);
    std::vector<VkImageView> swapchainImageViews(swapchainImages.size());
    for (int i = 0; i < swapchainImageViews.size(); ++i)
    {
        swapchainImageViews[i] = RHI::Memory::Image::create_image_view(
            device, swapchainImages[i], surfaceFormat->format, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    uint32_t frameInFlightCount = static_cast<uint32_t>(swapchainImages.size());
    VkFormat depthImageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    auto swapchainDepthImage = RHI::Memory::Image::create_allocated_image(
        device, physicalDevice, width, height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkImageView swapchainDepthImageView = RHI::Memory::Image::create_image_view(
        device, swapchainDepthImage.first, depthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderPass renderPass = RHI::RenderPass::create_render_pass(device, surfaceFormat->format, depthImageFormat);

    VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    std::vector<VkFramebuffer> framebuffers =
        RHI::RenderPass::create_framebuffers(device, renderPass, swapchainImageViews, swapchainDepthImageView, extent);

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
        UniformDesc::get_uniform_descriptor_set_layout_bindings();
    std::vector<VkDescriptorSetLayout> setLayouts = {
        RHI::Pipeline::Shader::create_descriptor_set_layout(device, setLayoutBindings)};
    VkPipelineLayout pipelineLayout = RHI::Pipeline::Shader::create_pipeline_layout(device, setLayouts);
    VkPipeline pipeline = RHI::Pipeline::create_pipeline(device, renderPass, "triangle", extent, pipelineLayout);

    VkCommandPool commandPool = RHI::Command::create_command_pool(device, graphicsFamilyIndex.value());
    VkCommandPool commandPoolTransient = RHI::Command::create_command_pool(device, graphicsFamilyIndex.value(), true);

    uint32_t bufferingType = 2;
    std::vector<VkCommandBuffer> commandBuffers =
        RHI::Command::allocate_command_buffers(device, commandPool, bufferingType);

    std::vector<VkSemaphore> acquireSemaphores;
    std::vector<VkSemaphore> renderSemaphores;
    std::vector<VkFence> inFlightFences;
    for (int i = 0; i < bufferingType; ++i)
    {
        acquireSemaphores.emplace_back(RHI::Parallel::create_semaphore(device));
        renderSemaphores.emplace_back(RHI::Parallel::create_semaphore(device));
        inFlightFences.emplace_back(RHI::Parallel::create_fence(device));
    }

    // vertex buffer

    const std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f}},
                                          {{0.5f, -0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f}},
                                          {{0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}},
                                          {{-0.5f, 0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f}},
                                          {{-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f}},
                                          {{0.5f, -0.5f, -0.5f}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f}},
                                          {{0.5f, 0.5f, -0.5f}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}},
                                          {{-0.5f, 0.5f, -0.5f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f}}};
    size_t vertexBufferSize = sizeof(Vertex) * vertices.size();
    auto vertexBuffer = RHI::Memory::Buffer::create_optimal_buffer_from_data(
        device, physicalDevice, vertexBufferSize, vertices.data(), commandPoolTransient, graphicsQueue,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // index buffer

    const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
    size_t indexBufferSize = sizeof(uint16_t) * indices.size();
    auto indexBuffer = RHI::Memory::Buffer::create_optimal_buffer_from_data(
        device, physicalDevice, indexBufferSize, indices.data(), commandPoolTransient, graphicsQueue,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // uniform buffers

    std::vector<std::pair<VkBuffer, VkDeviceMemory>> uniformBuffers(frameInFlightCount);
    std::vector<void *> uniformBuffersMapped(frameInFlightCount);
    size_t uniformBufferSize = sizeof(UniformBufferObjectT);
    for (int i = 0; i < frameInFlightCount; ++i)
    {
        uniformBuffers[i] = RHI::Memory::Buffer::create_allocated_buffer(
            device, physicalDevice, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(device, uniformBuffers[i].second, 0, uniformBufferSize, 0, &uniformBuffersMapped[i]);
    }

    std::vector<VkDescriptorPoolSize> poolSizes = UniformDesc::get_uniform_descriptor_pool_sizes(frameInFlightCount);
    VkDescriptorPool descriptorPool =
        RHI::Pipeline::Shader::create_descriptor_pool(device, poolSizes, frameInFlightCount);
    std::vector<VkDescriptorSetLayout> uniformSetLayouts(frameInFlightCount, setLayouts[0]);
    std::vector<VkDescriptorSet> descriptorSets =
        RHI::Pipeline::Shader::allocate_desriptor_sets(device, descriptorPool, frameInFlightCount, uniformSetLayouts);

    const std::vector<unsigned char> imagePixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 0, 255, 255};
    auto texture = RHI::Memory::Image::create_image_texture_from_data(
        device, physicalDevice, 2, 2, imagePixels.data(), commandPoolTransient, graphicsQueue, VK_FORMAT_R8G8B8A8_SRGB);
    VkImageView textureView = RHI::Memory::Image::create_image_view(device, texture.first, VK_FORMAT_R8G8B8A8_SRGB,
                                                                    VK_IMAGE_ASPECT_COLOR_BIT);
    VkSampler sampler = RHI::Memory::Image::create_image_sampler(device, VK_FILTER_NEAREST);

    for (int i = 0; i < frameInFlightCount; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i].first,
            .offset = 0,
            .range = sizeof(UniformBufferObjectT),
        };
        VkDescriptorImageInfo imageInfo = {
            .sampler = sampler,
            .imageView = textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        std::vector<VkWriteDescriptorSet> writes =
            UniformDesc::get_uniform_descriptor_set_writes(descriptorSets[i], bufferInfo, imageInfo);
        RHI::Pipeline::Shader::write_descriptor_sets(device, writes);
    }

    uint32_t backBufferIndex = 0;
    while (!WSI::should_close(window))
    {
        WSI::poll_events();

        uint32_t imageIndex = RHI::Render::acquire_back_buffer(device, swapchain, acquireSemaphores[backBufferIndex],
                                                               inFlightFences[backBufferIndex]);

        UniformBufferObjectT ubo = {
            .model = glm::mat4(1.f),
            .view = glm::lookAt(glm::vec3(0.f, 1.f, 1.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.)),
            .proj = glm::perspective(glm::radians(45.f), extent.width / (float)extent.height, 0.1f, 1000.f),
        };
        memcpy(uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));

        RHI::Render::record_back_buffer_begin_render_pass(commandBuffers[backBufferIndex], renderPass,
                                                          framebuffers[imageIndex], extent, pipeline);
        RHI::Render::record_back_buffer_descriptor_sets_commands(commandBuffers[backBufferIndex], pipelineLayout,
                                                                 descriptorSets[imageIndex]);
        RHI::Render::record_back_buffer_draw_indexed_object_commands(
            commandBuffers[backBufferIndex], vertexBuffer.first, indexBuffer.first, indices.size());
        RHI::Render::record_back_buffer_end_render_pass(commandBuffers[backBufferIndex]);

        RHI::Render::submit_back_buffer(graphicsQueue, commandBuffers[backBufferIndex],
                                        acquireSemaphores[backBufferIndex], renderSemaphores[backBufferIndex],
                                        inFlightFences[backBufferIndex]);

        RHI::Render::present_back_buffer(presentQueue, swapchain, imageIndex, renderSemaphores[backBufferIndex]);

        WSI::swap_buffers(window);
        backBufferIndex = (backBufferIndex + 1) % bufferingType;
    }

    vkDeviceWaitIdle(device);

    RHI::Memory::Image::destroy_image_sampler(device, sampler);
    RHI::Memory::Image::destroy_image_view(device, textureView);
    RHI::Memory::free_memory(device, texture.second);
    RHI::Memory::Image::destroy_image(device, texture.first);

    RHI::Pipeline::Shader::destroy_descriptor_pool(device, descriptorPool);

    for (int i = 0; i < frameInFlightCount; ++i)
    {
        RHI::Memory::free_memory(device, uniformBuffers[i].second);
        RHI::Memory::Buffer::destroy_buffer(device, uniformBuffers[i].first);
    }

    RHI::Memory::free_memory(device, indexBuffer.second);
    RHI::Memory::Buffer::destroy_buffer(device, indexBuffer.first);

    RHI::Memory::free_memory(device, vertexBuffer.second);
    RHI::Memory::Buffer::destroy_buffer(device, vertexBuffer.first);

    for (int i = 0; i < bufferingType; ++i)
    {
        RHI::Parallel::destroy_fence(device, inFlightFences[i]);
        RHI::Parallel::destroy_semaphore(device, renderSemaphores[i]);
        RHI::Parallel::destroy_semaphore(device, acquireSemaphores[i]);
    }
    inFlightFences.clear();
    renderSemaphores.clear();
    acquireSemaphores.clear();

    RHI::Command::destroy_command_pool(device, commandPoolTransient);
    RHI::Command::destroy_command_pool(device, commandPool);

    RHI::Pipeline::destroy_pipeline(device, pipeline);
    RHI::Pipeline::Shader::destroy_pipeline_layout(device, pipelineLayout);
    for (VkDescriptorSetLayout setLayout : setLayouts)
    {
        RHI::Pipeline::Shader::destroy_descriptor_set_layout(device, setLayout);
    }

    RHI::RenderPass::destroy_framebuffers(device, framebuffers);

    RHI::RenderPass::destroy_render_pass(device, renderPass);

    RHI::Memory::Image::destroy_image_view(device, swapchainDepthImageView);
    RHI::Memory::free_memory(device, swapchainDepthImage.second);
    RHI::Memory::Image::destroy_image(device, swapchainDepthImage.first);

    for (int i = 0; i < swapchainImageViews.size(); ++i)
    {
        RHI::Memory::Image::destroy_image_view(device, swapchainImageViews[i]);
    }
    RHI::Presentation::SwapChain::destroy_swap_chain(device, swapchain);

    RHI::Device::destroy_logical_device(device);

    RHI::Presentation::Surface::destroy_surface(instance, surface);

    RHI::Instance::Debug::destroy_debug_report_callback(instance, debugReport);
    RHI::Instance::Debug::destroy_debug_messenger(instance, debugMessenger);

    RHI::Instance::destroy_instance(instance);

    WSI::destroy_window(window);

    WSI::terminate();

    return EXIT_SUCCESS;
}
