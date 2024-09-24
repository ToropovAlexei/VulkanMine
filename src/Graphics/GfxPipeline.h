#pragma once

#include "GfxDevice.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct GfxPipelineConfigInfo : NonCopyable {
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class GfxPipeline : NonCopyable {
public:
  GfxPipeline(GfxDevice &gfxDevice, const std::string &vertFilePath,
              const std::string &fragFilePath,
              const GfxPipelineConfigInfo &configInfo);

  ~GfxPipeline();

  static void defaultGfxPipelineConfigInfo(GfxPipelineConfigInfo &configInfo);

  void bind(VkCommandBuffer commandBuffer);

private:
  static std::vector<char> readFile(const std::string &filePath);

  void createGfxPipeline(const std::string &vertFilePath,
                         const std::string &fragFilePath,
                         const GfxPipelineConfigInfo &configInfo);

  void createShaderModule(const std::vector<char> &code,
                          VkShaderModule *shaderModule);

  GfxDevice &m_gfxDevice;
  VkPipeline m_graphicsPipeline;
  VkShaderModule m_vertShaderModule;
  VkShaderModule m_fragShaderModule;
};