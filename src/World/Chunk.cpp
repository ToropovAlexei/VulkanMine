#include "Chunk.hpp"
#include <memory>

struct Voxel {
  bool isSolid = false;
};

// Размер чанка
const int CHUNK_SIZE = 16;

// Глобальные переменные для упрощения: позиции граней куба и UV-координаты
// для каждой грани
const glm::vec3 cubeVertices[8] = {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
                                   {0.5f, 0.5f, -0.5f},   {-0.5f, 0.5f, -0.5f},
                                   {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},
                                   {0.5f, 0.5f, 0.5f},    {-0.5f, 0.5f, 0.5f}};

// Индексы для граней куба (шестиугольные стороны)
const int cubeIndices[6][4] = {
    {0, 1, 2, 3}, // Задняя грань
    {4, 5, 6, 7}, // Передняя грань
    {0, 1, 5, 4}, // Нижняя грань
    {2, 3, 7, 6}, // Верхняя грань
    {0, 3, 7, 4}, // Левая грань
    {1, 2, 6, 5}  // Правая грань
};

// Текстурные координаты для куба
const glm::vec2 uvCoords[4] = {
    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

// Проверяем, существует ли воксель на указанной позиции в чанке и является ли
// он твердым
bool isVoxelSolid(const std::vector<std::vector<std::vector<Voxel>>> &chunk,
                  int x, int y, int z) {
  if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 ||
      z >= CHUNK_SIZE) {
    return false; // Границы чанка
  }
  return chunk[x][y][z].isSolid;
}

// Генерируем вершины и индексы для видимых граней кубов в случайном чанке
void generateChunk(std::vector<GfxModel::Vertex> &vertices,
                   std::vector<uint32_t> &indices) {
  // Инициализируем случайный чанк
  std::vector<std::vector<std::vector<Voxel>>> chunk(
      CHUNK_SIZE, std::vector<std::vector<Voxel>>(
                      CHUNK_SIZE, std::vector<Voxel>(CHUNK_SIZE)));

  // Генерируем случайные воксели (например, 50% шанс, что воксель будет
  // твердым)
  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        chunk[x][y][z].isSolid = rand() % 2 == 0;
      }
    }
  }

  // Перебираем все воксели и добавляем видимые грани
  uint32_t indexOffset = 0;
  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        if (!chunk[x][y][z].isSolid) {
          continue; // Если воксель пустой, пропускаем его
        }

        // Проверяем каждую грань куба (шесть сторон)
        for (int face = 0; face < 6; ++face) {
          // Если соседний воксель закрывает грань, не рендерим ее
          int nx = x + (face == 1 ? 1 : (face == 0 ? -1 : 0));
          int ny = y + (face == 3 ? 1 : (face == 2 ? -1 : 0));
          int nz = z + (face == 5 ? 1 : (face == 4 ? -1 : 0));
          if (isVoxelSolid(chunk, nx, ny, nz)) {
            continue; // Грань закрыта
          }

          // Если грань видима, добавляем вершины и индексы для нее
          for (int i = 0; i < 4; ++i) {
            GfxModel::Vertex vertex;
            vertex.pos =
                glm::vec3(x, y, z) + cubeVertices[cubeIndices[face][i]];
            vertex.color =
                glm::vec3(rand() % 4 * 0.25f, rand() % 4 * 0.25f,
                          rand() % 4 * 0.25f); // Белый цвет для всех вершин
            vertex.uv = uvCoords[i];
            vertices.push_back(vertex);
          }

          // Индексы для текущей грани
          indices.push_back(indexOffset + 0);
          indices.push_back(indexOffset + 1);
          indices.push_back(indexOffset + 2);
          indices.push_back(indexOffset + 2);
          indices.push_back(indexOffset + 3);
          indices.push_back(indexOffset + 0);

          // Сдвигаем индекс для следующей грани
          indexOffset += 4;
        }
      }
    }
  }
}

std::shared_ptr<GfxModel> Chunk::getTestChunkModel(GfxDevice &gfxDevice) {
  std::vector<GfxModel::Vertex> vertices;
  std::vector<uint32_t> indices;
  generateChunk(vertices, indices);
  return std::make_shared<GfxModel>(gfxDevice, vertices, indices);
}
