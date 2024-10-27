#pragma once

#include "BlockId.hpp"
#include <array>
#include <string>
#include <vector>

class Block {
public:
  enum class Faces { Front, Back, Top, Bottom, Left, Right, Count };

public:
  Block() = default;
  Block(BlockId id, std::string name, std::vector<std::string> textures,
        bool isOpaque);
  inline BlockId id() const noexcept { return m_id; }

  inline std::string &getFaceTexture(Faces face) noexcept {
    return m_textures[static_cast<size_t>(face)];
  };
  inline bool isOpaque() const noexcept { return m_isOpaque; }

private:
  void buildTextures(std::vector<std::string> textures);

private:
  BlockId m_id;
  std::string m_name;
  bool m_isOpaque;
  std::array<std::string, static_cast<size_t>(Faces::Count)> m_textures;
};