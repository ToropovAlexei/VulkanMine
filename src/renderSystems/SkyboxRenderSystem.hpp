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

  static constexpr float SIZE = 1000.0f;
  std::array<SkyboxVertex, 24> m_vertices = {
      // Передняя грань
      SkyboxVertex{{-SIZE, SIZE, SIZE}, 6},  // 0
      SkyboxVertex{{SIZE, SIZE, SIZE}, 5},   // 1
      SkyboxVertex{{SIZE, -SIZE, SIZE}, 7},  // 2
      SkyboxVertex{{-SIZE, -SIZE, SIZE}, 4}, // 3

      // Задняя грань
      SkyboxVertex{{-SIZE, SIZE, -SIZE}, 1},  // 4
      SkyboxVertex{{SIZE, SIZE, -SIZE}, 2},   // 5
      SkyboxVertex{{SIZE, -SIZE, -SIZE}, 3},  // 6
      SkyboxVertex{{-SIZE, -SIZE, -SIZE}, 0}, // 7

      // Левая грань
      SkyboxVertex{{-SIZE, SIZE, SIZE}, 6},   // 8
      SkyboxVertex{{-SIZE, -SIZE, SIZE}, 4},  // 9
      SkyboxVertex{{-SIZE, SIZE, -SIZE}, 1},  // 10
      SkyboxVertex{{-SIZE, -SIZE, -SIZE}, 0}, // 11

      // Правая грань
      SkyboxVertex{{SIZE, SIZE, SIZE}, 5},   // 12
      SkyboxVertex{{SIZE, -SIZE, SIZE}, 7},  // 13
      SkyboxVertex{{SIZE, SIZE, -SIZE}, 2},  // 14
      SkyboxVertex{{SIZE, -SIZE, -SIZE}, 3}, // 15

      // Верхняя грань
      SkyboxVertex{{-SIZE, SIZE, SIZE}, 6},  // 16
      SkyboxVertex{{SIZE, SIZE, SIZE}, 5},   // 17
      SkyboxVertex{{-SIZE, SIZE, -SIZE}, 1}, // 18
      SkyboxVertex{{SIZE, SIZE, -SIZE}, 2},  // 19

      // Нижняя грань
      SkyboxVertex{{-SIZE, -SIZE, SIZE}, 4},  // 20
      SkyboxVertex{{SIZE, -SIZE, SIZE}, 7},   // 21
      SkyboxVertex{{-SIZE, -SIZE, -SIZE}, 0}, // 22
      SkyboxVertex{{SIZE, -SIZE, -SIZE}, 3},  // 23
  };

  std::array<uint32_t, 36> m_indices = {
      // Передняя грань
      1, 0, 2, 2, 0, 3,

      // Задняя грань
      4, 5, 6, 4, 6, 7,

      // Левая грань
      9, 8, 10, 11, 9, 10,

      // Правая грань
      12, 13, 15, 12, 15, 14,

      // Верхняя грань
      16, 17, 19, 16, 19, 18,

      // Нижняя грань
      21, 20, 23, 23, 20, 22};
};