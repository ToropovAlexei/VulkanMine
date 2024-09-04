#pragma once

#include "GfxDevice.hpp"
#include <vector>
#include <glm/glm.hpp>

class GfxModel : NonCopyable {
public:
  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();
  };

  GfxModel(GfxDevice &gfxDevice, const std::vector<Vertex> &vertices);
  ~GfxModel();

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);

private:
  GfxDevice &gfxDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;
};