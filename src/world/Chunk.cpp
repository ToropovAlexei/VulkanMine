#include "Chunk.hpp"
#include "BlockId.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tracy/Tracy.hpp>
#include <vector>

Chunk::Chunk(BlocksManager &blocksManager, int x, int z)
    : m_x{x}, m_z{z}, m_worldX{toWorldPos(x)}, m_worldZ{toWorldPos(z)}, m_blocksManager{blocksManager} {}

void Chunk::generateMesh(RenderDeviceVk *device) {
  bool expected = false;
  if (m_isLocked.compare_exchange_strong(expected, true)) {
    if (m_vertices.empty()) {
      m_isLocked.store(false);
      return;
    }

    std::vector<ChunkVertex> tempVertices;
    std::swap(m_vertices, tempVertices);
    std::vector<uint32_t> tempIndices;
    std::swap(m_indices, tempIndices);
    m_mesh = std::make_shared<Mesh<ChunkVertex>>(device, tempVertices, tempIndices);
    m_isMeshOutdated = false;
    m_isLocked.store(false);
  }
}

void Chunk::addFrontFace(int x, int y, int z, float textureIdx) {
  ZoneScoped;
  uint32_t startIndex = static_cast<uint32_t>(m_vertices.size());

  // Вершины передней грани (на +Z)
  // Добавляем вершины
  m_vertices.emplace_back(compressVertex(x, y, z + 1, 0, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y, z + 1, 1, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y + 1, z + 1, 1, 1), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y + 1, z + 1, 0, 1), textureIdx);

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
  // Добавляем вершины
  m_vertices.emplace_back(compressVertex(x + 1, y, z, 0, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y, z, 1, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y + 1, z, 1, 1), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y + 1, z, 0, 1), textureIdx);

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
  // Добавляем вершины
  m_vertices.emplace_back(compressVertex(x, y, z, 0, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y, z + 1, 1, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y + 1, z + 1, 1, 1), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y + 1, z, 0, 1), textureIdx);

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
  // Добавляем вершины
  m_vertices.emplace_back(compressVertex(x + 1, y, z + 1, 0, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y, z, 1, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y + 1, z, 1, 1), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y + 1, z + 1, 0, 1), textureIdx);

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
  // Добавляем вершины
  m_vertices.emplace_back(compressVertex(x, y + 1, z + 1, 0, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y + 1, z + 1, 1, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y + 1, z, 1, 1), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y + 1, z, 0, 1), textureIdx);

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
  // Добавляем вершины
  m_vertices.emplace_back(compressVertex(x, y, z, 0, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y, z, 1, 0), textureIdx);
  m_vertices.emplace_back(compressVertex(x + 1, y, z + 1, 1, 1), textureIdx);
  m_vertices.emplace_back(compressVertex(x, y, z + 1, 0, 1), textureIdx);

  // Добавляем индексы для двух треугольников
  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 1);
  m_indices.push_back(startIndex + 2);

  m_indices.push_back(startIndex);
  m_indices.push_back(startIndex + 2);
  m_indices.push_back(startIndex + 3);
}

int Chunk::toWorldPos(int x) { return x * Chunk::CHUNK_SIZE; }

void Chunk::generateVerticesAndIndices(std::shared_ptr<Chunk> front, std::shared_ptr<Chunk> back,
                                       std::shared_ptr<Chunk> left, std::shared_ptr<Chunk> right) {
  ZoneScoped;
  bool expected = false;
  if (!m_isLocked.compare_exchange_strong(expected, true)) {
    return;
  }
  m_vertices.reserve(6000);
  m_indices.reserve(9000);

  size_t voxelIdx = 0;
  for (int y = 0; y < m_maxY; y++) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
      for (int x = 0; x < CHUNK_SIZE; x++) {
        auto &block = m_blocksManager.getBlockById(m_voxels[voxelIdx++].blockId);
        size_t current = voxelIdx - 1;
        if (block.id() == BlockId::Air) {
          continue;
        }

        if (y == HIGHEST_BLOCK_IDX || canAddFace(current + CHUNK_SQ_SIZE)) {
          addTopFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Top));
        }
        if (y == 0 || canAddFace(current - CHUNK_SQ_SIZE)) {
          addBottomFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Bottom));
        }

        if (z == 0) {
          if (!front || front->canAddFace(x, y, LAST_BLOCK_IDX)) {
            addBackFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Front));
          }
        } else {
          if (canAddFace(current - CHUNK_SIZE)) {
            addBackFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Front));
          }
        }
        if (z == LAST_BLOCK_IDX) {
          if (!back || back->canAddFace(x, y, 0)) {
            addFrontFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Back));
          }
        } else {
          if (canAddFace(current + CHUNK_SIZE)) {
            addFrontFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Back));
          }
        }

        if (x == 0) {
          if (!left || left->canAddFace(LAST_BLOCK_IDX, y, z)) {
            addLeftFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Left));
          }
        } else {
          if (canAddFace(current - 1)) {
            addLeftFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Left));
          }
        }
        if (x == LAST_BLOCK_IDX) {
          if (!right || right->canAddFace(0, y, z)) {
            addRightFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Right));
          }
        } else {
          if (canAddFace(current + 1)) {
            addRightFace(x, y, z, block.getFaceTextureIdx(Block::Faces::Right));
          }
        }
      }
    }
  }
  m_isModified = false;
  m_isMeshOutdated = true;
  m_isLocked.store(false);
}

void Chunk::shrinkAirBlocks() {
  bool isAirOnly = true;
  for (size_t i = m_maxY * CHUNK_SQ_SIZE; i < m_voxels.size(); i++) {
    if (m_voxels[i].blockId != BlockId::Air) {
      isAirOnly = false;
    }
  }
  if (isAirOnly) {
    const size_t newSize = m_voxels.size() - static_cast<size_t>(CHUNK_SQ_SIZE);
    m_voxels.resize(newSize);
    updateMaxY();
  }
}