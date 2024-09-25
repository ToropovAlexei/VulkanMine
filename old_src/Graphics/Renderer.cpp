#include "Renderer.hpp"
#include "GfxSwapChain.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>

Renderer::Renderer(Window &window, GfxDevice &gfxDevice)
    : gfxDevice{gfxDevice}, window{window} {
  recreateSwapChain();
  createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapChain() {
  auto extent = window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = window.getExtent();
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(gfxDevice.device());

  if (gfxSwapChain == nullptr) {
    gfxSwapChain = std::make_unique<GfxSwapChain>(gfxDevice, extent);
  } else {
    std::shared_ptr<GfxSwapChain> oldSwapChain = std::move(gfxSwapChain);
    gfxSwapChain =
        std::make_unique<GfxSwapChain>(gfxDevice, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*gfxSwapChain.get())) {
      throw std::runtime_error("Swap chain image or depth format has changed!");
    }
  }
}

void Renderer::createCommandBuffers() {
  commandBuffers.resize(GfxSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = gfxDevice.getCommandPool(),
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
  };

  if (vkAllocateCommandBuffers(gfxDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void Renderer::freeCommandBuffers() {
  vkFreeCommandBuffers(gfxDevice.device(), gfxDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = gfxSwapChain->acquireNextImage(&currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  return commandBuffer;
}

void Renderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame not in progress");

  auto commandBuffer = getCurrentCommandBuffer();
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  auto result =
      gfxSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      window.wasWindowResized()) {
    window.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  isFrameStarted = false;

  currentFrameIndex =
      (currentFrameIndex + 1) % GfxSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass "
                           "without first calling beginFrame");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't call beginSwapChainRenderPass on a different command buffer");

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = gfxSwapChain->getRenderPass(),
      .framebuffer = gfxSwapChain->getFrameBuffer(currentImageIndex),
      .renderArea =
          {
              .offset = {0, 0},
              .extent = gfxSwapChain->getSwapChainExtent(),
          },
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(gfxSwapChain->getSwapChainExtent().width),
      .height = static_cast<float>(gfxSwapChain->getSwapChainExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  VkRect2D scissor{{0, 0}, gfxSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call endSwapChainRenderPass "
                           "without first calling beginFrame");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't call endSwapChainRenderPass on a different command buffer");

  vkCmdEndRenderPass(commandBuffer);
}
