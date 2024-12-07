#pragma once

#include "../../core/NonCopyable.hpp"
#include "../../core/Window.hpp"
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_handles.hpp>

struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  bool graphicsFamilySupportsTimeStamps;

  std::optional<uint32_t> transferFamily;
  bool transferFamilySupportsTimeStamps;

  std::optional<uint32_t> presentFamily;
  bool presentFamilySupportsTimeStamps;

  bool IsComplete() { return graphicsFamily.has_value() && transferFamily.has_value() && presentFamily.has_value(); }
};

class RenderDeviceVk : NonCopyable {
public:
  RenderDeviceVk(Window *window);
  ~RenderDeviceVk();

  QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice device);
  inline QueueFamilyIndices findQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
  SwapChainSupportDetails getSwapChainSupport();

  inline vk::Device &getDevice() noexcept { return m_device; };
  inline vk::PhysicalDevice &getPhysicalDevice() noexcept { return m_physicalDevice; };
  inline vk::SurfaceKHR &getSurface() noexcept { return m_surface; };
  inline vk::Queue &getGraphicsQueue() noexcept { return m_graphicsQueue; };
  inline vk::Queue &getTransferQueue() noexcept { return m_transferQueue; };
  inline vk::Queue &getPresentQueue() noexcept { return m_presentQueue; };
  inline VmaAllocator &getAllocator() noexcept { return m_allocator; };
  inline vk::CommandPool &getCommandPool() noexcept { return m_commandPool; };
  inline vk::Instance &getInstance() noexcept { return m_instance; };

  void createImageWithInfo(const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage, vk::Image &image,
                           VmaAllocation &imageAllocation);
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);
  vk::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                    VmaAllocationCreateFlags flags, vk::Buffer &buffer, VmaAllocation &allocation,
                    VmaAllocationInfo &allocInfo);
  void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

private:
  void initVulkan();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createAllocator();
  void createCommandPool();

  void checkValidationLayerSupport();
  bool checkInstanceExtensionSupport(std::vector<const char *> &requiredExtensions);
  void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
  std::vector<const char *> getRequiredExtensions();
  int rateDeviceSuitability(const vk::PhysicalDevice &device);
  bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);

private:
  Window *m_window;
  vk::SurfaceKHR m_surface;
  vk::Instance m_instance;
  vk::PhysicalDevice m_physicalDevice;
  vk::Device m_device;
  VmaAllocator m_allocator;

  vk::Queue m_graphicsQueue;
  vk::Queue m_transferQueue;
  vk::Queue m_presentQueue;

  vk::CommandPool m_commandPool;

  vk::DebugUtilsMessengerEXT m_debugMessenger;
  vk::detail::DispatchLoaderDynamic dldi;
};