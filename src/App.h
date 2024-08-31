#pragma once

#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxModel.hpp"
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
  void loadModels();
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();

private:
  Window window{WIDTH, HEIGHT, "Vulkan"};
  GfxDevice gfxDevice{window};
  GfxSwapChain gfxSwapChain{gfxDevice, window.getExtent()};
  std::unique_ptr<GfxPipeline> gfxPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
  std::unique_ptr<GfxModel> gfxModel;
};