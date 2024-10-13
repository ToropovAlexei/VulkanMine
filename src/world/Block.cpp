#include "Block.hpp"
#include <stdexcept>

void Block::buildTextures(std::vector<std::string> textures) {
  if (textures.empty()) {
    return;
  }
  if (textures.size() == 1) {
    for (size_t i = 0; i < m_textures.size(); i++) {
      m_textures[i] = textures[0];
    }
    return;
  }
  if (textures.size() == 3) {
    m_textures[static_cast<size_t>(Faces::Front)] = textures[1];
    m_textures[static_cast<size_t>(Faces::Back)] = textures[1];
    m_textures[static_cast<size_t>(Faces::Top)] = textures[0];
    m_textures[static_cast<size_t>(Faces::Bottom)] = textures[2];
    m_textures[static_cast<size_t>(Faces::Left)] = textures[1];
    m_textures[static_cast<size_t>(Faces::Right)] = textures[1];
    return;
  }
  if (textures.size() == 6) {
    m_textures[static_cast<size_t>(Faces::Front)] = textures[0];
    m_textures[static_cast<size_t>(Faces::Back)] = textures[1];
    m_textures[static_cast<size_t>(Faces::Top)] = textures[2];
    m_textures[static_cast<size_t>(Faces::Bottom)] = textures[3];
    m_textures[static_cast<size_t>(Faces::Left)] = textures[4];
    m_textures[static_cast<size_t>(Faces::Right)] = textures[5];
    return;
  }
  throw std::runtime_error("Failed to build textures for block " + m_name);
}

Block::Block(BlockId id, std::string name, std::vector<std::string> textures,
             bool isOpaque)
    : m_id{id}, m_name{name}, m_isOpaque{isOpaque} {
  buildTextures(textures);
}
