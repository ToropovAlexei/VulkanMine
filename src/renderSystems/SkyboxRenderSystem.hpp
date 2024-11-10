#pragma once

#include "ChunkRenderSystem.hpp"
#include "glm/fwd.hpp"
#include <array>
#include <memory>
#include <vector>

class SkyboxRenderSystem {
  struct SkyboxVertex {
    glm::vec3 pos;
    uint8_t skyRegion;

    static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions() {
      std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
      bindingDescriptions[0].binding = 0;
      bindingDescriptions[0].stride = sizeof(SkyboxVertex);
      bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

      return bindingDescriptions;
    }

    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
      return {
          {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(SkyboxVertex, pos)},
          {1, 0, vk::Format::eR32Uint, offsetof(SkyboxVertex, skyRegion)},
      };
    }
  };

public:
  SkyboxRenderSystem(RenderDeviceVk *device, vk::RenderPass renderPass, vk::DescriptorSetLayout descriptorSetLayout);
  ~SkyboxRenderSystem();

  void render(FrameData &frameData);

private:
  void createPipelineLayout(vk::DescriptorSetLayout descriptorSetLayout);
  void createPipeline(vk::RenderPass renderPass);

private:
  RenderDeviceVk *m_device;
  std::unique_ptr<PipelineVk> m_pipeline;
  vk::PipelineLayout m_pipelineLayout;
  std::unique_ptr<Mesh<SkyboxVertex>> m_mesh;

  std::array<SkyboxVertex, 24> m_vertices = {
      // Передняя грань
      SkyboxVertex{{-1.0f, 1.0f, 1.0f}, 6},  // 0
      SkyboxVertex{{1.0f, 1.0f, 1.0f}, 5},   // 1
      SkyboxVertex{{1.0f, -1.0f, 1.0f}, 7},  // 2
      SkyboxVertex{{-1.0f, -1.0f, 1.0f}, 4}, // 3

      // Задняя грань
      SkyboxVertex{{-1.0f, 1.0f, -1.0f}, 1},  // 4
      SkyboxVertex{{1.0f, 1.0f, -1.0f}, 2},   // 5
      SkyboxVertex{{1.0f, -1.0f, -1.0f}, 3},  // 6
      SkyboxVertex{{-1.0f, -1.0f, -1.0f}, 0}, // 7

      // Левая грань
      SkyboxVertex{{-1.0f, 1.0f, 1.0f}, 6},   // 8
      SkyboxVertex{{-1.0f, -1.0f, 1.0f}, 4},  // 9
      SkyboxVertex{{-1.0f, 1.0f, -1.0f}, 1},  // 10
      SkyboxVertex{{-1.0f, -1.0f, -1.0f}, 0}, // 11

      // Правая грань
      SkyboxVertex{{1.0f, 1.0f, 1.0f}, 5},   // 12
      SkyboxVertex{{1.0f, -1.0f, 1.0f}, 7},  // 13
      SkyboxVertex{{1.0f, 1.0f, -1.0f}, 2},  // 14
      SkyboxVertex{{1.0f, -1.0f, -1.0f}, 3}, // 15

      // Верхняя грань
      SkyboxVertex{{-1.0f, 1.0f, 1.0f}, 6},  // 16
      SkyboxVertex{{1.0f, 1.0f, 1.0f}, 5},   // 17
      SkyboxVertex{{-1.0f, 1.0f, -1.0f}, 1}, // 18
      SkyboxVertex{{1.0f, 1.0f, -1.0f}, 2},  // 19

      // Нижняя грань
      SkyboxVertex{{-1.0f, -1.0f, 1.0f}, 4},  // 20
      SkyboxVertex{{1.0f, -1.0f, 1.0f}, 7},   // 21
      SkyboxVertex{{-1.0f, -1.0f, -1.0f}, 0}, // 22
      SkyboxVertex{{1.0f, -1.0f, -1.0f}, 3},  // 23
  };

  std::array<uint32_t, 36> m_indices = {
      // Передняя грань
      0, 1, 2, 0, 2, 3,

      // Задняя грань
      4, 5, 6, 4, 6, 7,

      // Левая грань
      8, 9, 10, 8, 10, 11,

      // Правая грань
      12, 13, 15, 12, 15, 14,

      // Верхняя грань
      16, 17, 19, 16, 19, 18,

      // Нижняя грань
      20, 21, 23, 20, 23, 22};
};