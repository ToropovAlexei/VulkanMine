#pragma once

#include "../GameObject.hpp"
#include "../utils/NonCopyable.hpp"
#include "GfxDevice.hpp"
#include "GfxPipeline.h"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class SimpleRenderSystem : NonCopyable {
public:
  SimpleRenderSystem(GfxDevice &gfxDevice, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  void renderGameObjects(VkCommandBuffer commandBuffer,
                         std::vector<GameObject> &gameObjects);

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

private:
  GfxDevice &gfxDevice;
  std::unique_ptr<GfxPipeline> gfxPipeline;
  VkPipelineLayout pipelineLayout;
};