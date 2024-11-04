#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

struct ChunkVertex {
  uint32_t posAndTexUV;
  float texIdx;

  static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(ChunkVertex);
    bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

    return bindingDescriptions;
  }

  static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32Uint, offsetof(ChunkVertex, posAndTexUV)},
        {1, 0, vk::Format::eR32Sfloat, offsetof(ChunkVertex, texIdx)},
    };
  }
};