#include "Keyboard.hpp"
#include <cassert>

std::bitset<GLFW_KEY_LAST + 1> Keyboard::m_keyStates;
std::bitset<GLFW_KEY_LAST + 1> Keyboard::m_justPressed;

Keyboard::Keyboard(GLFWwindow *window) : m_window(window) {
  glfwSetKeyCallback(window, Keyboard::keyCallback);
}

void Keyboard::update() { m_justPressed.reset(); }

bool Keyboard::isKeyPressed(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return m_keyStates.test(key);
}

bool Keyboard::isKeyReleased(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return !m_keyStates.test(key);
}

bool Keyboard::isKeyJustPressed(int key) const {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");
  return m_justPressed.test(key);
}

void Keyboard::keyCallback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
  assert(key >= 0 && key <= GLFW_KEY_LAST && "Invalid keyboard key");

  if (action == GLFW_PRESS) {
    m_keyStates.set(key);
    m_justPressed.set(key);
    return;
  }

  if (action == GLFW_RELEASE) {
    m_keyStates.reset(key);
  }
}