#pragma once

#include "GameObject.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxPipeline.h"
#include "Graphics/Renderer.hpp"
#include "Window.h"
#include "utils/NonCopyable.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class App : NonCopyable {
public:
  App();
  ~App();

  void run();

public:
  static const int WIDTH = 800;
  static const int HEIGHT = 600;

private:
  void loadGameObjects();
  void createPipelineLayout();
  void createPipeline();
  void renderGameObjects(VkCommandBuffer commandBuffer);

private:
  Window window{WIDTH, HEIGHT, "Vulkan"};
  GfxDevice gfxDevice{window};
  Renderer renderer{window, gfxDevice};
  std::unique_ptr<GfxPipeline> gfxPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<GameObject> gameObjects;
};