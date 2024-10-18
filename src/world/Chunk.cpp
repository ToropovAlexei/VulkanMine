#include "Chunk.hpp"
#include "BlockId.hpp"
#include "Tracy/tracy/Tracy.hpp"
#include "Voxel.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

Chunk::Chunk(BlocksManager &blocksManager, TextureAtlas &textureAtlas, int x,
             int z)
    : m_x{x}, m_z{z}, m_worldX{toWorldPos(x)}, m_worldZ{toWorldPos(z)},
      m_blocksManager{blocksManager}, m_textureAtlas{textureAtlas} {
  ZoneScoped;
  for (size_t i = 0; i < CHUNK_VOLUME; i++) {
    m_voxels[i] = Voxel(BlockId::Air);
  }
}

void Chunk::generateMesh(RenderDeviceVk *device) {
  if (m_vertices.empty()) {
    return;
  }

  m_mesh = std::make_unique<Mesh<ChunkVertex>>(device, m_vertices, m_indices);
  m_vertices.clear();
  m_indices.clear();
}

void Chunk::addFrontFace(int x, int y, int z, float textureIdx) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(m_vertices.size());

  // Вершины передней грани (на +Z)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f); // Верхний левый

  // Добавляем вершины
  m_vertices.emplace_back(ChunkVertex{v0, glm::vec3(0.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v1, glm::vec3(1.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v2, glm::vec3(1.0f, 1.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v3, glm::vec3(0.0f, 1.0f, textureIdx)});

  // Добавляем индексы для двух треугольников
  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 1);
  m_indices.push_back(startIndex + 2);

  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 2);
  m_indices.push_back(startIndex + 3);
}

void Chunk::addBackFace(int x, int y, int z, float textureIdx) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(m_vertices.size());

  // Вершины задней грани (на -Z)
  glm::vec3 v0 = glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f); // Верхний левый

  // Добавляем вершины
  m_vertices.emplace_back(ChunkVertex{v0, glm::vec3(0.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v1, glm::vec3(1.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v2, glm::vec3(1.0f, 1.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v3, glm::vec3(0.0f, 1.0f, textureIdx)});

  // Добавляем индексы для двух треугольников
  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 1);
  m_indices.push_back(startIndex + 2);

  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 2);
  m_indices.push_back(startIndex + 3);
}

void Chunk::addLeftFace(int x, int y, int z, float textureIdx) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(m_vertices.size());

  // Вершины левой грани (на -X)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f); // Верхний левый

  // Добавляем вершины
  m_vertices.emplace_back(ChunkVertex{v0, glm::vec3(0.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v1, glm::vec3(1.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v2, glm::vec3(1.0f, 1.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v3, glm::vec3(0.0f, 1.0f, textureIdx)});

  // Добавляем индексы для двух треугольников
  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 1);
  m_indices.push_back(startIndex + 2);

  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 2);
  m_indices.push_back(startIndex + 3);
}

void Chunk::addRightFace(int x, int y, int z, float textureIdx) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(m_vertices.size());

  // Вершины правой грани (на +X)
  glm::vec3 v0 = glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f); // Верхний левый

  // Добавляем вершины
  m_vertices.emplace_back(ChunkVertex{v0, glm::vec3(0.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v1, glm::vec3(1.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v2, glm::vec3(1.0f, 1.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v3, glm::vec3(0.0f, 1.0f, textureIdx)});

  // Добавляем индексы для двух треугольников
  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 1);
  m_indices.push_back(startIndex + 2);

  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 2);
  m_indices.push_back(startIndex + 3);
}

