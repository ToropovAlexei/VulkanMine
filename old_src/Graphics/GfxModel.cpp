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
    hashCombine(seed, vertex.pos, vertex.color, vertex.uv);
    return seed;
  }
};
} // namespace std

GfxModel::GfxModel(GfxDevice &gfxDevice, const std::vector<Vertex> &vertices,
                   const std::vector<uint32_t> &indices)
    : m_gfxDevice(gfxDevice) {
  createVertexBuffers(vertices);
  createIndexBuffer(indices);
}

void GfxModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  m_vertexCount = static_cast<uint32_t>(vertices.size());
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  uint32_t vertexSize = sizeof(vertices[0]);

  GfxBuffer stagingBuffer{m_gfxDevice, vertexSize, m_vertexCount,
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  m_vertexBuffer = std::make_unique<GfxBuffer>(
      m_gfxDevice, vertexSize, m_vertexCount,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  m_gfxDevice.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(),
                         bufferSize);
}

void GfxModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0,
                       VK_INDEX_TYPE_UINT32);
}

void GfxModel::draw(VkCommandBuffer commandBuffer) {
  vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
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
          {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)}};
}

void GfxModel::createIndexBuffer(const std::vector<uint32_t> &indices) {
  m_indexCount = static_cast<uint32_t>(indices.size());
  if (m_indexCount == 0) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
  uint32_t indexSize = sizeof(indices[0]);

  GfxBuffer stagingBuffer{m_gfxDevice, indexSize, m_indexCount,
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  m_indexBuffer = std::make_unique<GfxBuffer>(
      m_gfxDevice, indexSize, m_indexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  m_gfxDevice.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(),
                         bufferSize);
}