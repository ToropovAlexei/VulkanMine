#pragma once

#include "RenderDeviceVk.hpp"
#include <vulkan/vulkan_structs.hpp>

class SwapChainVk {
public:
  SwapChainVk(RenderDeviceVk *renderDevice);
  ~SwapChainVk();

  void init(vk::Extent2D extent);
  void cleanup();

  vk::Extent2D getExtent() const noexcept { return m_extent; };
  vk::Format getImageFormat() const noexcept { return m_imageFormat; }
  const std::vector<vk::ImageView> &getImageViews() const noexcept {
    return m_imageViews;
  }
  vk::SwapchainKHR getSwapChain() const noexcept { return m_swapChain; }

private:
  void createSwapChain(vk::Extent2D extent);
  void createImageViews();
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D extent);

private:
  RenderDeviceVk *m_renderDevice;
  vk::SwapchainKHR m_swapChain;
  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_imageViews;
  vk::Format m_imageFormat;
  vk::Extent2D m_extent;
};