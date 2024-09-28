#pragma once

#include "../../core/NonCopyable.hpp"
#include "../../core/Window.hpp"
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_handles.hpp>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  bool graphicsFamilySupportsTimeStamps;

  std::optional<uint32_t> transferFamily;
  bool transferFamilySupportsTimeStamps;

  std::optional<uint32_t> presentFamily;
  bool presentFamilySupportsTimeStamps;

  bool IsComplete() {
    return graphicsFamily.has_value() && transferFamily.has_value() &&
           presentFamily.has_value();
  }
};

class RenderDeviceVk : NonCopyable {
public:
  RenderDeviceVk(Window *window);
  ~RenderDeviceVk();

  QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice device);

  vk::Device &getDevice() noexcept { return m_device; };
  vk::PhysicalDevice &getPhysicalDevice() noexcept { return m_physicalDevice; };
  vk::SurfaceKHR &getSurface() noexcept { return m_surface; };
  vk::Queue &getGraphicsQueue() noexcept { return m_graphicsQueue; };
  vk::Queue &getTransferQueue() noexcept { return m_transferQueue; };
  vk::Queue &getPresentQueue() noexcept { return m_presentQueue; };
  VmaAllocator &getAllocator() noexcept { return m_allocator; };

public:
  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

private:
  void initVulkan();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createAllocator();
  void createCommandPool();

  void checkValidationLayerSupport();
  bool
  checkInstanceExtensionSupport(std::vector<const char *> &requiredExtensions);
  void populateDebugMessengerCreateInfo(
      vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
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
  vk::DispatchLoaderDynamic dldi;
};