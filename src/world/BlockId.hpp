#pragma once

#include <cstdint>

enum class BlockId : uint16_t {
  Air = 0,
  Stone = 1,
  Grass = 2,
  Dirt = 3,
  Cobblestone = 4,
  Bedrock = 5,
  CoalOre = 6,
  DiamondOre = 7,
  GoldOre = 8,
  Gravel = 9,
  IronOre = 10,
  Obsidian = 11,
  RedstoneOre = 12,
  Glowstone = 13,
  Sand = 14,
  Water = 15,
  Leaves = 16,
  Debug = 17,
  Count
};
