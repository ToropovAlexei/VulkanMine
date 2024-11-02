#pragma once

#include "../core/Frustum.hpp"
#include "BlocksManager.hpp"
#include "Chunk.hpp"
#include "PlayerController.hpp"
#include "TextureAtlas.hpp"
#include "WorldGenerator.hpp"
#include <memory>
#include <shared_mutex>
#include <thread>
#include <vector>

class ChunksManager {
public:
  ChunksManager(BlocksManager &blocksManager, TextureAtlas &textureAtlas,
                PlayerController &playerController);
  ~ChunksManager();

  std::vector<std::shared_ptr<Chunk>> getChunksToRender(Frustum &frustum);
  void insertChunk(std::shared_ptr<Chunk> chunk);
  void forEachChunk(std::function<void(std::shared_ptr<Chunk>)> func);

private:
  inline int toChunkPos(int x) const noexcept {
    if (x >= 0) {
      return x / Chunk::CHUNK_SIZE;
    }
    return (x - Chunk::CHUNK_SIZE + 1) / Chunk::CHUNK_SIZE;
  };
  inline size_t getChunkIdx(int x, int z) const noexcept {
    return (x - m_playerController.getChunkX() + m_loadRadius) +
           (z - m_playerController.getChunkZ() + m_loadRadius) *
               m_chunksVectorSideSize;
  }
  inline size_t getCenterIdx() const noexcept {
    return m_loadRadius + m_loadRadius * m_chunksVectorSideSize;
  }
  inline std::shared_ptr<Chunk> getChunkAt(int x, int z) noexcept {
    if (x > m_playerController.getChunkX() + m_loadRadius ||
        x < m_playerController.getChunkX() - m_loadRadius ||
        z > m_playerController.getChunkZ() + m_loadRadius ||
        z < m_playerController.getChunkZ() - m_loadRadius) {
      return nullptr;
    }
    return getChunkAt(getChunkIdx(x, z));
  }
  inline std::shared_ptr<Chunk> getChunkAt(size_t idx) noexcept {
    if (idx >= m_chunks.size()) {
      return nullptr;
    }
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_chunks[idx];
  }
  inline std::array<std::shared_ptr<Chunk>, 4>
  getChunksAroundChunk(int x, int z) noexcept {
    auto leftChunk = getChunkAt(x - 1, z);
    auto rightChunk = getChunkAt(x + 1, z);
    auto frontChunk = getChunkAt(x, z - 1);
    auto backChunk = getChunkAt(x, z + 1);

    return {leftChunk, rightChunk, frontChunk, backChunk};
  }

  void asyncProcessChunks();
  void loadChunks();
  void moveChunks();
  void updateModifiedChunks();

  bool isChunkVisible(const Frustum &frustum, int x, int z);

private:
  bool m_isRunning = true;
  int m_chunkLastMovedX = 0;
  int m_chunkLastMovedZ = 0;
  int m_maxAsyncChunksLoading = 12;
  int m_maxAsyncChunksToUpdate = 12;
  int m_loadRadius = 32;
  int m_chunksVectorSideSize = m_loadRadius * 2 + 1;
  BlocksManager &m_blocksManager;
  TextureAtlas &m_textureAtlas;
  PlayerController &m_playerController;
  WorldGenerator m_worldGenerator;
  std::shared_mutex m_mutex;

  std::thread m_thread;

  std::vector<std::shared_ptr<Chunk>> m_chunks;
};