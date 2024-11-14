#include "wsi.hpp"

#include "vulkan_tutorial.hpp"

// TODO : fix compilation

int main()
{
	// TODO : descriptor sets (mvp)
	// TODO : textures
	// TODO : better instance and device creation
	// TODO : better pipeline creation
	
	try
	{
		init_wsi();

		create_window();

		vulkanInit();
		vulkanExtensions();
		vulkanLayers();
		vulkanCreate();
#ifndef NDEBUG
		vulkanDebugMessenger();
#endif
		vulkanSurface(window);
		vulkanPhysicalDevice();
		vulkanLogicalDevice();
		vulkanSwapchain();
		vulkanImageViews();
		vulkanRenderPass();
		vulkanGraphicsPipeline();
		vulkanFramebuffers();
		
		vulkanCommandPool();
		vulkanVertexBuffer();
		vulkanCommandBuffer();

		vulkanMultithreadObjects();

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(device);

		vulkanDestroy();

		destroy_window(window);

		terminate_wsi();


	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
