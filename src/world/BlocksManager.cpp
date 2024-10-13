#include "BlocksManager.hpp"
#include "../assets/BlockLoader.hpp"

BlocksManager::BlocksManager(std::string blocksPath) { loadBlocks(blocksPath); }

void BlocksManager::loadBlocks(std::string blocksPath) {
  BlockLoader loader(blocksPath);
  auto loadedBlocks = loader.loadBlocks(blocksPath);

  for (auto &block : loadedBlocks) {
    m_blocks[static_cast<size_t>(block.id())] = block;
  }
}
