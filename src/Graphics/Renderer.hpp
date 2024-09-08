#pragma once

#include "../utils/NonCopyable.hpp"
#include "GfxDevice.hpp"
#include "GfxSwapChain.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>

class Renderer : NonCopyable {
public:
  Renderer(Window &window, GfxDevice &gfxDevice);
  ~Renderer();

  VkRenderPass getSwapChainRenderPass() const {
    return gfxSwapChain->getRenderPass();
  }
  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() {
    assert(isFrameStarted &&
           "Cannog get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted &&
           "Cannog get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

private:
  Window &window;
  GfxDevice &gfxDevice;
  std::unique_ptr<GfxSwapChain> gfxSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex = 0;
  int currentFrameIndex = 0;
  bool isFrameStarted = false;
};