void Chunk::addTopFace(int x, int y, int z, float textureIdx) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(m_vertices.size());

  // Вершины верхней грани (на +Y)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 1.0f, z + 1.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 1.0f, z + 0.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 1.0f, z + 0.0f); // Верхний левый

  // Добавляем вершины
  m_vertices.emplace_back(ChunkVertex{v0, glm::vec3(0.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v1, glm::vec3(1.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v2, glm::vec3(1.0f, 1.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v3, glm::vec3(0.0f, 1.0f, textureIdx)});

  // Добавляем индексы для двух треугольников
  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 1);
  m_indices.push_back(startIndex + 2);

  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 2);
  m_indices.push_back(startIndex + 3);
}

void Chunk::addBottomFace(int x, int y, int z, float textureIdx) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(m_vertices.size());

  // Вершины нижней грани (на -Y)
  glm::vec3 v0 = glm::vec3(x + 0.0f, y + 0.0f, z + 0.0f); // Нижний левый
  glm::vec3 v1 = glm::vec3(x + 1.0f, y + 0.0f, z + 0.0f); // Нижний правый
  glm::vec3 v2 = glm::vec3(x + 1.0f, y + 0.0f, z + 1.0f); // Верхний правый
  glm::vec3 v3 = glm::vec3(x + 0.0f, y + 0.0f, z + 1.0f); // Верхний левый

  // Добавляем вершины
  m_vertices.emplace_back(ChunkVertex{v0, glm::vec3(0.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v1, glm::vec3(1.0f, 0.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v2, glm::vec3(1.0f, 1.0f, textureIdx)});
  m_vertices.emplace_back(ChunkVertex{v3, glm::vec3(0.0f, 1.0f, textureIdx)});

  // Добавляем индексы для двух треугольников
  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 1);
  m_indices.push_back(startIndex + 2);

  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 2);
  m_indices.push_back(startIndex + 3);
}

int Chunk::toWorldPos(int x) { return x * Chunk::CHUNK_SIZE; }

bool Chunk::canAddFace(int x, int y, int z) const {
  if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 ||
      z >= CHUNK_SIZE) {
    return true;
  }
  auto idx = getIdxFromCoords(x, y, z);
  auto voxel = m_voxels[idx];
  auto blockId = voxel.blockId;
  auto &block = m_blocksManager.getBlockById(blockId);

  return !block.isOpaque();
}

void Chunk::generateVerticesAndIndices() {
  ZoneScoped;
  std::vector<ChunkVertex> m_vertices;
  m_vertices.reserve(1.6e6);
  std::vector<uint32_t> m_indices;
  m_indices.reserve(2.4e6);

  for (int y = 0; y < CHUNK_HEIGHT; y++) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
      for (int z = 0; z < CHUNK_SIZE; z++) {
        auto &block = m_blocksManager.getBlockById(
            m_voxels[getIdxFromCoords(x, y, z)].blockId);
        if (block.id() == BlockId::Air) {
          continue;
        }

        if (canAddFace(x, y + 1, z)) {
          addTopFace(x, y, z,
                     m_textureAtlas.getTextureIdx(
                         block.getFaceTexture(Block::Faces::Top)));
        }
        if (canAddFace(x, y - 1, z)) {
          addBottomFace(x, y, z,
                        m_textureAtlas.getTextureIdx(
                            block.getFaceTexture(Block::Faces::Bottom)));
        }
        if (canAddFace(x, y, z + 1)) {
          addFrontFace(x, y, z,
                       m_textureAtlas.getTextureIdx(
                           block.getFaceTexture(Block::Faces::Front)));
        }
        if (canAddFace(x, y, z - 1)) {
          addBackFace(x, y, z,
                      m_textureAtlas.getTextureIdx(
                          block.getFaceTexture(Block::Faces::Back)));
        }
        if (canAddFace(x - 1, y, z)) {
          addLeftFace(x, y, z,
                      m_textureAtlas.getTextureIdx(
                          block.getFaceTexture(Block::Faces::Left)));
        }
        if (canAddFace(x + 1, y, z)) {
          addRightFace(x, y, z,
                       m_textureAtlas.getTextureIdx(
                           block.getFaceTexture(Block::Faces::Right)));
        }
      }
    }
  }
}