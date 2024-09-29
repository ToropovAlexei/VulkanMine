#pragma once

#include "RenderDeviceVk.hpp"
#include "ShaderModuleVk.hpp"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct PipelineVkConfigInfo {
  vk::PipelineViewportStateCreateInfo viewportInfo;
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
  vk::PipelineMultisampleStateCreateInfo multisampleInfo;
  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
  vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<vk::DynamicState> dynamicStateEnables;
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription>
      vertexInputAttributeDescriptions;
  vk::PipelineLayout pipelineLayout = nullptr;
  vk::RenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class PipelineVk : NonCopyable {
public:
  PipelineVk(RenderDeviceVk *device, const std::string &vertFilePath,
             const std::string &fragFilePath,
             const PipelineVkConfigInfo &configInfo);

  ~PipelineVk();

  static void defaultPipelineVkConfigInfo(PipelineVkConfigInfo &configInfo);

  void bind(vk::CommandBuffer commandBuffer);

private:
  void createPipelineVk(const PipelineVkConfigInfo &configInfo);

  RenderDeviceVk *m_device;
  vk::Pipeline m_graphicsPipeline;
  std::unique_ptr<ShaderModuleVk> m_vertShaderModule;
  std::unique_ptr<ShaderModuleVk> m_fragShaderModule;
};