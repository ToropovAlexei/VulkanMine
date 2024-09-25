#include "Window.h"
#include "GLFW/glfw3.h"
#include <stdexcept>

Window::Window(int width, int height, std::string windowName)
    : width(width), height(height), windowName(windowName) {
  initWindow();
}

void Window::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width,
                                       int height) {
  auto windowInstance =
      reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  windowInstance->framebufferResized = true;
  windowInstance->width = width;
  windowInstance->height = height;
}