#pragma once

#include "RenderDeviceVk.hpp"

class BufferVk {
public:
  BufferVk(RenderDeviceVk *device, vk::DeviceSize instanceSize, uint32_t instanceCount, vk::BufferUsageFlags usageFlags,
           VmaMemoryUsage memoryUsage, vk::DeviceSize minOffsetAlignment = 1);
  ~BufferVk();

  BufferVk(const BufferVk &) = delete;
  BufferVk &operator=(const BufferVk &) = delete;

  void map();
  void unmap();

  void writeToBuffer(void *data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

  void writeToIndex(void *data, vk::DeviceSize index);
  vk::Result flushIndex(vk::DeviceSize index);
  vk::DescriptorBufferInfo descriptorInfoForIndex(vk::DeviceSize index);
  void invalidateIndex(vk::DeviceSize index);

  inline vk::Buffer getBuffer() const noexcept { return m_buffer; }
  inline void *getMappedMemory() const noexcept { return m_mapped; }
  inline uint32_t getInstanceCount() const noexcept { return m_instanceCount; }
  inline vk::DeviceSize getInstanceSize() const noexcept { return m_instanceSize; }
  inline vk::DeviceSize getAlignmentSize() const noexcept { return m_instanceSize; }
  inline vk::BufferUsageFlags getUsageFlags() const noexcept { return m_usageFlags; }
  inline VmaMemoryUsage getMemoryPropertyFlags() const noexcept { return m_memoryUsage; }
  inline vk::DeviceSize getBufferSize() const noexcept { return m_bufferSize; }

private:
  static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

  RenderDeviceVk *m_device;
  void *m_mapped = nullptr;
  vk::Buffer m_buffer = VK_NULL_HANDLE;
  VmaAllocation m_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo m_allocationInfo{};

  vk::DeviceSize m_bufferSize;
  uint32_t m_instanceCount;
  vk::DeviceSize m_instanceSize;
  vk::DeviceSize m_alignmentSize;
  vk::BufferUsageFlags m_usageFlags;
  VmaMemoryUsage m_memoryUsage;
};
