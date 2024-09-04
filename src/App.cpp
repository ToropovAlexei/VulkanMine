#include "App.h"
#include "Graphics/GfxModel.hpp"
#include "Graphics/GfxPipeline.h"
#include <array>
#include <vulkan/vulkan_core.h>

App::App() {
  loadModels();
  createPipelineLayout();
  recreateSwapChain();
  createCommandBuffers();
}

App::~App() {
  vkDestroyPipelineLayout(gfxDevice.device(), pipelineLayout, nullptr);
}

void App::run() {
  while (!window.shouldClose()) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(gfxDevice.device());
}

void App::loadModels() {
  std::vector<GfxModel::Vertex> vertices = {
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  gfxModel = std::make_unique<GfxModel>(gfxDevice, vertices);
}

void App::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(gfxDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void App::createPipeline() {
  GfxPipelineConfigInfo pipelineConfig{};
  GfxPipeline::defaultGfxPipelineConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = gfxSwapChain->getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  gfxPipeline = std::make_unique<GfxPipeline>(
      gfxDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipelineConfig);
}

void App::recreateSwapChain() {
  auto extent = window.getExtent();

  while (extent.width == 0 || extent.height == 0) {
    extent = window.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(gfxDevice.device());

  if (gfxSwapChain == nullptr) {
    gfxSwapChain = std::make_unique<GfxSwapChain>(gfxDevice, extent);
  } else {
    gfxSwapChain = std::make_unique<GfxSwapChain>(gfxDevice, extent,
                                                  std::move(gfxSwapChain));

    if (gfxSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  createPipeline();
}

void App::createCommandBuffers() {
  commandBuffers.resize(gfxSwapChain->imageCount());

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

void App::freeCommandBuffers() {
  vkFreeCommandBuffers(gfxDevice.device(), gfxDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

void App::recordCommandBuffer(int imageIndex) {

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = gfxSwapChain->getRenderPass(),
      .framebuffer = gfxSwapChain->getFrameBuffer(imageIndex),
      .renderArea =
          {
              .offset = {0, 0},
              .extent = gfxSwapChain->getSwapChainExtent(),
          },
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data(),
  };

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(gfxSwapChain->getSwapChainExtent().width),
      .height = static_cast<float>(gfxSwapChain->getSwapChainExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  VkRect2D scissor{{0, 0}, gfxSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  gfxPipeline->bind(commandBuffers[imageIndex]);
  gfxModel->bind(commandBuffers[imageIndex]);
  gfxModel->draw(commandBuffers[imageIndex]);

  vkCmdEndRenderPass(commandBuffers[imageIndex]);

  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void App::drawFrame() {
  uint32_t imageIndex;
  VkResult result = gfxSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  recordCommandBuffer(imageIndex);
  result = gfxSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],
                                              &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      window.wasWindowResized()) {
    window.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}