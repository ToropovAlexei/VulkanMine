#pragma once

#include "GfxDevice.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class GfxSwapChain : NonCopyable {
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  GfxSwapChain(GfxDevice &deviceRef, VkExtent2D windowExtent);
  GfxSwapChain(GfxDevice &deviceRef, VkExtent2D windowExtent,
               std::shared_ptr<GfxSwapChain> previousSwapChain);
  ~GfxSwapChain();

  VkFramebuffer getFrameBuffer(int index) {
    return m_swapChainFramebuffers[index];
  }
  VkRenderPass getRenderPass() { return m_renderPass; }
  VkImageView getImageView(int index) { return m_swapChainImageViews[index]; }
  size_t imageCount() { return m_swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return m_swapChainExtent; }
  uint32_t width() { return m_swapChainExtent.width; }
  uint32_t height() { return m_swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(m_swapChainExtent.width) /
           static_cast<float>(m_swapChainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers,
                                uint32_t *imageIndex);

  bool compareSwapFormats(const GfxSwapChain &other) const {
    return other.m_swapChainDepthFormat == m_swapChainDepthFormat &&
           other.m_swapChainImageFormat == m_swapChainImageFormat;
  }

private:
  void init();
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat m_swapChainImageFormat;
  VkFormat m_swapChainDepthFormat;
  VkExtent2D m_swapChainExtent;

  std::vector<VkFramebuffer> m_swapChainFramebuffers;
  VkRenderPass m_renderPass;

  std::vector<VkImage> m_depthImages;
  std::vector<VkDeviceMemory> m_depthImageMemorys;
  std::vector<VkImageView> m_depthImageViews;
  std::vector<VkImage> m_swapChainImages;
  std::vector<VkImageView> m_swapChainImageViews;

  GfxDevice &m_device;
  VkExtent2D m_windowExtent;

  VkSwapchainKHR m_swapChain;
  std::shared_ptr<GfxSwapChain> m_oldSwapChain;

  std::vector<VkSemaphore> m_imageAvailableSemaphores;
  std::vector<VkSemaphore> m_renderFinishedSemaphores;
  std::vector<VkFence> m_inFlightFences;
  std::vector<VkFence> m_imagesInFlight;
  size_t m_currentFrame = 0;
};
