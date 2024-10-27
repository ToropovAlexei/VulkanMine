#pragma once

#include "Chunk.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

class PlayerController {
public:
  PlayerController(glm::vec3 playerPos);

  inline int getChunkX() const noexcept { return m_playerX; }
  inline int getChunkZ() const noexcept { return m_playerZ; }
  inline glm::vec3 &getPosInChunk() noexcept { return m_playerPos; }
  inline float getWorldX() const noexcept {
    return m_playerX * Chunk::CHUNK_SIZE + m_playerPos.x;
  }
  inline float getWorldZ() const noexcept {
    return m_playerZ * Chunk::CHUNK_SIZE + m_playerPos.z;
  }
  inline float getWorldY() const noexcept { return m_playerPos.y; }

  void move(glm::vec3 direction);

private:
  int m_playerX;
  int m_playerZ;
  glm::vec3 m_playerPos;
};