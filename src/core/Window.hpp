#pragma once

#include "GLFW/glfw3.h"
#include <string>
#include <vulkan/vulkan.hpp>

class Window {
public:
  Window(int width, int height, std::string windowName);
  ~Window();

  inline GLFWwindow *getGLFWwindow() const noexcept { return m_window; }
  inline bool wasWindowResized() const noexcept { return m_framebufferResized; }
  void resetWindowResizedFlag() noexcept { m_framebufferResized = false; }
  inline vk::Extent2D getExtent() const noexcept {
    return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
  }
  inline bool shouldClose() const { return glfwWindowShouldClose(m_window); }
  void createWindowSurface(vk::Instance &instance, vk::SurfaceKHR &surface);
  inline void hideCursor() { glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
  inline void showCursor() { glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
  inline bool isCursorHidden() const { return glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED; }
  inline int getWidth() const noexcept { return m_width; }
  inline int getHeight() const noexcept { return m_height; }
  void pollEvents() { glfwPollEvents(); }

private:
  void initWindow();
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

private:
  int m_width;
  int m_height;
  bool m_framebufferResized = false;
  std::string m_windowName;
  GLFWwindow *m_window;
};