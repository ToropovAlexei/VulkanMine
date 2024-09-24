#include "ChunkRenderSystem.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

struct PushConstantData {
  glm::mat4 modelMatrix{1.0f};
};

ChunkRenderSystem::ChunkRenderSystem(GfxDevice &gfxDevice,
                                     VkRenderPass renderPass,
                                     VkDescriptorSetLayout globalSetLayout)
    : m_gfxDevice{gfxDevice} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

ChunkRenderSystem::~ChunkRenderSystem() {
  vkDestroyPipelineLayout(m_gfxDevice.device(), m_pipelineLayout, nullptr);
}

void ChunkRenderSystem::renderChunks(FrameInfo &frameInfo) {
  m_gfxPipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
                          1, &frameInfo.globalDescriptorSet, 0, nullptr);

  for (auto &gameObject : frameInfo.gameObjects) {
    PushConstantData push{
        .modelMatrix = gameObject.second.transform.mat4(),
    };

    vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(PushConstantData), &push);
    gameObject.second.model->bind(frameInfo.commandBuffer);
    gameObject.second.model->draw(frameInfo.commandBuffer);
  }
}

void ChunkRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(PushConstantData),
  };

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts = descriptorSetLayouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  if (vkCreatePipelineLayout(m_gfxDevice.device(), &pipelineLayoutInfo, nullptr,
                             &m_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void ChunkRenderSystem::createPipeline(VkRenderPass renderPass) {
  GfxPipelineConfigInfo pipelineConfig{};
  GfxPipeline::defaultGfxPipelineConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;
  m_gfxPipeline = std::make_unique<GfxPipeline>(
      m_gfxDevice, "shaders/chunk_shader.vert.spv",
      "shaders/chunk_shader.frag.spv", pipelineConfig);
}