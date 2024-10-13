#include "TextureAtlas.hpp"
#include <filesystem>

TextureAtlas::TextureAtlas(RenderDeviceVk *renderDevice,
                           std::string texturesPath)
    : m_texture{renderDevice, getTexturesPaths(texturesPath)} {
  auto textures = getTexturesNames(texturesPath);

  for (size_t i = 0; i < textures.size(); i++) {
    m_texturesIndices[textures[i]] = static_cast<float>(i);
  }
}

std::vector<std::string>
TextureAtlas::getTexturesPaths(std::string texturesPath) {
  std::vector<std::string> textures;
  for (const auto &texturePath :
       std::filesystem::directory_iterator(texturesPath)) {
    textures.push_back(texturePath.path().string());
  }
  if (textures.empty()) {
    throw std::runtime_error("Textures not found in " + texturesPath + " path");
  }
  return textures;
}

std::vector<std::string>
TextureAtlas::getTexturesNames(std::string texturesPath) {
  std::vector<std::string> textures;
  for (const auto &texturePath :
       std::filesystem::directory_iterator(texturesPath)) {
    textures.push_back(texturePath.path().stem().string());
  }
  if (textures.empty()) {
    throw std::runtime_error("Textures not found in " + texturesPath + " path");
  }
  return textures;
}