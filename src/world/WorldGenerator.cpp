#include "WorldGenerator.hpp"
#include <algorithm>
#include <memory>

WorldGenerator::WorldGenerator(BlocksManager &blockManager) : m_blockManager{blockManager} {
  ZoneScoped;
  heightGenNoise = FastNoise::New<FastNoise::OpenSimplex2>();
  heightGenFBm = FastNoise::New<FastNoise::FractalFBm>();
  heightGenFBm->SetSource(heightGenNoise);
  heightGenFBm->SetOctaveCount(2);
  tempNoise = FastNoise::New<FastNoise::OpenSimplex2>();
  node = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/"
                                           "DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                                           "AAAAAM3MTD4AMzMzPwAAAAA/");
}

std::shared_ptr<Chunk> WorldGenerator::generateChunk(int cx, int cz) {
  ZoneScoped;
  auto chunk = new Chunk(m_blockManager, cx, cz);
  std::vector<float> heights(Chunk::CHUNK_SQ_SIZE);
  std::vector<float> temps(Chunk::CHUNK_SQ_SIZE);
  heightGenFBm->GenUniformGrid2D(heights.data(), cx * Chunk::CHUNK_SIZE, cz * Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE,
                                 Chunk::CHUNK_SIZE, 0.005f, 1337);
  tempNoise->GenUniformGrid2D(temps.data(), cx * Chunk::CHUNK_SIZE, cz * Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE,
                              Chunk::CHUNK_SIZE, 0.002f, 1337);

  const int maxHeight = 128;
  const int minHeight = 45;
  const int waterLevel = 62;

  const int maxHeightInChunk =
      static_cast<int>((*std::max_element(heights.begin(), heights.end()) + 1.0f) * (maxHeight - minHeight) / 2.0f) +
      minHeight;

  chunk->m_maxY = std::max(maxHeightInChunk, waterLevel);
  chunk->m_voxels.resize(chunk->m_maxY * Chunk::CHUNK_SQ_SIZE);

  size_t idx = 0;
  for (int z = 0; z < Chunk::CHUNK_SIZE; ++z) {
    for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
      size_t activeIdx = idx++;
      // float caveNoise = caveNoiseGen.GetNoise(static_cast<float>(cx *
      // Chunk::WIDTH + x) / Chunk::WIDTH, static_cast<float>(cz * Chunk::WIDTH
      // + z) / Chunk::WIDTH);

      const int height = static_cast<int>((heights[activeIdx] + 1.0f) * (maxHeight - minHeight) / 2.0f) + minHeight;
      size_t blockIdx = x + z * Chunk::CHUNK_SIZE;
      for (int y = 0; y < std::max(height, waterLevel); ++y) {
        size_t currentBlockIdx = blockIdx;
        blockIdx += Chunk::CHUNK_SQ_SIZE;
        if (y > height) {
          chunk->m_voxels[currentBlockIdx].blockId = BlockId::Water;
          continue;
        }
        chunk->m_voxels[currentBlockIdx].blockId = temps[activeIdx] > 0.5 ? BlockId::CoalOre : BlockId::Grass;
        if (y == 0) {
          chunk->m_voxels[currentBlockIdx].blockId = BlockId::Bedrock;
        } else if (y == height - 1) {
          chunk->m_voxels[currentBlockIdx].blockId = BlockId::Grass;
        } else if (y > height - 4) {
          chunk->m_voxels[currentBlockIdx].blockId = BlockId::Dirt;
        } else {
          chunk->m_voxels[currentBlockIdx].blockId = BlockId::Stone;
        }
      }
    }
  }

  return std::shared_ptr<Chunk>(chunk);
}
