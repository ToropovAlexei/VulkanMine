#include "Window.hpp"

Window::Window(int width, int height, std::string windowName)
    : m_width(width), m_height(height), m_windowName(windowName) {
  initWindow();
}

Window::~Window() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void Window::createWindowSurface(vk::Instance &instance,
                                 vk::SurfaceKHR &surface) {
  VkSurfaceKHR rawSurface;
  VkResult result =
      glfwCreateWindowSurface(instance, m_window, nullptr, &rawSurface);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface!");
  }

  surface = vk::SurfaceKHR(rawSurface);
}

void Window::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr,
                              nullptr);

  glfwSetWindowUserPointer(m_window, this);
  glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width,
                                       int height) {
  auto windowInstance =
      reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  windowInstance->m_framebufferResized = true;
  windowInstance->m_width = width;
  windowInstance->m_height = height;
}
