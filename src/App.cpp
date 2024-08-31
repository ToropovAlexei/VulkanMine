#include "App.h"
#include "Graphics/GfxModel.hpp"
#include "Graphics/GfxPipeline.h"
#include <array>
#include <vulkan/vulkan_core.h>

App::App() {
  loadModels();
  createPipelineLayout();
  createPipeline();
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
  auto pipelineConfig = GfxPipeline::defaultGfxPipelineConfigInfo(
      gfxSwapChain.width(), gfxSwapChain.height());

  pipelineConfig.renderPass = gfxSwapChain.getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  gfxPipeline = std::make_unique<GfxPipeline>(
      gfxDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipelineConfig);
}

void App::createCommandBuffers() {
  commandBuffers.resize(gfxSwapChain.imageCount());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = gfxDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(gfxDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  for (int i = 0; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = gfxSwapChain.getRenderPass();
    renderPassInfo.framebuffer = gfxSwapChain.getFrameBuffer(i);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = gfxSwapChain.getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    gfxPipeline->bind(commandBuffers[i]);
    gfxModel->bind(commandBuffers[i]);
    gfxModel->draw(commandBuffers[i]);

    vkCmdEndRenderPass(commandBuffers[i]);

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void App::drawFrame() {
  uint32_t imageIndex;
  VkResult result = gfxSwapChain.acquireNextImage(&imageIndex);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  result = gfxSwapChain.submitCommandBuffers(&commandBuffers[imageIndex],
                                             &imageIndex);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}