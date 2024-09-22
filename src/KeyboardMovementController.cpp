#include "KeyboardMovementController.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/constants.hpp"
#include <limits>

void KeyboardMovementController::moveInPlaneXZ(Keyboard kbd, float dt,
                                               GameObject &gameObject) {
  glm::vec3 rotate{0};

  if (kbd.isKeyPressed(keys.lookRight)) {
    rotate.y += 1.0f;
  }
  if (kbd.isKeyPressed(keys.lookLeft)) {
    rotate.y -= 1.0f;
  }
  if (kbd.isKeyPressed(keys.lookUp)) {
    rotate.x += 1.0f;
  }
  if (kbd.isKeyPressed(keys.lookDown)) {
    rotate.x -= 1.0f;
  }

  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
  }

  gameObject.transform.rotation.x =
      glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
  gameObject.transform.rotation.y =
      glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

  float yaw = gameObject.transform.rotation.y;
  const glm::vec3 forwardDir{glm::sin(yaw), 0.f, glm::cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
  const glm::vec3 upDir{0.f, -1.f, 0.f};

  glm::vec3 moveDir{0};
  if (kbd.isKeyPressed(keys.moveForward)) {
    moveDir += forwardDir;
  }
  if (kbd.isKeyPressed(keys.moveBackward)) {
    moveDir -= forwardDir;
  }
  if (kbd.isKeyPressed(keys.moveRight)) {
    moveDir += rightDir;
  }
  if (kbd.isKeyPressed(keys.moveLeft)) {
    moveDir -= rightDir;
  }
  if (kbd.isKeyPressed(keys.moveUp)) {
    moveDir += upDir;
  }
  if (kbd.isKeyPressed(keys.moveDown)) {
    moveDir -= upDir;
  }

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation +=
        moveSpeed * dt * glm::normalize(moveDir);
  }
}