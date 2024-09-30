#pragma once

#include "GLFW/glfw3.h"
#include <bitset>

class Mouse {
public:
  Mouse(GLFWwindow *window);

  void update();
  bool isLeftButtonPressed() const {
    return mouseButtonStates[GLFW_MOUSE_BUTTON_LEFT];
  }
  bool isRightButtonPressed() const {
    return mouseButtonStates[GLFW_MOUSE_BUTTON_RIGHT];
  }
  bool isMiddleButtonPressed() const {
    return mouseButtonStates[GLFW_MOUSE_BUTTON_MIDDLE];
  }
  bool isLeftButtonReleased() const {
    return !mouseButtonStates[GLFW_MOUSE_BUTTON_LEFT];
  }
  bool isRightButtonReleased() const {
    return !mouseButtonStates[GLFW_MOUSE_BUTTON_RIGHT];
  }
  bool isMiddleButtonReleased() const {
    return !mouseButtonStates[GLFW_MOUSE_BUTTON_MIDDLE];
  }
  bool isButtonPressed(int button) const;
  bool isButtonReleased(int button) const;
  bool isButtonJustPressed(int button) const;
  double getDeltaX() const { return currentX - lastX; }
  double getDeltaY() const { return currentY - lastY; }
  double getX() const { return currentX; }
  double getY() const { return currentY; }
  float getScrollOffset() const { return scrollOffset; }

private:
  static void cursorPositionCallback(GLFWwindow *window, double xpos,
                                     double ypos);
  static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void scrollCallback(GLFWwindow *window, double xoffset,
                             double yoffset);

private:
  GLFWwindow *m_window;

  static double currentX;
  static double currentY;
  static double lastX;
  static double lastY;
  static float scrollOffset;
  static std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> mouseButtonStates;
  static std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> mouseJustPressed;
};