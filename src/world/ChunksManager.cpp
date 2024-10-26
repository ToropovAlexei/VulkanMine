#include "ChunksManager.hpp"
#include <algorithm>
#include <execution>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <tracy/Tracy.hpp>
#include <tuple>
#include <vector>

ChunksManager::ChunksManager(BlocksManager &blocksManager,
                             TextureAtlas &textureAtlas, int playerX,
                             int playerZ)
    : m_blocksManager{blocksManager}, m_textureAtlas{textureAtlas},
      m_worldGenerator{blocksManager, textureAtlas} {
  ZoneScoped;
  setPlayerPos(playerX, playerZ);
  m_chunks.resize(
      static_cast<size_t>(m_chunksVectorSideSize * m_chunksVectorSideSize));
  m_thread = std::thread([this]() { asyncProcessChunks(); });
}

ChunksManager::~ChunksManager() {
  m_isRunning = false;
  m_thread.join();
}

void ChunksManager::asyncProcessChunks() {
  ZoneScoped;
  while (m_isRunning) {
    loadChunks();
    moveChunks();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}

void ChunksManager::loadChunks() {
  ZoneScoped;
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  std::vector<std::future<void>> futures;

  std::vector<std::tuple<int, int>> chunksToGenerate;

  const size_t centerIdx = getCenterIdx();

  if (m_chunks[centerIdx] == nullptr) {
    chunksToGenerate.push_back({m_playerX, m_playerZ});
  }

  int radius = 1;
  while (radius <= m_loadRadius &&
         chunksToGenerate.size() < m_maxAsyncChunksLoading) {
    int xStart = m_playerX - radius;
    int xEnd = m_playerZ + radius;
    int zStart = m_playerX - radius;
    int zEnd = m_playerZ + radius;

    for (int x = xStart; x <= xEnd; x++) {
      if (chunksToGenerate.size() >= m_maxAsyncChunksLoading) {
        break;
      }
      if (!getChunkAt(x, zStart)) {
        chunksToGenerate.push_back({x, zStart});
      }
      if (!getChunkAt(x, zEnd)) {
        chunksToGenerate.push_back({x, zEnd});
      }
    }
    for (int z = zStart + 1; z <= zEnd - 1; z++) {
      if (chunksToGenerate.size() >= m_maxAsyncChunksLoading) {
        break;
      }
      if (!getChunkAt(xStart, z)) {
        chunksToGenerate.push_back({xStart, z});
      }
      if (!getChunkAt(xEnd, z)) {
        chunksToGenerate.push_back({xEnd, z});
      }
    }
    radius++;
  }

  for (const auto &chunkPos : chunksToGenerate) {
    futures.emplace_back(std::async(std::launch::async, [this, &chunkPos]() {
      auto chunk = m_worldGenerator.generateChunk(std::get<0>(chunkPos),
                                                  std::get<1>(chunkPos));
      chunk->generateVerticesAndIndices();
      this->insertChunk(chunk);
    }));
  }

  for (auto &fut : futures) {
    fut.get();
  }
}

void ChunksManager::moveChunks() {
  ZoneScoped;
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  if (m_chunkLastMovedX == m_playerX && m_chunkLastMovedZ == m_playerZ) {
    return;
  };

  m_chunkLastMovedX = m_playerX;
  m_chunkLastMovedZ = m_playerZ;
  std::vector<std::shared_ptr<Chunk>> newChunks(m_chunks.size());

  for (auto &chunk : m_chunks) {
    if (!chunk) {
      continue;
    }
    auto idx = getChunkIdx(chunk->x(), chunk->z());
    if (idx < m_chunks.size()) {
      newChunks[idx] = chunk;
    }
  }
  lock.unlock();
  std::unique_lock<std::shared_mutex> lock2(m_mutex);
  std::swap(m_chunks, newChunks);
}

void ChunksManager::setPlayerPos(int x, int z) {
  m_playerX = toChunkPos(x);
  m_playerZ = toChunkPos(z);
}