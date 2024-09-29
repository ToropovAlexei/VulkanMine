#pragma once

#include "RenderDeviceVk.hpp"

class BufferVk {
public:
  BufferVk(RenderDeviceVk *device, vk::DeviceSize instanceSize,
           uint32_t instanceCount, vk::BufferUsageFlags usageFlags,
           VmaMemoryUsage memoryUsage, vk::DeviceSize minOffsetAlignment = 1);
  ~BufferVk();

  BufferVk(const BufferVk &) = delete;
  BufferVk &operator=(const BufferVk &) = delete;

  void map();
  void unmap();

  void writeToBuffer(void *data, vk::DeviceSize size = VK_WHOLE_SIZE,
                     vk::DeviceSize offset = 0);
  vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE,
                   vk::DeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE,
                                          vk::DeviceSize offset = 0);
  void invalidate(vk::DeviceSize size = VK_WHOLE_SIZE,
                  vk::DeviceSize offset = 0);

  void writeToIndex(void *data, vk::DeviceSize index);
  vk::Result flushIndex(vk::DeviceSize index);
  vk::DescriptorBufferInfo descriptorInfoForIndex(vk::DeviceSize index);
  void invalidateIndex(vk::DeviceSize index);

  vk::Buffer getBuffer() const { return m_buffer; }
  void *getMappedMemory() const { return m_mapped; }
  uint32_t getInstanceCount() const { return m_instanceCount; }
  vk::DeviceSize getInstanceSize() const { return m_instanceSize; }
  vk::DeviceSize getAlignmentSize() const { return m_instanceSize; }
  vk::BufferUsageFlags getUsageFlags() const { return m_usageFlags; }
  VmaMemoryUsage getMemoryPropertyFlags() const { return m_memoryUsage; }
  vk::DeviceSize getBufferSize() const { return m_bufferSize; }

private:
  static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize,
                                     vk::DeviceSize minOffsetAlignment);

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
