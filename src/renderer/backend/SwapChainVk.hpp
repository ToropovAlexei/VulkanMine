#pragma once

#include "../../core/NonCopyable.hpp"
#include "RenderDeviceVk.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class SwapChainVk : NonCopyable {
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  SwapChainVk(RenderDeviceVk *device, vk::Extent2D windowExtent);
  SwapChainVk(RenderDeviceVk *device, vk::Extent2D windowExtent, std::shared_ptr<SwapChainVk> previousSwapChain);
  ~SwapChainVk();

  inline vk::Framebuffer getFrameBuffer(size_t index) noexcept { return m_swapChainFramebuffers[index]; }
  inline vk::RenderPass getRenderPass() noexcept { return m_renderPass; }
  inline vk::ImageView getImageView(size_t index) noexcept { return m_swapChainImageViews[index]; }
  inline size_t imageCount() noexcept { return m_swapChainImages.size(); }
  inline vk::Format getSwapChainImageFormat() noexcept { return m_swapChainImageFormat; }
  inline vk::Extent2D getSwapChainExtent() noexcept { return m_swapChainExtent; }
  inline uint32_t width() noexcept { return m_swapChainExtent.width; }
  inline uint32_t height() noexcept { return m_swapChainExtent.height; }

  inline float extentAspectRatio() noexcept { return m_aspectRatio; }
  vk::Format findDepthFormat();

  vk::Result acquireNextImage(uint32_t *imageIndex);
  vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex);

  inline bool compareSwapFormats(const SwapChainVk &other) const noexcept {
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

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  vk::Format m_swapChainImageFormat;
  vk::Format m_swapChainDepthFormat;
  vk::Extent2D m_swapChainExtent;
  float m_aspectRatio = 0.0f;

  std::vector<vk::Framebuffer> m_swapChainFramebuffers;
  vk::RenderPass m_renderPass;

  std::vector<vk::Image> m_depthImages;
  std::vector<VmaAllocation> m_depthImageMemorys;
  std::vector<vk::ImageView> m_depthImageViews;
  std::vector<vk::Image> m_swapChainImages;
  std::vector<vk::ImageView> m_swapChainImageViews;

  RenderDeviceVk *m_device;
  vk::Extent2D m_windowExtent;

  vk::SwapchainKHR m_swapChain;
  std::shared_ptr<SwapChainVk> m_oldSwapChain;

  std::vector<vk::Semaphore> m_imageAvailableSemaphores;
  std::vector<vk::Semaphore> m_renderFinishedSemaphores;
  std::vector<vk::Fence> m_inFlightFences;
  std::vector<vk::Fence> m_imagesInFlight;
  size_t m_currentFrame = 0;
};
