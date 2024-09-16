#include "SimpleRenderSystem.hpp"

struct PushConstantData {
  glm::mat4 transform{1.0f};
  glm::mat4 model{1.0f};
};

SimpleRenderSystem::SimpleRenderSystem(GfxDevice &gfxDevice,
                                       VkRenderPass renderPass)
    : gfxDevice{gfxDevice} {
  createPipelineLayout();
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(gfxDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::renderGameObjects(
    FrameInfo &frameInfo, std::vector<GameObject> &gameObjects) {
  gfxPipeline->bind(frameInfo.commandBuffer);

  auto projectionView =
      frameInfo.camera.getProjection() * frameInfo.camera.getView();

  for (auto &gameObject : gameObjects) {
    auto modelMatrix = gameObject.transform.mat4();
    PushConstantData push{
        .transform = projectionView * modelMatrix,
        .model = modelMatrix,
    };

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(PushConstantData), &push);
    gameObject.model->bind(frameInfo.commandBuffer);
    gameObject.model->draw(frameInfo.commandBuffer);
  }
}

void SimpleRenderSystem::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(PushConstantData),
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  if (vkCreatePipelineLayout(gfxDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  GfxPipelineConfigInfo pipelineConfig{};
  GfxPipeline::defaultGfxPipelineConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  gfxPipeline = std::make_unique<GfxPipeline>(
      gfxDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipelineConfig);
}