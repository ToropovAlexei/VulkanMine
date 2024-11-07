#pragma once

#include "backend/BufferVk.hpp"
#include "backend/RenderDeviceVk.hpp"
#include <memory>
#include <tracy/Tracy.hpp>

template <typename T> class Mesh {
public:
  Mesh(RenderDeviceVk *device, const std::vector<T> &vertices, const std::vector<uint32_t> &indices)
      : m_device{device} {
    ZoneScoped;
    createVertexBuffers(vertices);
    createIndexBuffer(indices);
  };

  inline void bind(vk::CommandBuffer commandBuffer) {
    ZoneScoped;
    vk::Buffer buffers[] = {m_vertexBuffer->getBuffer()};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);
    commandBuffer.bindIndexBuffer(m_indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
  };
  inline void draw(vk::CommandBuffer commandBuffer) {
    ZoneScoped;
    commandBuffer.drawIndexed(m_indexCount, 1, 0, 0, 0);
  };

  inline uint32_t getVertexCount() const noexcept { return m_vertexCount; }
  inline uint32_t getIndexCount() const noexcept { return m_indexCount; }

private:
  void createVertexBuffers(const std::vector<T> &vertices) {
    ZoneScoped;
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    vk::DeviceSize bufferSize = sizeof(T) * m_vertexCount;
    uint32_t vertexSize = sizeof(T);

    BufferVk stagingBuffer = {m_device,
                              vertexSize,
                              m_vertexCount,
                              vk::BufferUsageFlagBits::eTransferSrc,
                              VMA_MEMORY_USAGE_AUTO,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT};

    stagingBuffer.writeToBuffer((void *)vertices.data(), bufferSize);

    m_vertexBuffer = std::make_unique<BufferVk>(
        m_device, vertexSize, m_vertexCount,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_AUTO);

    m_device->copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
  };
  void createIndexBuffer(const std::vector<uint32_t> &indices) {
    ZoneScoped;
    m_indexCount = static_cast<uint32_t>(indices.size());
    vk::DeviceSize bufferSize = sizeof(uint32_t) * m_indexCount;
    uint32_t indexSize = sizeof(uint32_t);

    BufferVk stagingBuffer = {m_device,
                              indexSize,
                              m_indexCount,
                              vk::BufferUsageFlagBits::eTransferSrc,
                              VMA_MEMORY_USAGE_AUTO,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT};

    stagingBuffer.writeToBuffer((void *)indices.data(), bufferSize);

    m_indexBuffer = std::make_unique<BufferVk>(
        m_device, indexSize, m_indexCount,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_AUTO);

    m_device->copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
  };

private:
  RenderDeviceVk *m_device;
  std::unique_ptr<BufferVk> m_vertexBuffer;
  uint32_t m_vertexCount;

  std::unique_ptr<BufferVk> m_indexBuffer;
  uint32_t m_indexCount;
};