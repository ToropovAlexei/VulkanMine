#pragma once

#include "../core/NonCopyable.hpp"
#include "../renderer/Mesh.hpp"
#include "../renderer/backend/PipelineVk.hpp"
#include "glm/ext/vector_float3.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};

struct FrameData {
  vk::CommandBuffer commandBuffer;
  std::vector<Mesh<Vertex>> meshes;
};

class ChunkRenderSystem : NonCopyable {
public:
  ChunkRenderSystem(RenderDeviceVk *device, vk::RenderPass renderPass,
                    vk::DescriptorSetLayout globalSetLayout);
  ~ChunkRenderSystem();

  void render(FrameData &frameData);

private:
  void createPipelineLayout(vk::DescriptorSetLayout globalSetLayout);
  void createPipeline(vk::RenderPass renderPass);

private:
  RenderDeviceVk *m_device;
  std::unique_ptr<PipelineVk> m_pipeline;
  vk::PipelineLayout m_pipelineLayout;
};