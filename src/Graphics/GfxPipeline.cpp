#include "GfxPipeline.h"
#include <fstream>
#include <iostream>

GfxPipeline::GfxPipeline(GfxDevice &gfxDevice, const std::string &vertFilePath,
                         const std::string &fragFilePath,
                         const GfxPipelineConfigInfo &configInfo)
    : gfxDevice(gfxDevice) {
  createGfxPipeline(vertFilePath, fragFilePath, configInfo);
}

std::vector<char> GfxPipeline::readFile(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

void GfxPipeline::createGfxPipeline(const std::string &vertFilePath,
                                    const std::string &fragFilePath,
                                    const GfxPipelineConfigInfo &configInfo) {
  auto vertShaderCode = readFile(vertFilePath);
  auto fragShaderCode = readFile(fragFilePath);

  std::cout << "Vert Shader Code Size: " << vertShaderCode.size() << std::endl;
  std::cout << "Frag Shader Code Size: " << fragShaderCode.size() << std::endl;
}

GfxPipeline::~GfxPipeline() {}

void GfxPipeline::createShaderModule(const std::vector<char> &code,
                                     VkShaderModule *shaderModule) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(gfxDevice.device(), &createInfo, nullptr,
                           shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
}

GfxPipelineConfigInfo
GfxPipeline::defaultGfxPipelineConfigInfo(uint32_t width, uint32_t height) {
  GfxPipelineConfigInfo configInfo{};

  return configInfo;
}