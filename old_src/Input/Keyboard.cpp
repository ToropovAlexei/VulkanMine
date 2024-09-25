#include "Keyboard.hpp"
#include <cassert>

std::bitset<GLFW_KEY_LAST + 1> Keyboard::keyStates;
std::bitset<GLFW_KEY_LAST + 1> Keyboard::justPressed;

Keyboard::Keyboard(GLFWwindow *window) : m_window(window) {
  glfwSetKeyCallback(window, Keyboard::keyCallback);
}

void Keyboard::update() { justPressed.reset(); }

bool Keyboard::isKeyPressed(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return keyStates[key];
}

bool Keyboard::isKeyReleased(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return !keyStates[key];
}

bool Keyboard::isKeyJustPressed(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return justPressed[key];
}

void Keyboard::keyCallback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");

  if (action == GLFW_PRESS) {
    keyStates.set(key);
    justPressed.set(key);
    return;
  }

  if (action == GLFW_RELEASE) {
    keyStates.reset(key);
  }
}