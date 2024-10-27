#pragma once

#include "../renderer/backend/TextureVk.hpp"
#include <string>
#include <tracy/Tracy.hpp>
#include <unordered_map>
#include <vector>

class TextureAtlas {
public:
  TextureAtlas(RenderDeviceVk *renderDevice, std::string texturesPath);

  inline float getTextureIdx(const std::string &textureName) const noexcept {
    ZoneScoped;
    return m_texturesIndices.find(textureName)->second;
  }

  inline TextureVk &getTexture() noexcept { return m_texture; }

private:
  static std::vector<std::string> getTexturesPaths(std::string texturesPath);

  std::vector<std::string> getTexturesNames(std::string texturesPath);

private:
  std::unordered_map<std::string, float> m_texturesIndices;
  TextureVk m_texture;
};