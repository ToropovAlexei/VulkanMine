#pragma once

#include "Graphics/GfxModel.hpp"
#include "glm/fwd.hpp"
#include <memory>

struct Transform2dComponent {
  glm::vec2 translation{};
  glm::vec2 scale{1.0f, 1.0f};
  float rotation;

  glm::mat2 mat2() {
    const float s = std::sin(rotation);
    const float c = std::cos(rotation);
    glm::mat2 rotMat = glm::mat2{c, s, -s, c};
    glm::mat2 scaleMat = glm::mat2{scale.x, 0.0f, 0.0f, scale.y};
    return rotMat * scaleMat;
  }
};

class GameObject {
public:
  using id_t = unsigned int;

  GameObject(const GameObject &) = delete;
  GameObject &operator=(const GameObject &) = delete;
  GameObject(GameObject &&) = default;
  GameObject &operator=(GameObject &&) = default;

  static GameObject createGameObject() {
    static id_t currentId = 0;
    return GameObject{currentId++};
  }

  id_t getId() const { return id; }

  std::shared_ptr<GfxModel> model{};
  glm::vec3 color{};
  Transform2dComponent transform2d{};

private:
  GameObject(id_t _id) : id(_id) {}

private:
  id_t id;
};