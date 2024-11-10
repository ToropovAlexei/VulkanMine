#include "BlockLoader.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

BlockLoader::BlockLoader(std::string_view blocksPath) { loadBlocks(blocksPath); }

std::vector<Block> BlockLoader::loadBlocks(std::string_view blocksPath) {
  std::vector<Block> blocks;
  for (const auto &blockPath : std::filesystem::directory_iterator(blocksPath)) {
    blocks.push_back(loadBlock(blockPath.path()));
  }
  if (blocks.empty()) {
    throw std::runtime_error("Blocks not found");
  }

  return blocks;
}

Block BlockLoader::loadBlock(std::filesystem::path filePath) {
  std::ifstream f(filePath);
  nlohmann::json blockData = nlohmann::json::parse(f);
  std::vector<std::string> textures;
  if (blockData.contains("textures")) {
    textures = blockData["textures"];
  }
  std::string name = blockData["name"];
  BlockId id = blockData["id"];
  std::array<uint8_t, 3u> emission = {0u, 0u, 0u};
  if (blockData.contains("emission")) {
    std::vector<int> emissionVec = blockData["emission"];
    if (emissionVec.size() != 3) {
      throw std::runtime_error("Emission is incorrect in " + filePath.filename().string());
    }
    for (size_t i = 0; i < emission.size(); i++) {
      emission[i] = static_cast<uint8_t>(emissionVec[i]);
    }
  }
  bool isOpaque = true;
  if (blockData.contains("is_opaque")) {
    isOpaque = blockData["is_opaque"];
  }
  //   int drawGroup = 0;
  //   if (blockData.contains("draw_group")) {
  //     drawGroup = blockData["draw_group"];
  //   }

  return Block(id, name, textures, isOpaque);
}
