#include "wsi.hpp"

#include "vulkan_minimal.hpp"

int main()
{
    // TODO : descriptor sets (mvp)
    // TODO : textures
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
    VkInstance instance = RHI::Instance::create_instance(layers, instanceExtensions);

    VkDebugUtilsMessengerEXT debugMessenger = RHI::Instance::Debug::create_debug_messenger(instance);
    VkDebugReportCallbackEXT debugReport = RHI::Instance::Debug::create_debug_report_callback(instance);

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

    std::vector<VkSurfaceFormatKHR> formats =
        RHI::Presentation::Surface::get_surface_available_formats(physicalDevice, surface);
    std::optional<VkSurfaceFormatKHR> surfaceFormat = RHI::Presentation::Surface::find_surface_format(
        formats, VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    VkSwapchainKHR swapchain =
        RHI::Presentation::SwapChain::create_swap_chain(physicalDevice, device, surface, surfaceFormat.value(),
                                                        static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    std::vector<VkImage> swapchainImages = RHI::Presentation::SwapChain::get_swap_chain_images(device, swapchain);
    std::vector<VkImageView> swapchainImageViews = RHI::Presentation::SwapChain::create_swap_chain_image_views(
        device, swapchain, swapchainImages, surfaceFormat->format);

    VkRenderPass renderPass = RHI::RenderPass::create_render_pass(device, surfaceFormat->format);

    VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    std::vector<VkFramebuffer> framebuffers =
        RHI::RenderPass::create_framebuffers(device, renderPass, swapchainImageViews, extent);

    VkPipeline pipeline = RHI::Pipeline::create_pipeline(device, renderPass, "triangle", extent);

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

    const std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}},
                                          {{0.5f, -0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}},
                                          {{0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}},
                                          {{-0.5f, 0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}}};
    size_t vertexBufferSize = sizeof(Vertex) * vertices.size();
    auto vertexBuffer = RHI::Memory::create_optimal_buffer_from_data(
        device, physicalDevice, vertexBufferSize, vertices.data(), commandPoolTransient, graphicsQueue);

    // index buffer

    const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
    size_t indexBufferSize = sizeof(uint16_t) * indices.size();
    auto indexBuffer = RHI::Memory::create_optimal_buffer_from_data(
        device, physicalDevice, indexBufferSize, indices.data(), commandPoolTransient, graphicsQueue);

    uint32_t backBufferIndex = 0;
    while (!WSI::should_close(window))
    {
        WSI::poll_events();

        uint32_t imageIndex = RHI::Render::acquire_back_buffer(device, swapchain, acquireSemaphores[backBufferIndex],
                                                               inFlightFences, backBufferIndex);

        RHI::Render::record_back_buffer_pipeline_commands(commandBuffers[backBufferIndex], renderPass,
                                                          framebuffers[imageIndex], extent, pipeline);
        RHI::Render::record_back_buffer_draw_indexed_object_commands(
            commandBuffers[backBufferIndex], vertexBuffer.first, indexBuffer.first, indices.size());
        RHI::Render::record_back_buffer_end(commandBuffers[backBufferIndex]);

        RHI::Render::submit_back_buffer(graphicsQueue, commandBuffers[backBufferIndex],
                                        acquireSemaphores[backBufferIndex], renderSemaphores[backBufferIndex],
                                        inFlightFences[backBufferIndex]);

        RHI::Render::present_back_buffer(presentQueue, swapchain, imageIndex, renderSemaphores[backBufferIndex]);

        WSI::swap_buffers(window);
        backBufferIndex = (backBufferIndex + 1) % bufferingType;
    }

    vkDeviceWaitIdle(device);

    RHI::Memory::free_memory(device, vertexBuffer.second);
    RHI::Memory::destroy_buffer(device, vertexBuffer.first);

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

    RHI::RenderPass::destroy_framebuffers(device, framebuffers);

    RHI::RenderPass::destroy_render_pass(device, renderPass);

    RHI::Presentation::SwapChain::destroy_swap_chain_image_views(device, swapchainImageViews);
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
