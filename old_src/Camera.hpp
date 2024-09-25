#pragma once

#include <glm/glm.hpp>

class Camera {
public:
  void setOrthographicProjection(float left, float right, float bottom,
                                 float top, float near, float far);

  void setPerspectiveProjection(float fovy, float aspect, float near,
                                float far);

  void setViewDirection(glm::vec3 position, glm::vec3 direction,
                        glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
  void setViewTarget(glm::vec3 position, glm::vec3 target,
                     glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
  void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

  const glm::mat4 &getProjection() const { return projection; }
  const glm::mat4 &getView() const { return view; }

private:
  glm::mat4 projection{1.0f};
  glm::mat4 view{1.0f};
};