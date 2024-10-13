#pragma once

#include "BlockId.hpp"

class Voxel {
public:
  Voxel() = default;
  Voxel(BlockId blockId) : blockId{blockId} {}

  BlockId blockId;
};