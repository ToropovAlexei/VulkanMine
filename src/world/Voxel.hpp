#pragma once

#include "BlockId.hpp"

class Voxel {
public:
  Voxel(BlockId blockId) : blockId{blockId} {}

  BlockId blockId;
};