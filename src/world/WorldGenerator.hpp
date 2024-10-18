#pragma once

#include "BlocksManager.hpp"
#include "Chunk.hpp"
#include "TextureAtlas.hpp"
#include <FastNoise/FastNoise.h>

class WorldGenerator {
public:
  WorldGenerator(BlocksManager &blockManager, TextureAtlas &textureAtlas);

  std::shared_ptr<Chunk> generateChunk(int cx, int cz);

private:
  FastNoise::SmartNode<FastNoise::OpenSimplex2> heightGenNoise;
  FastNoise::SmartNode<FastNoise::FractalFBm> heightGenFBm;
  FastNoise::SmartNode<FastNoise::OpenSimplex2> tempNoise;
  FastNoise::SmartNode<> node;
  BlocksManager &m_blockManager;
  TextureAtlas &m_textureAtlas;
};
