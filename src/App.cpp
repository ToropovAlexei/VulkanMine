#include "App.h"
#include "Graphics/GfxModel.hpp"
#include "glm/fwd.hpp"
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

App::App() { loadGameObjects(); }

void App::run() {
  while (!window.shouldClose()) {
    glfwPollEvents();
    if (auto commandBuffer = renderer.beginFrame()) {
      renderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      renderer.endSwapChainRenderPass(commandBuffer);
      renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(gfxDevice.device());
}

void App::loadGameObjects() {
  std::vector<GfxModel::Vertex> vertices = {
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  auto gfxModel = std::make_shared<GfxModel>(gfxDevice, vertices);

  auto triangle = GameObject::createGameObject();
  triangle.model = gfxModel;
  triangle.color = {0.0f, 1.0f, 0.0f};
  triangle.transform2d.translation.x = 0.2f;
  triangle.transform2d.scale = {2.0f, 2.0f};
  triangle.transform2d.rotation = glm::half_pi<float>();

  gameObjects.push_back(std::move(triangle));
}
