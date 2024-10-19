#include "ChunksManager.hpp"
#include <tracy/Tracy.hpp>
#include <algorithm>
#include <execution>
#include <memory>
#include <tuple>
#include <vector>
#include <future>

ChunksManager::ChunksManager(BlocksManager &blocksManager,
                             TextureAtlas &textureAtlas, int playerX,
                             int playerZ)
    : m_blocksManager{blocksManager}, m_textureAtlas{textureAtlas},
      m_worldGenerator{blocksManager, textureAtlas} {
  ZoneScoped;
  setPlayerPos(playerX, playerZ);


  std::async(std::launch::async, [this]() {
    std::vector<std::tuple<int, int>> chunksToGenerate;
    for (int x = m_playerX - 24; x <= m_playerX + 24; x++) {
      for (int z = m_playerZ - 24; z <= m_playerZ + 24; z++) {
        chunksToGenerate.emplace_back(x, z);
      }
    }

    std::for_each(std::execution::par, chunksToGenerate.begin(), chunksToGenerate.end(),
      [this](std::tuple<int, int>& chunkPos) {
        auto chunk = m_worldGenerator.generateChunk(
          std::get<0>(chunkPos), std::get<1>(chunkPos));
        chunk->generateVerticesAndIndices();
        this->insertChunk(chunk);
      });
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