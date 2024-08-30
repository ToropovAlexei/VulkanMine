#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
  Window(int width, int height, std::string windowName);
  ~Window();

  bool shouldClose() const { return glfwWindowShouldClose(window); }

private:
  void initWindow();

private:
  const int width;
  const int height;

  std::string windowName;
  GLFWwindow *window;
};