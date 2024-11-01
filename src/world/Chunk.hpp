#pragma once

#include "../renderSystems/ChunkVertex.hpp"
#include "../renderer/Mesh.hpp"
#include "BlocksManager.hpp"
#include "TextureAtlas.hpp"
#include "Voxel.hpp"
#include <memory>
#include <vector>

class Chunk {
public:
  Chunk(BlocksManager &blocksManager, TextureAtlas &textureAtlas, int x, int z);
  Chunk(const Chunk &) = delete;
  Chunk(Chunk &&) = delete;

  inline int x() const noexcept { return m_x; }
  inline int z() const noexcept { return m_z; }
  inline int worldX() const noexcept { return m_worldX; }
  inline int worldZ() const noexcept { return m_worldZ; }

  void setBlock(int x, int y, int z, BlockId id);

  std::unique_ptr<Mesh<ChunkVertex>> &getMesh() { return m_mesh; }
  void generateVerticesAndIndices(std::shared_ptr<Chunk> front,
                                  std::shared_ptr<Chunk> back,
                                  std::shared_ptr<Chunk> left,
                                  std::shared_ptr<Chunk> right);
  void generateMesh(RenderDeviceVk *device);

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
  bool canAddFace(int x, int y, int z) const;
  void shrinkAirBlocks();

private:
  int m_x;
  int m_z;
  int m_worldX;
  int m_worldZ;
  BlocksManager &m_blocksManager;
  TextureAtlas &m_textureAtlas;

  std::vector<Voxel> m_voxels;

  std::vector<ChunkVertex> m_vertices;
  std::vector<uint32_t> m_indices;
  std::unique_ptr<Mesh<ChunkVertex>> m_mesh;
};