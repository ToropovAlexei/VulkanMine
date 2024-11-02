#include "Chunk.hpp"
#include "BlockId.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <tracy/Tracy.hpp>
#include <vector>

Chunk::Chunk(BlocksManager &blocksManager, TextureAtlas &textureAtlas, int x,
             int z)
    : m_x{x}, m_z{z}, m_worldX{toWorldPos(x)}, m_worldZ{toWorldPos(z)},
      m_blocksManager{blocksManager}, m_textureAtlas{textureAtlas} {
  ZoneScoped;
  m_voxels.reserve(CHUNK_SQ_SIZE * 64);
}

void Chunk::generateMesh(RenderDeviceVk *device) {
  if (m_mutex.try_lock()) {
    if (m_vertices.empty()) {
      m_mutex.unlock();
      return;
    }

    m_mesh = std::make_shared<Mesh<ChunkVertex>>(device, m_vertices, m_indices);
    m_isMeshOutdated = false;
    m_mutex.unlock();
  }
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
  assert(x >= 0 && x < CHUNK_SIZE);
  assert(y >= 0 && y <= CHUNK_HEIGHT);
  assert(z >= 0 && z < CHUNK_SIZE);
  size_t idx = getIdxFromCoords(x, y, z);
  if (idx >= m_voxels.size()) {
    return true;
  }
  auto &block = m_blocksManager.getBlockById(m_voxels[idx].blockId);

  return !block.isOpaque();
}

void Chunk::generateVerticesAndIndices(std::shared_ptr<Chunk> front,
                                       std::shared_ptr<Chunk> back,
                                       std::shared_ptr<Chunk> left,
                                       std::shared_ptr<Chunk> right) {
  ZoneScoped;
  std::lock_guard<std::mutex> lock(m_mutex);
  // TODO очищать в другое время
  clearVerticesAndIndices();
  m_vertices.reserve(50000);
  m_indices.reserve(60000);

  const int maxY = static_cast<int>(m_voxels.size()) / CHUNK_SQ_SIZE;
  size_t voxelIdx = 0;
  for (int y = 0; y < maxY; y++) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
      for (int x = 0; x < CHUNK_SIZE; x++) {
        auto &block =
            m_blocksManager.getBlockById(m_voxels[voxelIdx++].blockId);
        if (block.id() == BlockId::Air) {
          continue;
        }

        if (y == HIGHEST_BLOCK_IDX || canAddFace(x, y + 1, z)) {
          addTopFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Top));
        }
        if (y == 0 || canAddFace(x, y - 1, z)) {
          addBottomFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Bottom));
        }

        if (z == 0) {
          if (!front || front->canAddFace(x, y, LAST_BLOCK_IDX)) {
            addBackFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Front));
          }
        } else {
          if (canAddFace(x, y, z - 1)) {
            addBackFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Front));
          }
        }
        if (z == LAST_BLOCK_IDX) {
          if (!back || back->canAddFace(x, y, 0)) {
            addFrontFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Back));
          }
        } else {
          if (canAddFace(x, y, z + 1)) {
            addFrontFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Back));
          }
        }

        if (x == 0) {
          if (!left || left->canAddFace(LAST_BLOCK_IDX, y, z)) {
            addLeftFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Left));
          }
        } else {
          if (canAddFace(x - 1, y, z)) {
            addLeftFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Left));
          }
        }
        if (x == LAST_BLOCK_IDX) {
          if (!right || right->canAddFace(0, y, z)) {
            addRightFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Right));
          }
        } else {
          if (canAddFace(x + 1, y, z)) {
            addRightFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Right));
          }
        }
      }
    }
  }
  m_isModified = false;
  m_isMeshOutdated = true;
}

void Chunk::setBlock(int x, int y, int z, BlockId id) {
  const size_t idx = getIdxFromCoords(x, y, z);
  if (idx >= m_voxels.size()) {
    const int prevY = (static_cast<int>(m_voxels.size()) / CHUNK_SQ_SIZE) - 1;
    if (prevY < y) {
      const size_t newSize =
          m_voxels.size() + static_cast<size_t>((y - prevY) * CHUNK_SQ_SIZE);
      m_voxels.resize(newSize);
    }
  }
  m_voxels[idx] = Voxel(id);
  if (id == BlockId::Air) {
    shrinkAirBlocks();
  }
};

void Chunk::shrinkAirBlocks() {
  const size_t prevY =
      (static_cast<size_t>(m_voxels.size()) / CHUNK_SQ_SIZE) - 1;
  bool isAirOnly = true;
  for (size_t i = prevY * CHUNK_SQ_SIZE; i < m_voxels.size(); i++) {
    if (m_voxels[i].blockId != BlockId::Air) {
      isAirOnly = false;
    }
  }
  if (isAirOnly) {
    const size_t newSize = m_voxels.size() - static_cast<size_t>(CHUNK_SQ_SIZE);
    m_voxels.resize(newSize);
  }
}

void Chunk::clearVerticesAndIndices() {
  m_vertices.clear();
  m_indices.clear();
}