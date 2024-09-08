#include "GfxModel.hpp"
#include <cstddef>
#include <cstring>
#include <vector>
#include <vulkan/vulkan_core.h>

GfxModel::GfxModel(GfxDevice &gfxDevice, const Builder &builder)
    : gfxDevice(gfxDevice) {
  createVertexBuffers(builder.vertices);
  createIndexBuffer(builder.indices);
}

GfxModel::~GfxModel() {
  vkDestroyBuffer(gfxDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(gfxDevice.device(), vertexBufferMemory, nullptr);

  if (hasIndexBuffer) {
    vkDestroyBuffer(gfxDevice.device(), indexBuffer, nullptr);
    vkFreeMemory(gfxDevice.device(), indexBufferMemory, nullptr);
  }
}

void GfxModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  gfxDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(gfxDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(gfxDevice.device(), stagingBufferMemory);

  gfxDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

  gfxDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  vkDestroyBuffer(gfxDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(gfxDevice.device(), stagingBufferMemory, nullptr);
}

void GfxModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  }
}

void GfxModel::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    return;
  }
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

void GfxModel::createIndexBuffer(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  if (indexCount == 0) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
  hasIndexBuffer = true;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  gfxDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(gfxDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(gfxDevice.device(), stagingBufferMemory);

  gfxDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

  gfxDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
  vkDestroyBuffer(gfxDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(gfxDevice.device(), stagingBufferMemory, nullptr);
}