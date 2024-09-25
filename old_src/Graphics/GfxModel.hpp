#pragma once

#include "GfxBuffer.hpp"
#include "GfxDevice.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class GfxModel : NonCopyable {
public:
  struct Vertex {
    glm::vec3 pos{};
    glm::vec3 color{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return pos == other.pos && color == other.color && uv == other.uv;
    }
  };

  GfxModel(GfxDevice &gfxDevice, const std::vector<Vertex> &vertices,
           const std::vector<uint32_t> &indices);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint32_t> &indices);

private:
  GfxDevice &m_gfxDevice;
  std::unique_ptr<GfxBuffer> m_vertexBuffer;
  uint32_t m_vertexCount;

  std::unique_ptr<GfxBuffer> m_indexBuffer;
  uint32_t m_indexCount;
};