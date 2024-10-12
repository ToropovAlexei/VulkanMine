#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

struct ChunkVertex {
  glm::vec3 pos;
  glm::vec3 uvd;

  static std::vector<vk::VertexInputBindingDescription>
  getBindingDescriptions() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(ChunkVertex);
    bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

    return bindingDescriptions;
  }

  static std::vector<vk::VertexInputAttributeDescription>
  getAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(ChunkVertex, pos)},
        {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(ChunkVertex, uvd)},
    };
  }
};