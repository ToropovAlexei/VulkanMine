#include "ChunkRenderSystem.hpp"
#include "ChunkVertex.hpp"
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

ChunkRenderSystem::ChunkRenderSystem(
    RenderDeviceVk *device, vk::RenderPass renderPass,
    vk::DescriptorSetLayout descriptorSetLayout)
    : m_device{device} {
  ZoneScoped;
  createPipelineLayout(descriptorSetLayout);
  createPipeline(renderPass);
}

ChunkRenderSystem::~ChunkRenderSystem() {
  ZoneScoped;
  m_device->getDevice().destroyPipelineLayout(m_pipelineLayout);
}

void ChunkRenderSystem::render(FrameData &frameData) {
  ZoneScoped;
  m_pipeline->bind(frameData.commandBuffer);

  frameData.commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1,
      &frameData.globalDescriptorSet, 0, nullptr);

  for (auto &chunk : frameData.chunks) {
    PushConstantData push = {
        .chunkPos = {chunk->worldX(), chunk->worldZ()},
    };
    frameData.commandBuffer.pushConstants(
        m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, sizeof(PushConstantData), &push);
    if (chunk->getMesh() != nullptr) {
      chunk->getMesh()->bind(frameData.commandBuffer);
      chunk->getMesh()->draw(frameData.commandBuffer);
    }
  }
  m_prevChunksToRender[frameData.frameIndex] = frameData.chunks;
}

void ChunkRenderSystem::createPipelineLayout(
    vk::DescriptorSetLayout descriptorSetLayout) {
  ZoneScoped;
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
  ZoneScoped;
  PipelineVkConfigInfo pipelineConfig = {};
  PipelineVk::defaultPipelineVkConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;
  pipelineConfig.vertexInputAttributeDescriptions =
      ChunkVertex::getAttributeDescriptions();
  pipelineConfig.vertexInputBindingDescriptions =
      ChunkVertex::getBindingDescriptions();
  m_pipeline = std::make_unique<PipelineVk>(
      m_device, "shaders/test_shader.vert.spv", "shaders/test_shader.frag.spv",
      pipelineConfig);
}