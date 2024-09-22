#include "Mouse.hpp"
#include "GLFW/glfw3.h"
#include <cassert>

double Mouse::currentX = 0.0;
double Mouse::currentY = 0.0;
double Mouse::lastX = 0.0;
double Mouse::lastY = 0.0;
float Mouse::scrollOffset = 0.0f;
std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> Mouse::mouseButtonStates;
std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> Mouse::mouseJustPressed;
bool Mouse::_isCursorHidden = false;

Mouse::Mouse(GLFWwindow *window) {
  glfwGetCursorPos(window, &lastX, &lastY);
  currentX = lastX;
  currentY = lastY;

  glfwSetCursorPosCallback(window, Mouse::cursorPositionCallback);
  glfwSetMouseButtonCallback(window, Mouse::mouseButtonCallback);
  glfwSetScrollCallback(window, Mouse::scrollCallback);
}

void Mouse::update() {
  mouseJustPressed.reset();
  scrollOffset = 0.0f;
  lastX = currentX;
  lastY = currentY;
}

bool Mouse::isButtonPressed(int button) const {
  return mouseButtonStates[button];
}

bool Mouse::isButtonReleased(int button) const {
  return !mouseButtonStates[button];
}

bool Mouse::isButtonJustPressed(int button) const {
  return mouseJustPressed[button];
}

void Mouse::cursorPositionCallback(GLFWwindow *window, double xpos,
                                   double ypos) {
  currentX = xpos;
  currentY = ypos;
}

void Mouse::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                int mods) {
  if (action == GLFW_PRESS) {
    mouseButtonStates.set(button);
    mouseJustPressed.set(button);
    return;
  }

  if (action == GLFW_RELEASE) {
    mouseButtonStates.reset(button);
  }
}

void Mouse::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  scrollOffset = static_cast<float>(yoffset);
}
