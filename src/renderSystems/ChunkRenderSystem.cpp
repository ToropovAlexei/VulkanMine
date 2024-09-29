#include "ChunkRenderSystem.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include <vulkan/vulkan_core.h>

struct PushConstantData {
  glm::mat4 modelMatrix{1.0f};
};

ChunkRenderSystem::ChunkRenderSystem(RenderDeviceVk *device,
                                     vk::RenderPass renderPass,
                                     vk::DescriptorSetLayout globalSetLayout)
    : m_device{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

ChunkRenderSystem::~ChunkRenderSystem() {
  m_device->getDevice().destroyPipelineLayout(m_pipelineLayout);
}

void ChunkRenderSystem::render(FrameData &frameData) {
  m_pipeline->bind(frameData.commandBuffer);

  for (auto &mesh : frameData.meshes) {
    mesh.bind(frameData.commandBuffer);
    mesh.draw(frameData.commandBuffer);
  }
}

void ChunkRenderSystem::createPipelineLayout(
    vk::DescriptorSetLayout globalSetLayout) {
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};

  m_pipelineLayout =
      m_device->getDevice().createPipelineLayout(pipelineLayoutInfo);
}

void ChunkRenderSystem::createPipeline(vk::RenderPass renderPass) {
  PipelineVkConfigInfo pipelineConfig = {};
  PipelineVk::defaultPipelineVkConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;
  m_pipeline = std::make_unique<PipelineVk>(
      m_device, "shaders/test_shader.vert.spv", "shaders/test_shader.frag.spv",
      pipelineConfig);
}