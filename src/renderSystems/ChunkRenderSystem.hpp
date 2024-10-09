#pragma once

#include "../core/NonCopyable.hpp"
#include "../renderer/backend/PipelineVk.hpp"
#include "../world/Chunk.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

struct PushConstantData {
  glm::vec2 chunkPos;
};

struct FrameData {
  vk::CommandBuffer commandBuffer;
  std::vector<std::shared_ptr<Chunk>> chunks;
  vk::DescriptorSet globalDescriptorSet;
};

class ChunkRenderSystem : NonCopyable {
public:
  ChunkRenderSystem(RenderDeviceVk *device, vk::RenderPass renderPass,
                    vk::DescriptorSetLayout descriptorSetLayout);
  ~ChunkRenderSystem();

  void render(FrameData &frameData);

private:
  void createPipelineLayout(vk::DescriptorSetLayout descriptorSetLayout);
  void createPipeline(vk::RenderPass renderPass);

private:
  RenderDeviceVk *m_device;
  std::unique_ptr<PipelineVk> m_pipeline;
  vk::PipelineLayout m_pipelineLayout;
};