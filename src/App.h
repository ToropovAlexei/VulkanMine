#pragma once

#include "GameObject.hpp"
#include "Graphics/GfxDescriptors.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/Renderer.hpp"
#include "Input/Keyboard.hpp"
#include "Input/Mouse.hpp"
#include "Window.h"
#include "utils/NonCopyable.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

class App : NonCopyable {
public:
  App();

  void run();

public:
  static const int WIDTH = 800;
  static const int HEIGHT = 600;

private:
  void loadGameObjects();

private:
  Window window{WIDTH, HEIGHT, "Vulkan"};
  GfxDevice gfxDevice{window};
  Renderer renderer{window, gfxDevice};
  std::unique_ptr<Keyboard> m_keyboard;
  std::unique_ptr<Mouse> m_mouse;

  std::unique_ptr<GfxDescriptorPool> globalPool{};
  GameObject::Map gameObjects;
};