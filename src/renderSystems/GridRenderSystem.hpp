#pragma once

#include "../core/NonCopyable.hpp"
#include "../renderer/backend/PipelineVk.hpp"
#include "ChunkRenderSystem.hpp"
#include <cstddef>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

struct GridVertex {
  glm::vec3 pos;

  static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(GridVertex);
    bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

    return bindingDescriptions;
  }

  static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(GridVertex, pos)},
    };
  }
};

class GridRenderSystem : NonCopyable {
public:
  GridRenderSystem(RenderDeviceVk *device, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout);
  ~GridRenderSystem();

  void render(FrameData &frameData);

private:
  void createPipelineLayout(vk::DescriptorSetLayout descriptorSetLayout);
  void createPipeline(vk::RenderPass renderPass);

private:
  RenderDeviceVk *m_device;
  std::unique_ptr<PipelineVk> m_pipeline;
  vk::PipelineLayout m_pipelineLayout;

  std::unique_ptr<Mesh<GridVertex>> m_mesh;

  static constexpr float SIZE = 100.0f;

  // Простой quad в мировых координатах
  std::array<GridVertex, 4> m_vertices = {
      GridVertex{{-SIZE, 0.0f, -SIZE}},  // 0: левый нижний
      GridVertex{{SIZE, 0.0f, -SIZE}},   // 1: правый нижний
      GridVertex{{-SIZE, 0.0f, SIZE}},   // 2: левый верхний
      GridVertex{{SIZE, 0.0f, SIZE}},    // 3: правый верхний
  };

  // Два треугольника
  std::array<uint32_t, 6> m_indices = {
      0, 1, 2,  // нижний треугольник
      1, 3, 2   // верхний треугольник
  };
};