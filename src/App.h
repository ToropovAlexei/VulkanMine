#pragma once

#include "GameObject.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxPipeline.h"
#include "Graphics/GfxSwapChain.hpp"
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
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void recreateSwapChain();
  void recordCommandBuffer(int imageIndex);
  void renderGameObjects(VkCommandBuffer commandBuffer);

private:
  Window window{WIDTH, HEIGHT, "Vulkan"};
  GfxDevice gfxDevice{window};
  std::unique_ptr<GfxSwapChain> gfxSwapChain;
  std::unique_ptr<GfxPipeline> gfxPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<GameObject> gameObjects;
};