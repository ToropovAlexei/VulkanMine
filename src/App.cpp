#include "App.h"
#include "Graphics/GfxModel.hpp"
#include "Graphics/GfxPipeline.h"
#include "glm/fwd.hpp"
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

struct PushConstantData {
  glm::mat2 transform{1.0f};
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

App::App() {
  loadGameObjects();
  createPipelineLayout();
  createPipeline();
}

App::~App() {
  vkDestroyPipelineLayout(gfxDevice.device(), pipelineLayout, nullptr);
}

void App::run() {
  while (!window.shouldClose()) {
    glfwPollEvents();
    if (auto commandBuffer = renderer.beginFrame()) {
      renderer.beginSwapChainRenderPass(commandBuffer);
      renderGameObjects(commandBuffer);
      renderer.endSwapChainRenderPass(commandBuffer);
      renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(gfxDevice.device());
}

void App::loadGameObjects() {
  std::vector<GfxModel::Vertex> vertices = {
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  auto gfxModel = std::make_shared<GfxModel>(gfxDevice, vertices);

  auto triangle = GameObject::createGameObject();
  triangle.model = gfxModel;
  triangle.color = {0.0f, 1.0f, 0.0f};
  triangle.transform2d.translation.x = 0.2f;
  triangle.transform2d.scale = {2.0f, 2.0f};
  triangle.transform2d.rotation = glm::half_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

void App::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(PushConstantData),
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(gfxDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void App::createPipeline() {
  GfxPipelineConfigInfo pipelineConfig{};
  GfxPipeline::defaultGfxPipelineConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = renderer.getSwapChainRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  gfxPipeline = std::make_unique<GfxPipeline>(
      gfxDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipelineConfig);
}

void App::renderGameObjects(VkCommandBuffer commandBuffer) {
  gfxPipeline->bind(commandBuffer);

  for (auto &gameObject : gameObjects) {
    PushConstantData push{
        .transform = gameObject.transform2d.mat2(),
        .offset = gameObject.transform2d.translation,
        .color = gameObject.color,
    };

    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(PushConstantData), &push);
    gameObject.model->bind(commandBuffer);
    gameObject.model->draw(commandBuffer);
  }
}