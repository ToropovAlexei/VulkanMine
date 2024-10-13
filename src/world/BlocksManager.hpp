#pragma once

#include "Block.hpp"
#include "BlockId.hpp"
#include <array>
#include <string>

class BlocksManager {
public:
  BlocksManager(std::string blocksPath);

  inline Block &GetBlockById(BlockId id) noexcept {
    return m_blocks[static_cast<size_t>(id)];
  };

private:
  void loadBlocks(std::string blocksPath);

private:
  std::array<Block, static_cast<size_t>(BlockId::Count)> m_blocks;
};