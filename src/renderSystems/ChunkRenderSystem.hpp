#pragma once

#include "../core/NonCopyable.hpp"
#include "../renderer/backend/PipelineVk.hpp"
#include "../renderer/backend/SwapChainVk.hpp"
#include "../world/Chunk.hpp"
#include "glm/fwd.hpp"
#include <array>
#include <cstddef>
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
  size_t frameIndex;
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
  // Нужно, чтобы буффер с мешем не удалился до отрисовки кадра
  std::array<std::vector<std::shared_ptr<Chunk>>,
             SwapChainVk::MAX_FRAMES_IN_FLIGHT>
      m_prevChunksToRender;
};