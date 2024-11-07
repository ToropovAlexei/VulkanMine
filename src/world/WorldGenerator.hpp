#pragma once

#include "BlocksManager.hpp"
#include "Chunk.hpp"
#include <FastNoise/FastNoise.h>

class WorldGenerator {
public:
  WorldGenerator(BlocksManager &blockManager);

  std::shared_ptr<Chunk> generateChunk(int cx, int cz);

private:
  FastNoise::SmartNode<FastNoise::OpenSimplex2> heightGenNoise;
  FastNoise::SmartNode<FastNoise::FractalFBm> heightGenFBm;
  FastNoise::SmartNode<FastNoise::OpenSimplex2> tempNoise;
  FastNoise::SmartNode<> node;
  BlocksManager &m_blockManager;
};
