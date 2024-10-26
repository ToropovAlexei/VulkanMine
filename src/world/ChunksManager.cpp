#include "ChunksManager.hpp"
#include <algorithm>
#include <execution>
#include <future>
#include <memory>
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

  std::vector<std::tuple<int, int>> chunksToGenerate;
  const int radius = 40;
  for (int x = m_playerX - radius; x <= m_playerX + radius; x++) {
    for (int z = m_playerZ - radius; z <= m_playerZ + radius; z++) {
      chunksToGenerate.emplace_back(x, z);
    }
  }

  std::vector<std::future<void>> futures;

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

void ChunksManager::setPlayerPos(int x, int z) {
  m_playerX = toChunkPos(x);
  m_playerZ = toChunkPos(z);
}