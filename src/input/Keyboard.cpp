#include "Keyboard.hpp"
#include <cassert>

std::bitset<GLFW_KEY_LAST + 1> Keyboard::keyStates;
std::bitset<GLFW_KEY_LAST + 1> Keyboard::justPressed;

Keyboard::Keyboard(GLFWwindow *window) {
  glfwSetKeyCallback(window, Keyboard::keyCallback);
}

void Keyboard::update() { justPressed.reset(); }

bool Keyboard::isKeyPressed(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return keyStates[static_cast<size_t>(key)];
}

bool Keyboard::isKeyReleased(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return !keyStates[static_cast<size_t>(key)];
}

bool Keyboard::isKeyJustPressed(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return justPressed[static_cast<size_t>(key)];
}

void Keyboard::keyCallback([[maybe_unused]] GLFWwindow *window, int key,
                           [[maybe_unused]] int scancode, int action,
                           [[maybe_unused]] int mods) {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");

  if (action == GLFW_PRESS) {
    keyStates.set(static_cast<size_t>(key));
    justPressed.set(static_cast<size_t>(key));
    return;
  }

  if (action == GLFW_RELEASE) {
    keyStates.reset(static_cast<size_t>(key));
  }
}