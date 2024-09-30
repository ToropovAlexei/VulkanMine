#pragma once

#include "../core/NonCopyable.hpp"
#include "../renderer/Mesh.hpp"
#include "../renderer/backend/PipelineVk.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

struct PushConstantData {
  glm::mat4 modelMatrix{1.0f};
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;

  static std::vector<vk::VertexInputBindingDescription>
  getBindingDescriptions() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

    return bindingDescriptions;
  }

  static std::vector<vk::VertexInputAttributeDescription>
  getAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
        {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)},
    };
  }
};

struct GameObject {
  Mesh<Vertex> mesh;
  glm::mat4 model;
};

struct FrameData {
  vk::CommandBuffer commandBuffer;
  std::vector<GameObject> gameObjects;
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