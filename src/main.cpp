#include "wsi.hpp"

#include "vulkan_minimal.hpp"

int main()
{
    // TODO : descriptor sets (mvp)
    // TODO : textures
    // TODO : better pipeline creation

    WSI::init();

    int width = 1366, height = 768;

    GLFWwindow *window = WSI::create_window(width, height);
    WSI::make_context_current(window);

    RHI::load_symbols();
    RHI::Instance::enumerate_available_layers();
    RHI::Instance::enumerate_available_instance_extensions();

    const std::vector<const char *> layers = {
        "VK_LAYER_KHRONOS_validation",
        // "VK_LAYER_LUNARG_monitor",
    };
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
        RHI::Presentation::SwapChain::create_swap_chain(physicalDevice, device, surface, surfaceFormat.value());
    std::vector<VkImage> swapchainImages = RHI::Presentation::SwapChain::get_swap_chain_images(device, swapchain);
    std::vector<VkImageView> swapchainImageViews = RHI::Presentation::SwapChain::create_swap_chain_image_views(
        device, swapchain, swapchainImages, surfaceFormat->format);

    VkRenderPass renderPass = RHI::RenderPass::create_render_pass(device, surfaceFormat->format);

    std::vector<VkFramebuffer> framebuffers = RHI::RenderPass::create_framebuffers(
        device, renderPass, swapchainImageViews, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});

    VkPipeline pipeline = RHI::Pipeline::create_pipeline(device, renderPass, "triangle",
                                                         {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});

    VkCommandPool commandPool = RHI::Command::create_command_pool(device, graphicsFamilyIndex.value());

    uint32_t bufferingType = 2;
    std::vector<VkCommandBuffer> commandBuffer =
        RHI::Command::allocate_command_buffers(device, commandPool, bufferingType);

    std::vector<VkSemaphore> drawSemaphore;
    std::vector<VkSemaphore> presentSemaphore;
    std::vector<VkFence> inFlightFence;
    for (int i = 0; i < bufferingType; ++i)
    {
        drawSemaphore.emplace_back(RHI::Parallel::create_semaphore(device));
        presentSemaphore.emplace_back(RHI::Parallel::create_semaphore(device));
        inFlightFence.emplace_back(RHI::Parallel::create_fence(device));
    }

    const std::vector<Vertex> vertices = {{{0.0f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}},
                                          {{0.5f, 0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}},
                                          {{-0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}}};
    VkBuffer vertexBuffer = RHI::Memory::create_buffer(device, sizeof(Vertex) * vertices.size());
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memReq);
    std::optional<uint32_t> memoryTypeIndex = RHI::Device::Memory::find_memory_type_index(
        physicalDevice, memReq, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory memory = RHI::Memory::allocate_memory(device, memReq.size, memoryTypeIndex.value());
    RHI::Memory::copy_data_to_memory(device, memory, vertices.data(), sizeof(Vertex) * vertices.size());
    RHI::Memory::bind_memory_to_buffer(device, vertexBuffer, memory);

    int currentImageIndex = 0;
    while (!WSI::should_close(window))
    {
        WSI::poll_events();

        vkWaitForFences(device, 1, &inFlightFence[currentImageIndex], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFence[currentImageIndex]);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, drawSemaphore[currentImageIndex], VK_NULL_HANDLE,
                              &imageIndex);

        vkResetCommandBuffer(commandBuffer[currentImageIndex], 0);

        // record command buffer

        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = 0, .pInheritanceInfo = nullptr};

        VkResult res = vkBeginCommandBuffer(commandBuffer[currentImageIndex], &commandBufferBeginInfo);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to begin recording command buffer : " << res << std::endl;

        VkClearValue clearColor = {.color = {0.2f, 0.2f, 0.2f, 1.f}};

        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderPass,
            .framebuffer = framebuffers[currentImageIndex],
            .renderArea = {.offset = {0, 0}, .extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}},
            .clearValueCount = 1,
            .pClearValues = &clearColor};

        vkCmdBeginRenderPass(commandBuffer[currentImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer[currentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkBuffer vbos[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer[currentImageIndex], 0, 1, vbos, offsets);

        VkViewport viewport = {.x = 0.f,
                               .y = 0.f,
                               .width = static_cast<float>(width),
                               .height = static_cast<float>(height),
                               .minDepth = 0.f,
                               .maxDepth = 1.f};

        vkCmdSetViewport(commandBuffer[currentImageIndex], 0, 1, &viewport);

        VkRect2D scissor = {.offset = {0, 0}, .extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};

        vkCmdSetScissor(commandBuffer[currentImageIndex], 0, 1, &scissor);

        vkCmdDraw(commandBuffer[currentImageIndex], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer[currentImageIndex]);

        res = vkEndCommandBuffer(commandBuffer[currentImageIndex]);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to record command buffer : " << res << std::endl;

        // draw

        VkSemaphore waitSemaphores[] = {drawSemaphore[currentImageIndex]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {presentSemaphore[currentImageIndex]};
        VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                   .waitSemaphoreCount = 1,
                                   .pWaitSemaphores = waitSemaphores,
                                   .pWaitDstStageMask = waitStages,
                                   .commandBufferCount = 1,
                                   .pCommandBuffers = &commandBuffer[currentImageIndex],
                                   .signalSemaphoreCount = 1,
                                   .pSignalSemaphores = signalSemaphores};

        res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence[currentImageIndex]);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to submit draw command buffer : " << res << std::endl;

        VkSwapchainKHR swapchains[] = {swapchain};
        VkPresentInfoKHR presentInfo = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                        .waitSemaphoreCount = 1,
                                        .pWaitSemaphores = signalSemaphores,
                                        .swapchainCount = 1,
                                        .pSwapchains = swapchains,
                                        .pImageIndices = &imageIndex,
                                        .pResults = nullptr};

        res = vkQueuePresentKHR(presentQueue, &presentInfo);
        if (res != VK_SUCCESS)
            std::cerr << "Failed to present : " << res << std::endl;

        WSI::swap_buffers(window);

        currentImageIndex = (currentImageIndex + 1) % bufferingType;
    }

    vkDeviceWaitIdle(device);

    RHI::Memory::free_memory(device, memory);
    RHI::Memory::destroy_buffer(device, vertexBuffer);

    for (int i = 0; i < bufferingType; ++i)
    {
        RHI::Parallel::destroy_fence(device, inFlightFence[i]);
        RHI::Parallel::destroy_semaphore(device, presentSemaphore[i]);
        RHI::Parallel::destroy_semaphore(device, drawSemaphore[i]);
    }
    inFlightFence.clear();
    presentSemaphore.clear();
    drawSemaphore.clear();

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
