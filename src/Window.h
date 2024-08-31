#pragma once

#define GLFW_INCLUDE_VULKAN
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

private:
  void initWindow();

private:
  const int width;
  const int height;

  std::string windowName;
  GLFWwindow *window;
};