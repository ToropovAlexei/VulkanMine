#include "ChunkRenderSystem.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

ChunkRenderSystem::ChunkRenderSystem(
    RenderDeviceVk *device, vk::RenderPass renderPass,
    vk::DescriptorSetLayout descriptorSetLayout)
    : m_device{device} {
  createPipelineLayout(descriptorSetLayout);
  createPipeline(renderPass);
}

ChunkRenderSystem::~ChunkRenderSystem() {
  m_device->getDevice().destroyPipelineLayout(m_pipelineLayout);
}

void ChunkRenderSystem::render(FrameData &frameData) {
  m_pipeline->bind(frameData.commandBuffer);

  frameData.commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1,
      &frameData.globalDescriptorSet, 0, nullptr);

  for (auto &gameObject : frameData.gameObjects) {
    PushConstantData push = {
        .modelMatrix = gameObject->model,
    };
    frameData.commandBuffer.pushConstants(
        m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, sizeof(PushConstantData), &push);
    gameObject->mesh.bind(frameData.commandBuffer);
    gameObject->mesh.draw(frameData.commandBuffer);
  }
}

void ChunkRenderSystem::createPipelineLayout(
    vk::DescriptorSetLayout descriptorSetLayout) {
  vk::PushConstantRange pushConstantRange = {
      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      .offset = 0,
      .size = sizeof(PushConstantData),
  };

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{
      descriptorSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
      .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts = descriptorSetLayouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange,
  };

  m_pipelineLayout =
      m_device->getDevice().createPipelineLayout(pipelineLayoutInfo);
}

void ChunkRenderSystem::createPipeline(vk::RenderPass renderPass) {
  PipelineVkConfigInfo pipelineConfig = {};
  PipelineVk::defaultPipelineVkConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;
  pipelineConfig.vertexInputAttributeDescriptions =
      Vertex::getAttributeDescriptions();
  pipelineConfig.vertexInputBindingDescriptions =
      Vertex::getBindingDescriptions();
  m_pipeline = std::make_unique<PipelineVk>(
      m_device, "shaders/test_shader.vert.spv", "shaders/test_shader.frag.spv",
      pipelineConfig);
}