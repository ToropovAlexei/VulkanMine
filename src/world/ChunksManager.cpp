#include "ChunksManager.hpp"
#include <algorithm>
#include <cstddef>
#include <future>
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
  ZoneScoped;
  m_playerX = toChunkPos(x);
  m_playerZ = toChunkPos(z);
}

std::vector<std::shared_ptr<Chunk>> ChunksManager::getChunksToRender() {
  ZoneScoped;
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  std::vector<std::shared_ptr<Chunk>> chunksToRender;

  const size_t centerIdx = getCenterIdx();

  auto addChunkIfValid = [&](size_t index) {
    if (m_chunks[index]) {
      chunksToRender.push_back(m_chunks[index]);
    }
  };
  addChunkIfValid(centerIdx);

  size_t radius = 1;
  while (radius <= m_loadRadius) {
    size_t offset = radius * m_chunksVectorSideSize;
    size_t topLeft = centerIdx - radius - offset;
    size_t topRight = centerIdx + radius - offset;
    size_t bottomLeft = centerIdx - radius + offset;
    size_t bottomRight = centerIdx + radius + offset;

    for (size_t i = topLeft; i <= topRight; i++) {
      addChunkIfValid(i);
    }
    for (size_t i = bottomLeft; i <= bottomRight; i++) {
      addChunkIfValid(i);
    }
    for (size_t i = topLeft + m_chunksVectorSideSize; i < bottomLeft;
         i += m_chunksVectorSideSize) {
      addChunkIfValid(i);
    }
    for (size_t i = topRight + m_chunksVectorSideSize; i < bottomRight;
         i += m_chunksVectorSideSize) {
      addChunkIfValid(i);
    }
    radius++;
  }

  return chunksToRender;
}

void ChunksManager::insertChunk(std::shared_ptr<Chunk> chunk) {
  ZoneScoped;
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  auto idx = getChunkIdx(chunk->x(), chunk->z());
  if (idx >= m_chunks.size()) {
    return;
  }
  // Может быть тут баг, т.к. используется shared lock
  m_chunks[idx] = chunk;
}

void ChunksManager::forEachChunk(
    std::function<void(std::shared_ptr<Chunk>)> func) {
  ZoneScoped;
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  for (auto &chunk : m_chunks) {
    if (chunk) {
      func(chunk);
    }
  }
}
