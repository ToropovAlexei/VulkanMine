#pragma once

#include "../renderSystems/ChunkVertex.hpp"
#include "../renderer/Mesh.hpp"
#include "BlocksManager.hpp"
#include "TextureAtlas.hpp"
#include "Voxel.hpp"
#include <atomic>
#include <memory>
#include <vector>

class Chunk {
  friend class WorldGenerator;

public:
  Chunk(BlocksManager &blocksManager, TextureAtlas &textureAtlas, int x, int z);
  Chunk(const Chunk &) = delete;
  Chunk(Chunk &&) = delete;

  inline int x() const noexcept { return m_x; }
  inline int z() const noexcept { return m_z; }
  inline int worldX() const noexcept { return m_worldX; }
  inline int worldZ() const noexcept { return m_worldZ; }

  inline void setBlock(size_t idx, BlockId id) noexcept {
    if (idx >= m_voxels.size()) {
      const int y = (static_cast<int>(idx + 1) / CHUNK_SQ_SIZE) + 1;
      if (m_maxY < y) {
        const size_t newSize = m_voxels.size() + static_cast<size_t>((y - m_maxY) * CHUNK_SQ_SIZE);
        m_voxels.resize(newSize);
        m_maxY = y;
      }
    }
    m_voxels[idx] = Voxel(id);
    if (id == BlockId::Air) {
      shrinkAirBlocks();
    }
  };

  inline void setBlock(int x, int y, int z, BlockId id) noexcept {
    const size_t idx = getIdxFromCoords(x, y, z);
    if (idx >= m_voxels.size()) {
      if (m_maxY < y) {
        const size_t newSize = m_voxels.size() + static_cast<size_t>((y - m_maxY) * CHUNK_SQ_SIZE);
        m_voxels.resize(newSize);
        updateMaxY();
      }
    }
    m_voxels[idx] = Voxel(id);
    if (id == BlockId::Air) {
      shrinkAirBlocks();
    }
  };

  inline bool isModified() const noexcept { return m_isModified; };
  inline void setIsModified(bool isModified) noexcept { m_isModified = isModified; };
  inline bool isMeshOutdated() const noexcept { return m_isMeshOutdated; };

  std::shared_ptr<Mesh<ChunkVertex>> &getMesh() { return m_mesh; }
  void generateVerticesAndIndices(std::shared_ptr<Chunk> front, std::shared_ptr<Chunk> back,
                                  std::shared_ptr<Chunk> left, std::shared_ptr<Chunk> right);
  void generateMesh(RenderDeviceVk *device);
  void clearVerticesAndIndices();

public:
  static constexpr int CHUNK_SIZE = 16;
  static constexpr int CHUNK_SQ_SIZE = CHUNK_SIZE * CHUNK_SIZE;
  static constexpr int CHUNK_HEIGHT = 256;
  static constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;
  static constexpr int LAST_BLOCK_IDX = CHUNK_SIZE - 1;
  static constexpr int HIGHEST_BLOCK_IDX = CHUNK_HEIGHT - 1;

private:
  void addFrontFace(int x, int y, int z, float textureIdx);
  void addBackFace(int x, int y, int z, float textureIdx);
  void addLeftFace(int x, int y, int z, float textureIdx);
  void addRightFace(int x, int y, int z, float textureIdx);
  void addTopFace(int x, int y, int z, float textureIdx);
  void addBottomFace(int x, int y, int z, float textureIdx);
  static int toWorldPos(int x);
  inline size_t getIdxFromCoords(int x, int y, int z) const noexcept {
    return static_cast<size_t>(x + z * CHUNK_SIZE + y * CHUNK_SQ_SIZE);
  };
  inline bool canAddFace(int x, int y, int z) const noexcept {
    assert(x >= 0 && x < CHUNK_SIZE);
    assert(y >= 0 && y <= CHUNK_HEIGHT);
    assert(z >= 0 && z < CHUNK_SIZE);
    return canAddFace(getIdxFromCoords(x, y, z));
  };
  inline bool canAddFace(size_t idx) const noexcept {
    if (idx >= m_voxels.size()) {
      return true;
    }
    auto &block = m_blocksManager.getBlockById(m_voxels[idx].blockId);

    return !block.isOpaque();
  };
  void shrinkAirBlocks();
  inline void updateMaxY() noexcept { m_maxY = (static_cast<int>(m_voxels.size()) / CHUNK_SQ_SIZE) - 1; };

private:
  int m_x;
  int m_z;
  int m_worldX;
  int m_worldZ;
  bool m_isModified = true;
  bool m_isMeshOutdated = true;
  int m_maxY = 0;
  BlocksManager &m_blocksManager;
  TextureAtlas &m_textureAtlas;

  std::vector<Voxel> m_voxels;

  std::vector<ChunkVertex> m_vertices;
  std::vector<uint32_t> m_indices;
  std::shared_ptr<Mesh<ChunkVertex>> m_mesh;
  std::atomic_bool m_isLocked;
};