#include "PlayerController.hpp"
#include "Chunk.hpp"

PlayerController::PlayerController(glm::vec3 playerPos) {
  m_playerPos =
      glm::vec3(std::fmod(playerPos.x, Chunk::CHUNK_SIZE), playerPos.y,
                std::fmod(playerPos.z, Chunk::CHUNK_SIZE));
  m_playerX = m_playerPos.x / Chunk::CHUNK_SIZE;
  m_playerZ = m_playerPos.z / Chunk::CHUNK_SIZE;
}

void PlayerController::move(glm::vec3 direction) {
  m_playerPos += direction;
  int dX = m_playerPos.x / Chunk::CHUNK_SIZE;
  int dZ = m_playerPos.z / Chunk::CHUNK_SIZE;
  // TODO проверить это
  if (m_playerPos.x < 0) {
    dX--;
  }
  if (m_playerPos.z < 0) {
    dZ--;
  }
  m_playerX += dX;
  m_playerZ += dZ;

  m_playerPos.x -= dX * Chunk::CHUNK_SIZE;
  m_playerPos.z -= dZ * Chunk::CHUNK_SIZE;
}