#pragma once

#include "../utils/NonCopyable.hpp"
#include <GLFW/glfw3.h>
#include <bitset>

class Keyboard : NonCopyable {
public:
  Keyboard(GLFWwindow *window);

  void update();
  bool isKeyPressed(int key) const;
  bool isKeyReleased(int key) const;
  bool isKeyJustPressed(int key) const;

private:
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

private:
  static std::bitset<GLFW_KEY_LAST + 1> keyStates;
  static std::bitset<GLFW_KEY_LAST + 1> justPressed;

  GLFWwindow *m_window;
};