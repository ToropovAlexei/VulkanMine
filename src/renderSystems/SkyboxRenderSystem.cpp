#include "SkyboxRenderSystem.hpp"
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

SkyboxRenderSystem::SkyboxRenderSystem(RenderDeviceVk *device, vk::RenderPass renderPass,
                                       vk::DescriptorSetLayout descriptorSetLayout)
    : m_device{device} {
  ZoneScoped;
  createPipelineLayout(descriptorSetLayout);
  createPipeline(renderPass);
  m_mesh = std::make_unique<Mesh<SkyboxVertex>>(m_device, m_vertices, m_indices);
}

SkyboxRenderSystem::~SkyboxRenderSystem() {
  ZoneScoped;
  m_device->getDevice().destroyPipelineLayout(m_pipelineLayout);
}

void SkyboxRenderSystem::render(FrameData &frameData) {
  ZoneScoped;
  m_pipeline->bind(frameData.commandBuffer);

  frameData.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1,
                                             &frameData.globalDescriptorSet, 0, nullptr);
  m_mesh->bind(frameData.commandBuffer);
  m_mesh->draw(frameData.commandBuffer);
}

void SkyboxRenderSystem::createPipelineLayout(vk::DescriptorSetLayout descriptorSetLayout) {
  ZoneScoped;

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
      .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts = descriptorSetLayouts.data(),
  };

  m_pipelineLayout = m_device->getDevice().createPipelineLayout(pipelineLayoutInfo);
}

void SkyboxRenderSystem::createPipeline(vk::RenderPass renderPass) {
  ZoneScoped;
  PipelineVkConfigInfo pipelineConfig = {};
  PipelineVk::defaultPipelineVkConfigInfo(pipelineConfig);
  pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
  pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
  pipelineConfig.depthStencilInfo.depthCompareOp = vk::CompareOp::eAlways;
  pipelineConfig.rasterizationInfo.cullMode = vk::CullModeFlagBits::eFront;
  pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;
  pipelineConfig.vertexInputAttributeDescriptions = SkyboxVertex::getAttributeDescriptions();
  pipelineConfig.vertexInputBindingDescriptions = SkyboxVertex::getBindingDescriptions();
  m_pipeline = std::make_unique<PipelineVk>(m_device, "skybox", "skybox", pipelineConfig);
}