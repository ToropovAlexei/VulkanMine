#pragma once

#include "Graphics/GfxPipeline.h"
#include "Window.h"

class App {
public:
  void run();

public:
  static const int WIDTH = 800;
  static const int HEIGHT = 600;

private:
  Window window{WIDTH, HEIGHT, "Vulkan"};
  GfxPipeline gfxPipeline{"shaders/simple_shader.vert.spv",
                          "shaders/simple_shader.frag.spv"};
};