#pragma once

#include <array>
#include <vulkan/vulkan.h>

#include "vertex.hpp"

namespace VertexDesc
{
inline VkVertexInputBindingDescription get_vertex_input_binding_description()
{
    // describe the buffer data
    VkVertexInputBindingDescription desc = {.binding = 0,
                                            .stride = sizeof(Vertex),
                                            // update every vertex (opposed to VK_VERTEX_INPUT_RATE_INSTANCE)
                                            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    return desc;
}
inline std::array<VkVertexInputAttributeDescription, 3> get_vertex_input_attribute_description()
{
    // attribute pointer
    std::array<VkVertexInputAttributeDescription, 3> desc;
    desc[0] = {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)};
    desc[1] = {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, color)};
    desc[2] = {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv)};
    return desc;
}
} // namespace VertexDesc
