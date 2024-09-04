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