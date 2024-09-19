#pragma once

#include "../GameObject.hpp"
#include "../utils/NonCopyable.hpp"
#include "FrameInfo.hpp"
#include "GfxDevice.hpp"
#include "GfxPipeline.h"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class SimpleRenderSystem : NonCopyable {
public:
  SimpleRenderSystem(GfxDevice &gfxDevice, VkRenderPass renderPass,
                     VkDescriptorSetLayout globalSetLayout);
  ~SimpleRenderSystem();

  void renderGameObjects(FrameInfo &frameInfo,
                         std::vector<GameObject> &gameObjects);

private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

private:
  GfxDevice &gfxDevice;
  std::unique_ptr<GfxPipeline> gfxPipeline;
  VkPipelineLayout pipelineLayout;
};