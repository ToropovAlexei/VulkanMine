#pragma once

#include "GLFW/glfw3.h"
#include <string>
#include <vulkan/vulkan.hpp>

class Window {
public:
  Window(int width, int height, std::string windowName);
  ~Window();

  GLFWwindow *getGLFWwindow() const noexcept { return m_window; }
  bool wasWindowResized() const noexcept { return m_framebufferResized; }
  void resetWindowResizedFlag() noexcept { m_framebufferResized = false; }
  vk::Extent2D getExtent() const noexcept {
    return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
  }
  bool shouldClose() const { return glfwWindowShouldClose(m_window); }
  void createWindowSurface(vk::Instance& instance, vk::SurfaceKHR &surface);
  void hideCursor() {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  void showCursor() {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }
  void pollEvents() { glfwPollEvents(); }

private:
  void initWindow();
  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height);

private:
  int m_width;
  int m_height;
  bool m_framebufferResized = false;
  std::string m_windowName;
  GLFWwindow *m_window;
};