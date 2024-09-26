#pragma once

#include "../../core/NonCopyable.hpp"
#include "../../core/Window.hpp"

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
  QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice device);

private:
  Window *m_window;
  vk::SurfaceKHR m_surface;
  vk::Instance m_instance;
  vk::PhysicalDevice m_physicalDevice;
  vk::Device m_device;

  vk::Queue m_graphicsQueue;
  vk::Queue m_transferQueue;
  vk::Queue m_presentQueue;

  vk::DebugUtilsMessengerEXT m_debugMessenger;
  vk::DispatchLoaderDynamic dldi;
};