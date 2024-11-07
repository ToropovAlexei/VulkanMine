#include "BufferVk.hpp"

#include <cassert>
#include <cstring>

/**
 * Returns the minimum instance size required to be compatible with devices
 * minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the
 * offset member (eg minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
vk::DeviceSize BufferVk::getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}

BufferVk::BufferVk(RenderDeviceVk *device, vk::DeviceSize instanceSize, uint32_t instanceCount,
                   vk::BufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags,
                   vk::DeviceSize minOffsetAlignment)
    : m_device{device}, m_instanceCount{instanceCount}, m_instanceSize{instanceSize}, m_usageFlags{usageFlags},
      m_memoryUsage{memoryUsage} {
  m_alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
  m_bufferSize = m_alignmentSize * instanceCount;
  device->createBuffer(m_bufferSize, usageFlags, memoryUsage, flags, m_buffer, m_allocation, m_allocationInfo);
}

BufferVk::~BufferVk() {
  unmap();
  vmaDestroyBuffer(m_device->getAllocator(), static_cast<VkBuffer>(m_buffer), m_allocation);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the
 * specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to
 * map the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
void BufferVk::map() { vmaMapMemory(m_device->getAllocator(), m_allocation, &m_mapped); }

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void BufferVk::unmap() {
  if (m_mapped) {
    vmaUnmapMemory(m_device->getAllocator(), m_allocation);
    m_mapped = nullptr;
  }
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole
 * buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void BufferVk::writeToBuffer(void *data, vk::DeviceSize size, vk::DeviceSize offset) {
  assert(m_mapped && "Cannot copy to unmapped buffer");

  if (size == VK_WHOLE_SIZE) {
    memcpy(m_mapped, data, m_bufferSize);
  } else {
    char *memOffset = (char *)m_mapped;
    memOffset += offset;
    memcpy(memOffset, data, size);
  }
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE
 * to flush the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
vk::Result BufferVk::flush(vk::DeviceSize size, vk::DeviceSize offset) {
  if (m_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY) {
    return vk::Result::eSuccess; // GPU_ONLY memory doesn't need flush
  }

  vmaFlushAllocation(m_device->getAllocator(), m_allocation, offset, size);
  return vk::Result::eSuccess;
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass
 * VK_WHOLE_SIZE to invalidate the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
void BufferVk::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
  if (m_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY) {
    return; // GPU_ONLY memory doesn't need invalidation
  }

  vmaInvalidateAllocation(m_device->getAllocator(), m_allocation, offset, size);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
vk::DescriptorBufferInfo BufferVk::descriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
  return vk::DescriptorBufferInfo{
      m_buffer,
      offset,
      size,
  };
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of
 * index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void BufferVk::writeToIndex(void *data, vk::DeviceSize index) {
  writeToBuffer(data, m_instanceSize, index * m_alignmentSize);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it
 * visible to the device
 *
 * @param index Used in offset calculation
 *
 */
vk::Result BufferVk::flushIndex(vk::DeviceSize index) { return flush(m_alignmentSize, index * m_alignmentSize); }

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
vk::DescriptorBufferInfo BufferVk::descriptorInfoForIndex(vk::DeviceSize index) {
  return descriptorInfo(m_alignmentSize, index * m_alignmentSize);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
void BufferVk::invalidateIndex(vk::DeviceSize index) { return invalidate(m_alignmentSize, index * m_alignmentSize); }
