#include "App.h"
#include "Graphics/FrameInfo.hpp"
#include "Graphics/GfxBuffer.hpp"
#include "Graphics/GfxDescriptors.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxModel.hpp"
#include "Graphics/GfxSwapChain.hpp"
#include "Graphics/SimpleRenderSystem.hpp"
#include "KeyboardMovementController.hpp"
#include "glm/fwd.hpp"
#include <chrono>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

struct GlobalUBO {
  glm::mat4 projectionView;
  glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.3f};
  glm::vec4 lightPos{-1.0f};
  glm::vec4 lightColor{1.0f};
};

App::App() {
  globalPool = GfxDescriptorPool::Builder(gfxDevice)
                   .setMaxSets(GfxSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                GfxSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .build();
  loadGameObjects();
  keyboard = std::make_unique<Keyboard>(window.getGLFWwindow());
}

void App::run() {
  std::vector<std::unique_ptr<GfxBuffer>> uboBuffers(
      GfxSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < GfxSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    uboBuffers[i] = std::make_unique<GfxBuffer>(
        gfxDevice, sizeof(GlobalUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }

  auto globalSetLayout =
      GfxDescriptorSetLayout::Builder(gfxDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(
      GfxSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    GfxDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem simpleRenderSystem{
      gfxDevice, renderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  Camera camera{};

  auto currentTime = std::chrono::high_resolution_clock::now();
  auto viewerObject = GameObject::createGameObject();
  KeyboardMovementController cameraController{};

  while (!window.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();
    currentTime = newTime;

    cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime,
                                   viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation,
                      viewerObject.transform.rotation);

    float aspectRatio = renderer.getAspectRatio();

    camera.setPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f,
                                    100.0f);
    if (auto commandBuffer = renderer.beginFrame()) {
      int frameIndex = renderer.getFrameIndex();

      GlobalUBO ubo{
          .projectionView = camera.getProjection() * camera.getView(),
          .lightPos = glm::vec4(viewerObject.transform.translation, 1.0f),
      };
      FrameInfo frameInfo{
          .frameIndex = frameIndex,
          .frameTime = frameTime,
          .commandBuffer = commandBuffer,
          .camera = camera,
          .globalDescriptorSet = globalDescriptorSets[frameIndex],
          .gameObjects = gameObjects,
      };
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      renderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(frameInfo);
      renderer.endSwapChainRenderPass(commandBuffer);
      renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(gfxDevice.device());
}

void App::loadGameObjects() {
  std::shared_ptr<GfxModel> model =
      GfxModel::createGfxModelFromFile(gfxDevice, "models/tank.obj");

  auto obj = GameObject::createGameObject();
  obj.model = model;
  obj.transform.translation = {0.0f, 1.0f, 2.5f};
  obj.transform.rotation = {glm::half_pi<float>(), 0.0f, 0.0f};
  obj.transform.scale = {0.5f, 0.5f, 0.5f};

  // auto cube2 = GameObject::createGameObject();
  // cube2.model = model;
  // cube2.transform.translation = {2.5f, 0.0f, 0.0f};
  // cube2.transform.scale = {0.5f, 0.5f, 0.5f};

  // auto cube3 = GameObject::createGameObject();
  // cube3.model = model;
  // cube3.transform.translation = {-2.5f, 0.0f, 0.0f};
  // cube3.transform.scale = {0.5f, 0.5f, 0.5f};

  gameObjects.emplace(obj.getId(), std::move(obj));
  // gameObjects.push_back(std::move(cube2));
  // gameObjects.push_back(std::move(cube3));
}
