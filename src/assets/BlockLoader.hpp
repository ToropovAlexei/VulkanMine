#pragma once

#include "../world/Block.hpp"
#include <filesystem>
#include <vector>

class BlockLoader {
public:
  BlockLoader(std::string_view blocksPath);

  std::vector<Block> loadBlocks(std::string_view blocksPath);

private:
  Block loadBlock(std::filesystem::path filePath);
};