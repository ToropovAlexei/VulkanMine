#pragma once

#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
  Window(int width, int height, std::string windowName);
  ~Window();

  bool shouldClose() const { return glfwWindowShouldClose(window); }
  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
  VkExtent2D getExtent() const {
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }
  void hideCursor() {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  void showCursor() {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  GLFWwindow *getGLFWwindow() const { return window; }

private:
  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height);
  void initWindow();

private:
  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;
  GLFWwindow *window;
};