#include "BlocksManager.hpp"
#include "../assets/BlockLoader.hpp"

BlocksManager::BlocksManager(std::string_view blocksPath, TextureAtlas &textureAtlas) : m_textureAtlas{textureAtlas} {
  loadBlocks(blocksPath);
}

void BlocksManager::loadBlocks(std::string_view blocksPath) {
  BlockLoader loader(blocksPath);
  auto loadedBlocks = loader.loadBlocks(blocksPath);

  for (auto &block : loadedBlocks) {
    if (block.id() == BlockId::Air) {
      continue;
    }
    block.setTexturesIndices(m_textureAtlas.getTextureIdx(block.getFaceTextureName(Block::Faces::Front)),
                             m_textureAtlas.getTextureIdx(block.getFaceTextureName(Block::Faces::Back)),
                             m_textureAtlas.getTextureIdx(block.getFaceTextureName(Block::Faces::Top)),
                             m_textureAtlas.getTextureIdx(block.getFaceTextureName(Block::Faces::Bottom)),
                             m_textureAtlas.getTextureIdx(block.getFaceTextureName(Block::Faces::Left)),
                             m_textureAtlas.getTextureIdx(block.getFaceTextureName(Block::Faces::Right)));
    m_blocks[static_cast<size_t>(block.id())] = block;
  }
}
