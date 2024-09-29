#pragma once

#include "RenderDeviceVk.hpp"
#include "SwapChainVk.hpp"
#include <cstddef>
#include <memory>

class Renderer {
public:
  Renderer(Window *window, RenderDeviceVk *device);

  VkRenderPass getSwapChainRenderPass() const {
    return m_swapChain->getRenderPass();
  }
  size_t getFrameIndex() const {
    assert(m_isFrameStarted &&
           "Cannog get frame index when frame not in progress");
    return m_currentFrameIndex;
  }
  vk::CommandBuffer getCurrentCommandBuffer() {
    assert(m_isFrameStarted &&
           "Cannog get command buffer when frame not in progress");
    return m_commandBuffers[m_currentFrameIndex];
  }

  vk::CommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
  void endSwapChainRenderPass(vk::CommandBuffer commandBuffer);

private:
  void recreateSwapChain();
  void createCommandBuffers();
  void freeCommandBuffers();

private:
  RenderDeviceVk *m_device;
  Window *m_window;
  std::unique_ptr<SwapChainVk> m_swapChain;
  std::vector<vk::CommandBuffer> m_commandBuffers;

  uint32_t m_currentImageIndex = 0;
  size_t m_currentFrameIndex = 0;
  bool m_isFrameStarted = false;
};