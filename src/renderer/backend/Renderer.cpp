#include "Renderer.hpp"
#include "SwapChainVk.hpp"
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.hpp>

Renderer::Renderer(Window *window, RenderDeviceVk *device)
    : m_device{device}, m_window{window} {
  ZoneScoped;
  recreateSwapChain();
  createCommandBuffers();
}

void Renderer::recreateSwapChain() {
  ZoneScoped;
  auto extent = m_window->getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = m_window->getExtent();
    glfwWaitEvents();
  }
  m_device->getDevice().waitIdle();

  if (m_swapChain == nullptr) {
    m_swapChain = std::make_unique<SwapChainVk>(m_device, extent);
  } else {
    std::shared_ptr<SwapChainVk> oldSwapChain = std::move(m_swapChain);
    m_swapChain = std::make_unique<SwapChainVk>(m_device, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*m_swapChain.get())) {
      throw std::runtime_error("Swap chain image or depth format has changed!");
    }
  }
}

void Renderer::createCommandBuffers() {
  ZoneScoped;
  m_commandBuffers.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  vk::CommandBufferAllocateInfo allocInfo = {
      .commandPool = m_device->getCommandPool(),
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size()),
  };

  m_commandBuffers = m_device->getDevice().allocateCommandBuffers(allocInfo);
}

void Renderer::freeCommandBuffers() {
  ZoneScoped;
  m_device->getDevice().freeCommandBuffers(m_device->getCommandPool(),
                                           m_commandBuffers);
  m_commandBuffers.clear();
}

vk::CommandBuffer Renderer::beginFrame() {
  ZoneScoped;
  assert(!m_isFrameStarted &&
         "Can't call beginFrame while already in progress");

  auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    // ImGui_ImplVulkanH_CreateOrResizeWindow(
    //     m_device->getInstance(), m_device->getPhysicalDevice(),
    //     m_device->getDevice(), &g_MainWindowData, g_QueueFamily, nullptr,
    //     fb_width, fb_height, g_MinImageCount);
    return nullptr;
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  m_isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  vk::CommandBufferBeginInfo beginInfo{};

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

void Renderer::endFrame() {
  ZoneScoped;
  assert(m_isFrameStarted && "Can't call endFrame while frame not in progress");

  auto commandBuffer = getCurrentCommandBuffer();
  commandBuffer.end();

  auto result =
      m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR ||
      result == vk::Result::eSuboptimalKHR || m_window->wasWindowResized()) {
    m_window->resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  m_isFrameStarted = false;

  m_currentFrameIndex =
      (m_currentFrameIndex + 1) % SwapChainVk::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  ZoneScoped;
  assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass "
                             "without first calling beginFrame");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't call beginSwapChainRenderPass on a different command buffer");

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0] = vk::ClearValue(
      vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
  clearValues[1] = vk::ClearValue({.depthStencil = {1.0f, 0}});

  vk::RenderPassBeginInfo renderPassInfo = {
      .renderPass = m_swapChain->getRenderPass(),
      .framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex),
      .renderArea =
          {
              .offset = {0, 0},
              .extent = m_swapChain->getSwapChainExtent(),
          },
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  vk::Viewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(m_swapChain->getSwapChainExtent().width),
      .height = static_cast<float>(m_swapChain->getSwapChainExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  vk::Rect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);
}

void Renderer::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  ZoneScoped;
  assert(m_isFrameStarted && "Can't call endSwapChainRenderPass "
                             "without first calling beginFrame");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't call endSwapChainRenderPass on a different command buffer");

  commandBuffer.endRenderPass();
}