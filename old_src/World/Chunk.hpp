#pragma once

#include "../Graphics/GfxModel.hpp"

class Chunk {
public:
  static std::shared_ptr<GfxModel> getTestChunkModel(GfxDevice &gfxDevice);

public:
  int m_x;
  int m_z;
  std::shared_ptr<GfxModel> m_model;
};