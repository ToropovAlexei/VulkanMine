#include "App.h"
#include "Graphics/GfxPipeline.h"
#include <vulkan/vulkan_core.h>

App::App() {
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
  }
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

void App::createCommandBuffers() {}

void App::drawFrame() {}