#pragma once

#include "GfxDevice.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct GfxPipelineConfigInfo {};

class GfxPipeline {
public:
  GfxPipeline(GfxDevice &gfxDevice, const std::string &vertFilePath,
              const std::string &fragFilePath,
              const GfxPipelineConfigInfo &configInfo);

  ~GfxPipeline();
  GfxPipeline(const GfxPipeline &) = delete;
  GfxPipeline &operator=(const GfxPipeline &) = delete;
  GfxPipeline(GfxPipeline &&) = delete;
  GfxPipeline &operator=(GfxPipeline &&) = delete;

  static GfxPipelineConfigInfo defaultGfxPipelineConfigInfo(uint32_t width,
                                                            uint32_t height);

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