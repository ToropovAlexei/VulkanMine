#pragma once

#include "../Graphics/FrameInfo.hpp"
#include "../Graphics/GfxDevice.hpp"
#include "../Graphics/GfxPipeline.h"
#include "../utils/NonCopyable.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

class ChunkRenderSystem : NonCopyable {
public:
  ChunkRenderSystem(GfxDevice &gfxDevice, VkRenderPass renderPass,
                    VkDescriptorSetLayout globalSetLayout);
  ~ChunkRenderSystem();

  void renderChunks(FrameInfo &frameInfo);

private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

private:
  GfxDevice &m_gfxDevice;
  std::unique_ptr<GfxPipeline> m_gfxPipeline;
  VkPipelineLayout m_pipelineLayout;
};