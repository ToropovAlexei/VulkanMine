#pragma once

#include "BlocksManager.hpp"
#include "Chunk.hpp"
#include "TextureAtlas.hpp"
#include "WorldGenerator.hpp"
#include <memory>
#include <mutex>
#include <vector>

class ChunksManager {
public:
  ChunksManager(BlocksManager &blocksManager, TextureAtlas &textureAtlas,
                int playerX, int playerZ);

  void setPlayerPos(int x, int z);
  std::vector<std::shared_ptr<Chunk>> &getChunks() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_chunks;
  }
  std::vector<std::shared_ptr<Chunk>> getChunksToRender() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_chunks;
  }
  void insertChunk(std::shared_ptr<Chunk> chunk) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_chunks.emplace_back(chunk);
  }

private:
  inline int toChunkPos(int x) const noexcept {
    if (x >= 0) {
      return x / Chunk::CHUNK_SIZE;
    }
    return (x - Chunk::CHUNK_SIZE + 1) / Chunk::CHUNK_SIZE;
  };

private:
  int m_playerX = 0;
  int m_playerZ = 0;
  BlocksManager &m_blocksManager;
  TextureAtlas &m_textureAtlas;
  WorldGenerator m_worldGenerator;
  std::mutex m_mutex;

  std::vector<std::shared_ptr<Chunk>> m_chunks;
};