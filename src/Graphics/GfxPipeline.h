#pragma once

#include "GfxDevice.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct GfxPipelineConfigInfo {
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
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

  static GfxPipelineConfigInfo defaultGfxPipelineConfigInfo(uint32_t width,
                                                            uint32_t height);

  void bind(VkCommandBuffer commandBuffer);

private:
  static std::vector<char> readFile(const std::string &filePath);

  void createGfxPipeline(const std::string &vertFilePath,
                         const std::string &fragFilePath,
                         const GfxPipelineConfigInfo &configInfo);

  void createShaderModule(const std::vector<char> &code,
                          VkShaderModule *shaderModule);

  GfxDevice &gfxDevice;
  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
};