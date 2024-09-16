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
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return pos == other.pos && color == other.color &&
             normal == other.normal && uv == other.uv;
    }
  };

  struct Builder {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    void loadModel(const std::string &filepath);
  };

  GfxModel(GfxDevice &gfxDevice, const Builder &builder);

  static std::unique_ptr<GfxModel>
  createGfxModelFromFile(GfxDevice &gfxDevice, const std::string &filepath);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint32_t> &indices);

private:
  GfxDevice &gfxDevice;
  std::unique_ptr<GfxBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<GfxBuffer> indexBuffer;
  uint32_t indexCount;
};