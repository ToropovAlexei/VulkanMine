#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Frustum {
public:
  void extractPlanes(const glm::mat4 &clip) {
    // Извлечение плоскостей из матрицы клиппинга (clip)
    planes[0] =
        glm::vec4(clip[0][3] + clip[0][0], clip[1][3] + clip[1][0],
                  clip[2][3] + clip[2][0], clip[3][3] + clip[3][0]); // левая
    planes[1] =
        glm::vec4(clip[0][3] - clip[0][0], clip[1][3] - clip[1][0],
                  clip[2][3] - clip[2][0], clip[3][3] - clip[3][0]); // правая
    planes[2] =
        glm::vec4(clip[0][3] + clip[0][1], clip[1][3] + clip[1][1],
                  clip[2][3] + clip[2][1], clip[3][3] + clip[3][1]); // верхняя
    planes[3] =
        glm::vec4(clip[0][3] - clip[0][1], clip[1][3] - clip[1][1],
                  clip[2][3] - clip[2][1], clip[3][3] - clip[3][1]); // нижняя
    planes[4] =
        glm::vec4(clip[0][2] + clip[0][3], clip[1][2] + clip[1][3],
                  clip[2][2] + clip[2][3], clip[3][2] + clip[3][3]); // ближняя
    planes[5] =
        glm::vec4(clip[0][3] - clip[0][2], clip[1][3] - clip[1][2],
                  clip[2][3] - clip[2][2], clip[3][3] - clip[3][2]); // дальняя

    for (int i = 0; i < 6; i++) {
      planes[i] = glm::normalize(planes[i]);
    }
  }
  const glm::vec4 *getPlanes() const { return planes; }

private:
  glm::vec4 planes[6]; // 6 плоскостей: левые, правые, верхние, нижние, ближние,
                       // дальние
};