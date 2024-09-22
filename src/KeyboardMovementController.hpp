#pragma once

#include "GameObject.hpp"
#include "Input/Keyboard.hpp"
#include "Input/Mouse.hpp"
#include <GLFW/glfw3.h>

class KeyboardMovementController {
public:
  struct KeyMappings {
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp = GLFW_KEY_SPACE;
    int moveDown = GLFW_KEY_X;
    int lookLeft = GLFW_KEY_LEFT;
    int lookRight = GLFW_KEY_RIGHT;
    int lookUp = GLFW_KEY_UP;
    int lookDown = GLFW_KEY_DOWN;
  };

  void moveInPlaneXZ(std::unique_ptr<Keyboard> &kbd,
                     std::unique_ptr<Mouse> &mouse, float dt,
                     GameObject &gameObject);

public:
  KeyMappings keys{};
  float moveSpeed = 3.0f;
  float lookSpeed = 25.0f;
};