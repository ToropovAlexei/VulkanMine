#include "GridRenderSystem.hpp"
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

GridRenderSystem::GridRenderSystem(RenderDeviceVk *device, vk::RenderPass renderPass,
                                   vk::DescriptorSetLayout descriptorSetLayout)
    : m_device{device} {
  ZoneScoped;
  createPipelineLayout(descriptorSetLayout);
  createPipeline(renderPass);
  m_mesh = std::make_unique<Mesh<GridVertex>>(m_device, m_vertices, m_indices);
}

GridRenderSystem::~GridRenderSystem() {
  ZoneScoped;
  m_device->getDevice().destroyPipelineLayout(m_pipelineLayout);
}

void GridRenderSystem::render(FrameData &frameData) {
  ZoneScoped;
  m_pipeline->bind(frameData.commandBuffer);

  frameData.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1,
                                             &frameData.globalDescriptorSet, 0, nullptr);
  m_mesh->bind(frameData.commandBuffer);
  m_mesh->draw(frameData.commandBuffer);
}

void GridRenderSystem::createPipelineLayout(vk::DescriptorSetLayout descriptorSetLayout) {
  ZoneScoped;

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
      .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts = descriptorSetLayouts.data(),
  };

  m_pipelineLayout = m_device->getDevice().createPipelineLayout(pipelineLayoutInfo);
}

void GridRenderSystem::createPipeline(vk::RenderPass renderPass) {
  ZoneScoped;
  PipelineVkConfigInfo pipelineConfig = {};
  PipelineVk::defaultPipelineVkConfigInfo(pipelineConfig);
  
  // Настройка прозрачности
  pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
  pipelineConfig.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  pipelineConfig.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  pipelineConfig.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
  pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
  pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
  pipelineConfig.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

  // Настройка глубины
  pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
  pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
  pipelineConfig.depthStencilInfo.depthCompareOp = vk::CompareOp::eLessOrEqual;
  
  // Отключаем отсечение граней и устанавливаем порядок вершин по часовой стрелке
  pipelineConfig.rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
  pipelineConfig.rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
  pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;

  // Настройка входных данных вершин
  pipelineConfig.vertexInputAttributeDescriptions = GridVertex::getAttributeDescriptions();
  pipelineConfig.vertexInputBindingDescriptions = GridVertex::getBindingDescriptions();

  // Важно: устанавливаем renderPass и pipelineLayout
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;

  m_pipeline = std::make_unique<PipelineVk>(m_device, "grid", "grid", pipelineConfig);
}