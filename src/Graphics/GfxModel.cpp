#include "GfxModel.hpp"
#include <cstddef>
#include <cstring>
#include <vector>
#include <vulkan/vulkan_core.h>

GfxModel::GfxModel(GfxDevice &gfxDevice, const std::vector<Vertex> &vertices)
    : gfxDevice(gfxDevice) {
  createVertexBuffers(vertices);
}

GfxModel::~GfxModel() {
  vkDestroyBuffer(gfxDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(gfxDevice.device(), vertexBufferMemory, nullptr);
}

void GfxModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  gfxDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         vertexBuffer, vertexBufferMemory);

  void *data;
  vkMapMemory(gfxDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(gfxDevice.device(), vertexBufferMemory);
}

void GfxModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void GfxModel::draw(VkCommandBuffer commandBuffer) {
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription>
GfxModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
GfxModel::Vertex::getAttributeDescriptions() {
  return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
          {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}};
}