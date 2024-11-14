#pragma once

#include <vector>

#include "vertex.hpp"


// layers
const std::vector<const char*> layers = {
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation",
#endif
    "VK_LAYER_LUNARG_monitor",
};


// device extensions
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


// creating a vertex buffer
const std::vector<Vertex> vertices = {
	{ {0.0f, -0.5f}, Color::red },
	{ {0.5f,  0.5f}, Color::green },
	{ {-0.5f, 0.5f}, Color::blue }
};