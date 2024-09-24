#include "GfxModel.hpp"
#include "../utils/hash.hpp"
#include "GfxBuffer.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace std {
template <> struct hash<GfxModel::Vertex> {
  size_t operator()(GfxModel::Vertex const &vertex) const {
    size_t seed = 0;
    hashCombine(seed, vertex.pos, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};
} // namespace std

GfxModel::GfxModel(GfxDevice &gfxDevice, const Builder &builder)
    : gfxDevice(gfxDevice) {
  createVertexBuffers(builder.vertices);
  createIndexBuffer(builder.indices);
}

void GfxModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  uint32_t vertexSize = sizeof(vertices[0]);

  GfxBuffer stagingBuffer{gfxDevice, vertexSize, vertexCount,
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<GfxBuffer>(
      gfxDevice, vertexSize, vertexCount,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  gfxDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(),
                       bufferSize);
}

void GfxModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0,
                         VK_INDEX_TYPE_UINT32);
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
          {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
          {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
          {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)}};
}

void GfxModel::createIndexBuffer(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  if (indexCount == 0) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
  uint32_t indexSize = sizeof(indices[0]);
  hasIndexBuffer = true;

  GfxBuffer stagingBuffer{gfxDevice, indexSize, indexCount,
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  indexBuffer = std::make_unique<GfxBuffer>(
      gfxDevice, indexSize, indexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  gfxDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(),
                       bufferSize);
}