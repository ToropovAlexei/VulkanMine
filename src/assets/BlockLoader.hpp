#pragma once

#include "../world/Block.hpp"
#include <filesystem>
#include <string>
#include <vector>

class BlockLoader {
public:
  BlockLoader(std::string blocksPath);

  std::vector<Block> loadBlocks(std::string blocksPath);

private:
  Block loadBlock(std::filesystem::path filePath);
};