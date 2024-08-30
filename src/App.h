#pragma once

#include "Graphics/GfxDevice.hpp"
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
  GfxDevice gfxDevice{window};
  GfxPipeline gfxPipeline{
      gfxDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv",
      GfxPipeline::defaultGfxPipelineConfigInfo(WIDTH, HEIGHT)};
};