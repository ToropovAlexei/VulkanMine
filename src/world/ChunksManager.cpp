#include "ChunksManager.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <tracy/Tracy.hpp>
#include <tuple>
#include <vector>

ChunksManager::ChunksManager(BlocksManager &blocksManager,
                             TextureAtlas &textureAtlas,
                             PlayerController &playerController)
    : m_blocksManager{blocksManager}, m_textureAtlas{textureAtlas},
      m_worldGenerator{blocksManager, textureAtlas},
      m_playerController{playerController} {
  ZoneScoped;
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
    moveChunks();
    loadChunks();

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}

void ChunksManager::loadChunks() {
  ZoneScoped;
  std::vector<std::future<void>> futures;

  std::vector<std::tuple<int, int>> chunksToGenerate;

  const size_t centerIdx = getCenterIdx();
  const int playerX = m_playerController.getChunkX();
  const int playerZ = m_playerController.getChunkZ();

  if (m_chunks[centerIdx] == nullptr) {
    chunksToGenerate.push_back({playerX, playerZ});
  }

  int radius = 1;
  while (radius <= m_loadRadius &&
         chunksToGenerate.size() < m_maxAsyncChunksLoading) {
    int xStart = playerX - radius;
    int xEnd = playerX + radius;
    int zStart = playerZ - radius;
    int zEnd = playerZ + radius;

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
      const auto x = std::get<0>(chunkPos);
      const auto z = std::get<1>(chunkPos);
      auto chunk = m_worldGenerator.generateChunk(x, z);
      const size_t idx = getChunkIdx(x, z);
      auto leftChunk = getChunkAt(idx - 1);
      auto rightChunk = getChunkAt(idx + 1);
      auto frontChunk = getChunkAt(idx - m_chunksVectorSideSize);
      auto backChunk = getChunkAt(idx + m_chunksVectorSideSize);
      chunk->generateVerticesAndIndices(frontChunk, backChunk, leftChunk,
                                        rightChunk);
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
  const int playerX = m_playerController.getChunkX();
  const int playerZ = m_playerController.getChunkZ();
  if (m_chunkLastMovedX == playerX && m_chunkLastMovedZ == playerZ) {
    return;
  };

  m_chunkLastMovedX = playerX;
  m_chunkLastMovedZ = playerZ;
  std::vector<std::shared_ptr<Chunk>> newChunks(m_chunks.size());

  for (auto &chunk : m_chunks) {
    if (!chunk) {
      continue;
    }
    auto x = chunk->x();
    auto z = chunk->z();
    if (x < playerX - m_loadRadius || x > playerX + m_loadRadius ||
        z < playerZ - m_loadRadius || z > playerZ + m_loadRadius) {
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

bool ChunksManager::isChunkVisible(const Frustum &frustum, int x, int z) {
  const glm::vec4 *planes = frustum.getPlanes();
  const int playerX = m_playerController.getChunkX();
  const int playerZ = m_playerController.getChunkZ();

  // Определяем углы чанка
  glm::vec3 min = glm::vec3((x - playerX) * Chunk::CHUNK_SIZE, 0,
                            (z - playerZ) * Chunk::CHUNK_SIZE);
  glm::vec3 max = min + glm::vec3(Chunk::CHUNK_SIZE, Chunk::CHUNK_HEIGHT,
                                  Chunk::CHUNK_SIZE);

  for (int i = 0; i < 6; ++i) {
    // Проверяем плоскость
    if (planes[i].x * min.x + planes[i].y * min.y + planes[i].z * min.z +
            planes[i].w >
        0)
      continue;
    if (planes[i].x * max.x + planes[i].y * min.y + planes[i].z * min.z +
            planes[i].w >
        0)
      continue;
    if (planes[i].x * min.x + planes[i].y * max.y + planes[i].z * min.z +
            planes[i].w >
        0)
      continue;
    if (planes[i].x * max.x + planes[i].y * max.y + planes[i].z * min.z +
            planes[i].w >
        0)
      continue;
    if (planes[i].x * min.x + planes[i].y * min.y + planes[i].z * max.z +
            planes[i].w >
        0)
      continue;
    if (planes[i].x * max.x + planes[i].y * min.y + planes[i].z * max.z +
            planes[i].w >
        0)
      continue;
    if (planes[i].x * min.x + planes[i].y * max.y + planes[i].z * max.z +
            planes[i].w >
        0)
      continue;
    if (planes[i].x * max.x + planes[i].y * max.y + planes[i].z * max.z +
            planes[i].w >
        0)
      continue;

    // Если чанк полностью за плоскостью, он невидим
    return false;
  }
  return true; // Чанк видим
}

std::vector<std::shared_ptr<Chunk>>
ChunksManager::getChunksToRender(Frustum &frustum) {
  ZoneScoped;
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  std::vector<std::shared_ptr<Chunk>> chunksToRender;
  chunksToRender.reserve(m_chunks.size() / 2);

  const size_t centerIdx = getCenterIdx();

  auto addChunkIfValid = [&](size_t index) {
    ZoneScopedN("addChunkIfValid");
    if (m_chunks[index] &&
        isChunkVisible(frustum, m_chunks[index]->x(), m_chunks[index]->z())) {
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
  auto x = chunk->x();
  auto z = chunk->z();
  const int playerX = m_playerController.getChunkX();
  const int playerZ = m_playerController.getChunkZ();
  if (x < playerX - m_loadRadius || x > playerX + m_loadRadius ||
      z < playerZ - m_loadRadius || z > playerZ + m_loadRadius) {
    return;
  }
  auto idx = getChunkIdx(chunk->x(), chunk->z());
  if (idx >= m_chunks.size()) {
    return;
  }
  std::unique_lock<std::shared_mutex> lock(m_mutex);
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
