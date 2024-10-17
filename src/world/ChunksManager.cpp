#include "ChunksManager.hpp"

ChunksManager::ChunksManager(BlocksManager &blocksManager,
                             TextureAtlas &textureAtlas, int playerX,
                             int playerZ)
    : m_blocksManager{blocksManager}, m_textureAtlas{textureAtlas},
      m_worldGenerator{blocksManager, textureAtlas} {
  setPlayerPos(playerX, playerZ);

  for (int x = -4; x <= 4; x++) {
    for (int z = -4; z <= 4; z++) {
      m_chunks.push_back(m_worldGenerator.generateChunk(x, z));
    }
  }
}

void ChunksManager::setPlayerPos(int x, int z) {
  m_playerX = toChunkPos(x);
  m_playerZ = toChunkPos(z);
}