#pragma once

#include "GfxDevice.hpp"
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class GfxModel {
public:
  struct Vertex {
    glm::vec2 pos;

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();
  };

  GfxModel(GfxDevice &gfxDevice, const std::vector<Vertex> &vertices);
  ~GfxModel();

  GfxModel(const GfxModel &) = delete;
  GfxModel &operator=(const GfxModel &) = delete;
  GfxModel(GfxModel &&) = delete;
  GfxModel &operator=(GfxModel &&) = delete;

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