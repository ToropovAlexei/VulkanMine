#include "PipelineVk.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

PipelineVk::PipelineVk(RenderDeviceVk *device, const std::string &vertFilePath,
                       const std::string &fragFilePath,
                       const PipelineVkConfigInfo &configInfo)
    : m_device(device) {
  m_vertShaderModule = std::make_unique<ShaderModuleVk>(
      device, vertFilePath, vk::ShaderStageFlagBits::eVertex);
  m_fragShaderModule = std::make_unique<ShaderModuleVk>(
      device, fragFilePath, vk::ShaderStageFlagBits::eFragment);
  createPipelineVk(configInfo);
}

void PipelineVk::createPipelineVk(const PipelineVkConfigInfo &configInfo) {
  vk::PipelineShaderStageCreateInfo shaderStages[] = {
      m_vertShaderModule->getShaderStageInfo(),
      m_fragShaderModule->getShaderStageInfo()};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.vertexBindingDescriptionCount =
      static_cast<uint32_t>(configInfo.vertexInputBindingDescriptions.size());
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(configInfo.vertexInputAttributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions =
      configInfo.vertexInputAttributeDescriptions.data();
  vertexInputInfo.pVertexBindingDescriptions =
      configInfo.vertexInputBindingDescriptions.data();

  vk::GraphicsPipelineCreateInfo pipelineInfo = {
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &configInfo.inputAssemblyInfo,
      .pViewportState = &configInfo.viewportInfo,
      .pRasterizationState = &configInfo.rasterizationInfo,
      .pMultisampleState = &configInfo.multisampleInfo,
      .pDepthStencilState = &configInfo.depthStencilInfo,
      .pColorBlendState = &configInfo.colorBlendInfo,
      .pDynamicState = &configInfo.dynamicStateInfo,
      .layout = configInfo.pipelineLayout,
      .renderPass = configInfo.renderPass,
      .subpass = configInfo.subpass,
      .basePipelineIndex = -1,
      .basePipelineHandle = VK_NULL_HANDLE};

  auto result =
      m_device->getDevice().createGraphicsPipeline(nullptr, pipelineInfo);

  if (result.result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  m_graphicsPipeline = result.value;
}

PipelineVk::~PipelineVk() {
  m_device->getDevice().destroyPipeline(m_graphicsPipeline);
}

void PipelineVk::defaultPipelineVkConfigInfo(PipelineVkConfigInfo &configInfo) {
  configInfo.inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
  configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  configInfo.viewportInfo.viewportCount = 1;
  configInfo.viewportInfo.pViewports = nullptr;
  configInfo.viewportInfo.scissorCount = 1;
  configInfo.viewportInfo.pScissors = nullptr;

  configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
  configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  configInfo.rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
  configInfo.rasterizationInfo.lineWidth = 1.0f;
  configInfo.rasterizationInfo.cullMode =
      vk::CullModeFlagBits::eNone; // TODO Change it
  configInfo.rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
  configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
  configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
  configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

  configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
  configInfo.multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
  configInfo.multisampleInfo.minSampleShading = 1.0f;          // Optional
  configInfo.multisampleInfo.pSampleMask = nullptr;            // Optional
  configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
  configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

  configInfo.colorBlendAttachment.colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor =
      vk::BlendFactor::eOne; // Optional
  configInfo.colorBlendAttachment.dstColorBlendFactor =
      vk::BlendFactor::eZero;                                       // Optional
  configInfo.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
  configInfo.colorBlendAttachment.srcAlphaBlendFactor =
      vk::BlendFactor::eOne; // Optional
  configInfo.colorBlendAttachment.dstAlphaBlendFactor =
      vk::BlendFactor::eZero;                                       // Optional
  configInfo.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional

  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = vk::LogicOp::eCopy; // Optional
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

  configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
  configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
  configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
  configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.front = {}; // Optional
  configInfo.depthStencilInfo.back = {};  // Optional

  configInfo.dynamicStateEnables = {vk::DynamicState::eViewport,
                                    vk::DynamicState::eScissor};
  configInfo.dynamicStateInfo.pDynamicStates =
      configInfo.dynamicStateEnables.data();
  configInfo.dynamicStateInfo.dynamicStateCount =
      static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
  configInfo.dynamicStateInfo.flags = {};
}

void PipelineVk::bind(vk::CommandBuffer commandBuffer) {
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             m_graphicsPipeline);
}