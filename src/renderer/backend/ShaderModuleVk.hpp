#pragma once

#include "../../core/NonCopyable.hpp"
#include "RenderDeviceVk.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class ShaderModuleVk : NonCopyable {
public:
  ShaderModuleVk(RenderDeviceVk *device, const std::string &filePath, vk::ShaderStageFlagBits stage);
  ~ShaderModuleVk();

  vk::PipelineShaderStageCreateInfo getShaderStageInfo() const;

private:
  static std::vector<char> readFile(const std::string &filename);
  void createShaderModule(const std::vector<char> &code);

private:
  RenderDeviceVk *m_device;
  vk::ShaderModule m_shaderModule{};
  vk::ShaderStageFlagBits m_stage;
};