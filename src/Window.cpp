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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
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