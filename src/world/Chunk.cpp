#include "Chunk.hpp"
#include "BlockId.hpp"
#include "Tracy/tracy/Tracy.hpp"
#include "Voxel.hpp"
#include <cstdint>
#include <memory>
#include <vector>

Chunk::Chunk(int x, int z)
    : m_x{x}, m_z{z}, m_worldX{toWorldPos(x)}, m_worldZ{toWorldPos(z)} {
  ZoneScoped;
  for (int i = 0; i < CHUNK_VOLUME; i++) {
    m_voxels.push_back(Voxel(BlockId::Grass));
  }
}

void Chunk::generateMesh(RenderDeviceVk *device) {
  ZoneScoped;
  std::vector<ChunkVertex> vertices;
  std::vector<uint32_t> indices;

  for (int y = 0; y < CHUNK_HEIGHT; y++) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
      for (int z = 0; z < CHUNK_SIZE; z++) {
        // Voxel &voxel =
        //     m_voxels[x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE];
        addTopFace(x, y, z, vertices, indices);
        addBottomFace(x, y, z, vertices, indices);
        addFrontFace(x, y, z, vertices, indices);
        addBackFace(x, y, z, vertices, indices);
        addLeftFace(x, y, z, vertices, indices);
        addRightFace(x, y, z, vertices, indices);
      }
    }
  }

  m_mesh = std::make_unique<Mesh<ChunkVertex>>(device, vertices, indices);
}

void Chunk::addFrontFace(int x, int y, int z,
                         std::vector<ChunkVertex> &vertices,
                         std::vector<uint32_t> &indices) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(vertices.size());

  // Вершины передней грани (на +Z)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f); // Верхний левый

  // Добавляем вершины
  vertices.emplace_back(ChunkVertex{v0, glm::vec2(0.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v1, glm::vec2(1.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v2, glm::vec2(1.0f, 1.0f)});
  vertices.emplace_back(ChunkVertex{v3, glm::vec2(0.0f, 1.0f)});

  // Добавляем индексы для двух треугольников
  indices.push_back(startIndex);
  indices.push_back(startIndex + 1);
  indices.push_back(startIndex + 2);

  indices.push_back(startIndex);
  indices.push_back(startIndex + 2);
  indices.push_back(startIndex + 3);
}

void Chunk::addBackFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                        std::vector<uint32_t> &indices) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(vertices.size());

  // Вершины задней грани (на -Z)
  glm::vec3 v0 = glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f); // Верхний левый

  // Добавляем вершины
  vertices.emplace_back(ChunkVertex{v0, glm::vec2(0.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v1, glm::vec2(1.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v2, glm::vec2(1.0f, 1.0f)});
  vertices.emplace_back(ChunkVertex{v3, glm::vec2(0.0f, 1.0f)});

  // Добавляем индексы для двух треугольников
  indices.push_back(startIndex);
  indices.push_back(startIndex + 1);
  indices.push_back(startIndex + 2);

  indices.push_back(startIndex);
  indices.push_back(startIndex + 2);
  indices.push_back(startIndex + 3);
}

void Chunk::addLeftFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                        std::vector<uint32_t> &indices) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(vertices.size());

  // Вершины левой грани (на -X)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f); // Верхний левый

  // Добавляем вершины
  vertices.emplace_back(ChunkVertex{v0, glm::vec2(0.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v1, glm::vec2(1.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v2, glm::vec2(1.0f, 1.0f)});
  vertices.emplace_back(ChunkVertex{v3, glm::vec2(0.0f, 1.0f)});

  // Добавляем индексы для двух треугольников
  indices.push_back(startIndex);
  indices.push_back(startIndex + 1);
  indices.push_back(startIndex + 2);

  indices.push_back(startIndex);
  indices.push_back(startIndex + 2);
  indices.push_back(startIndex + 3);
}

void Chunk::addRightFace(int x, int y, int z,
                         std::vector<ChunkVertex> &vertices,
                         std::vector<uint32_t> &indices) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(vertices.size());

  // Вершины правой грани (на +X)
  glm::vec3 v0 = glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f); // Верхний левый

  // Добавляем вершины
  vertices.emplace_back(ChunkVertex{v0, glm::vec2(0.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v1, glm::vec2(1.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v2, glm::vec2(1.0f, 1.0f)});
  vertices.emplace_back(ChunkVertex{v3, glm::vec2(0.0f, 1.0f)});

  // Добавляем индексы для двух треугольников
  indices.push_back(startIndex);
  indices.push_back(startIndex + 1);
  indices.push_back(startIndex + 2);

  indices.push_back(startIndex);
  indices.push_back(startIndex + 2);
  indices.push_back(startIndex + 3);
}

void Chunk::addTopFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                       std::vector<uint32_t> &indices) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(vertices.size());

  // Вершины верхней грани (на +Y)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f); // Верхний левый

  // Добавляем вершины
  vertices.emplace_back(ChunkVertex{v0, glm::vec2(0.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v1, glm::vec2(1.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v2, glm::vec2(1.0f, 1.0f)});
  vertices.emplace_back(ChunkVertex{v3, glm::vec2(0.0f, 1.0f)});

  // Добавляем индексы для двух треугольников
  indices.push_back(startIndex);
  indices.push_back(startIndex + 1);
  indices.push_back(startIndex + 2);

  indices.push_back(startIndex);
  indices.push_back(startIndex + 2);
  indices.push_back(startIndex + 3);
}

void Chunk::addBottomFace(int x, int y, int z,
                          std::vector<ChunkVertex> &vertices,
                          std::vector<uint32_t> &indices) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(vertices.size());

  // Вершины нижней грани (на -Y)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f); // Верхний левый

  // Добавляем вершины
  vertices.emplace_back(ChunkVertex{v0, glm::vec2(0.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v1, glm::vec2(1.0f, 0.0f)});
  vertices.emplace_back(ChunkVertex{v2, glm::vec2(1.0f, 1.0f)});
  vertices.emplace_back(ChunkVertex{v3, glm::vec2(0.0f, 1.0f)});

  // Добавляем индексы для двух треугольников
  indices.push_back(startIndex);
  indices.push_back(startIndex + 1);
  indices.push_back(startIndex + 2);

  indices.push_back(startIndex);
  indices.push_back(startIndex + 2);
  indices.push_back(startIndex + 3);
}

int Chunk::toWorldPos(int x) { return x * Chunk::CHUNK_SIZE; }