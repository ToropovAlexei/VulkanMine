#pragma once

#include "Block.hpp"
#include "BlockId.hpp"
#include "TextureAtlas.hpp"
#include <array>
#include <string_view>

class BlocksManager {
public:
  BlocksManager(std::string_view blocksPath, TextureAtlas &textureAtlas);

  inline Block &getBlockById(BlockId id) noexcept { return m_blocks[static_cast<size_t>(id)]; };

private:
  void loadBlocks(std::string_view blocksPath);

private:
  std::array<Block, static_cast<size_t>(BlockId::Count)> m_blocks;
  TextureAtlas &m_textureAtlas;
};