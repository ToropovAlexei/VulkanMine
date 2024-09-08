#include "SimpleRenderSystem.hpp"
#include "glm/common.hpp"
#include "glm/gtc/constants.hpp"

struct PushConstantData {
  glm::mat4 transform{1.0f};
  alignas(16) glm::vec3 color;
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

void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,
                                           std::vector<GameObject> &gameObjects,
                                           const Camera &camera) {
  gfxPipeline->bind(commandBuffer);

  for (auto &gameObject : gameObjects) {
    gameObject.transform.rotation.y = glm::mod(
        gameObject.transform.rotation.y + 0.0001f, glm::two_pi<float>());
    gameObject.transform.rotation.x = glm::mod(
        gameObject.transform.rotation.x + 0.00005f, glm::two_pi<float>());
    PushConstantData push{
        .transform = camera.getProjection() * gameObject.transform.mat4(),
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