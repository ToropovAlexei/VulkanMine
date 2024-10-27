#pragma once

#include "BlockId.hpp"

class Voxel {
public:
  Voxel() : blockId{BlockId::Air} {};
  Voxel(BlockId blockId) : blockId{blockId} {}

  BlockId blockId;
};