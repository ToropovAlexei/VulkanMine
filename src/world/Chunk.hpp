#pragma once

#include "../renderSystems/ChunkVertex.hpp"
#include "../renderer/Mesh.hpp"
#include "Voxel.hpp"
#include <memory>
#include <vector>

class Chunk {
public:
  Chunk(int x, int z);

  inline int x() const noexcept { return m_x; }
  inline int z() const noexcept { return m_z; }

  std::unique_ptr<Mesh<ChunkVertex>> &getMesh() { return m_mesh; }
  void generateMesh(RenderDeviceVk *device);

public:
  static constexpr int CHUNK_SIZE = 16;
  static constexpr int CHUNK_HEIGHT = 256;
  static constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;

private:
  void addFrontFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                    std::vector<uint32_t> &indices);
  void addBackFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                   std::vector<uint32_t> &indices);
  void addLeftFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                   std::vector<uint32_t> &indices);
  void addRightFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                    std::vector<uint32_t> &indices);
  void addTopFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                  std::vector<uint32_t> &indices);
  void addBottomFace(int x, int y, int z, std::vector<ChunkVertex> &vertices,
                     std::vector<uint32_t> &indices);

private:
  int m_x;
  int m_z;

  std::vector<Voxel> m_voxels;
  std::unique_ptr<Mesh<ChunkVertex>> m_mesh;
};