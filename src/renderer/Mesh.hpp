#pragma once

#include "backend/BufferVk.hpp"
#include "backend/RenderDeviceVk.hpp"
#include <memory>

template <typename T> class Mesh {
public:
  Mesh(RenderDeviceVk *device, const std::vector<T> &vertices,
       const std::vector<uint32_t> &indices)
      : m_device{device} {
    createVertexBuffers(vertices);
    createIndexBuffer(indices);
  };

  void bind(vk::CommandBuffer commandBuffer) {
    vk::Buffer buffers[] = {m_vertexBuffer->getBuffer()};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);
    commandBuffer.bindIndexBuffer(m_indexBuffer->getBuffer(), 0,
                                  vk::IndexType::eUint32);
  };
  void draw(vk::CommandBuffer commandBuffer) {
    commandBuffer.drawIndexed(m_indexCount, 1, 0, 0, 0);
  };

private:
  void createVertexBuffers(const std::vector<T> &vertices) {
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    uint32_t vertexSize = sizeof(vertices[0]);

    BufferVk stagingBuffer = {m_device, vertexSize, m_vertexCount,
                              vk::BufferUsageFlagBits::eTransferSrc,
                              VMA_MEMORY_USAGE_CPU_ONLY};

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data(), bufferSize);

    m_vertexBuffer =
        std::make_unique<BufferVk>(m_device, vertexSize, m_vertexCount,
                                   vk::BufferUsageFlagBits::eVertexBuffer |
                                       vk::BufferUsageFlagBits::eTransferDst,
                                   VMA_MEMORY_USAGE_GPU_ONLY);

    m_device->copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(),
                         bufferSize);
  };
  void createIndexBuffer(const std::vector<uint32_t> &indices) {
    m_indexCount = static_cast<uint32_t>(indices.size());
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    uint32_t indexSize = sizeof(indices[0]);

    BufferVk stagingBuffer = {m_device, indexSize, m_indexCount,
                              vk::BufferUsageFlagBits::eTransferSrc,
                              VMA_MEMORY_USAGE_CPU_ONLY};

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data(), bufferSize);
    stagingBuffer.unmap();

    m_indexBuffer =
        std::make_unique<BufferVk>(m_device, indexSize, m_indexCount,
                                   vk::BufferUsageFlagBits::eIndexBuffer |
                                       vk::BufferUsageFlagBits::eTransferDst,
                                   VMA_MEMORY_USAGE_GPU_ONLY);

    m_device->copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(),
                         bufferSize);
  };

private:
  RenderDeviceVk *m_device;
  std::unique_ptr<BufferVk> m_vertexBuffer;
  uint32_t m_vertexCount;

  std::unique_ptr<BufferVk> m_indexBuffer;
  uint32_t m_indexCount;
};