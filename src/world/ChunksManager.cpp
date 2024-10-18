#include "ChunksManager.hpp"
#include <Tracy/tracy/Tracy.hpp>
#include <algorithm>
#include <execution>
#include <memory>
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
  for (int x = m_playerX - 16; x <= m_playerX + 16; x++) {
    for (int z = m_playerZ - 16; z <= m_playerZ + 16; z++) {
      chunksToGenerate.emplace_back(x, z);
    }
  }

  std::for_each(chunksToGenerate.begin(), chunksToGenerate.end(),
                [this](std::tuple<int, int> &chunkPos) {
                  auto chunk = m_worldGenerator.generateChunk(
                      std::get<0>(chunkPos), std::get<1>(chunkPos));
                  chunk->generateVerticesAndIndices();
                  this->insertChunk(chunk);
                });
  // m_taskflow.for_each(chunksToGenerate.begin(), chunksToGenerate.end(),
  //                     [this](std::tuple<int, int> &chunkPos) {
  //                       this->insertChunk(m_worldGenerator.generateChunk(
  //                           std::get<0>(chunkPos), std::get<1>(chunkPos)));
  //                     });

  // m_executor.run(m_taskflow).wait();
}

void ChunksManager::setPlayerPos(int x, int z) {
  m_playerX = toChunkPos(x);
  m_playerZ = toChunkPos(z);
}