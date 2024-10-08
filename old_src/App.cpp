#include "App.h"
#include "FastNoiseLite.h"
#include "Graphics/FrameInfo.hpp"
#include "Graphics/GfxBuffer.hpp"
#include "Graphics/GfxDescriptors.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxModel.hpp"
#include "Graphics/GfxSwapChain.hpp"
#include "KeyboardMovementController.hpp"
#include "RenderSystems/ChunkRenderSystem.hpp"
#include "World/Chunk.hpp"
#include "glm/fwd.hpp"
#include <chrono>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

std::vector<GfxModel::Vertex>
generateTerrainMesh(int width, int height, float scale, float amplitude) {
  std::vector<GfxModel::Vertex> vertices;
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  noise.SetFrequency(0.05f); // Контролирует частоту шума

  // Генерация вершин с использованием Perlin noise для высоты
  for (int z = 0; z < height; ++z) {
    for (int x = 0; x < width; ++x) {
      float heightValue =
          noise.GetNoise((float)x * scale, (float)z * scale) * amplitude;
      glm::vec3 color;
      if (heightValue < 0.3f) {
        color = glm::vec3(0.1f, 0.4f, 0.1f); // Зеленая трава
      } else if (heightValue < 0.8f) {
        color = glm::vec3(0.5f, 0.3f, 0.2f); // Коричневая земля
      } else {
        color = glm::vec3(1.0f, 1.0f, 1.0f); // Белый снег
      }
      vertices.push_back({
          .pos = glm::vec3(x, heightValue, z),
          .color = color,
      });
    }
  }
  return vertices;
}

std::vector<uint32_t> generateTerrainIndices(int width, int height) {
  std::vector<uint32_t> indices;

  for (int z = 0; z < height - 1; ++z) {
    for (int x = 0; x < width - 1; ++x) {
      int topLeft = (z * width) + x;
      int topRight = topLeft + 1;
      int bottomLeft = ((z + 1) * width) + x;
      int bottomRight = bottomLeft + 1;

      // Первый треугольник
      indices.push_back(topLeft);
      indices.push_back(bottomLeft);
      indices.push_back(topRight);

      // Второй треугольник
      indices.push_back(topRight);
      indices.push_back(bottomLeft);
      indices.push_back(bottomRight);
    }
  }

  return indices;
}

struct GlobalUBO {
  glm::mat4 projectionView;
};

App::App() {
  globalPool = GfxDescriptorPool::Builder(gfxDevice)
                   .setMaxSets(GfxSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                GfxSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .build();
  loadGameObjects();
  m_keyboard = std::make_unique<Keyboard>(window.getGLFWwindow());
  m_mouse = std::make_unique<Mouse>(window.getGLFWwindow());
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

  ChunkRenderSystem chunkRenderSystem{
      gfxDevice, renderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  Camera camera{};

  auto currentTime = std::chrono::high_resolution_clock::now();
  auto viewerObject = GameObject::createGameObject();
  KeyboardMovementController cameraController{};

  window.hideCursor();

  while (!window.shouldClose()) {
    m_keyboard->update();
    m_mouse->update();
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();
    currentTime = newTime;

    cameraController.moveInPlaneXZ(m_keyboard, m_mouse, frameTime,
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
      chunkRenderSystem.renderChunks(frameInfo);
      renderer.endSwapChainRenderPass(commandBuffer);
      renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(gfxDevice.device());
}

void App::loadGameObjects() {
  auto vertices = generateTerrainMesh(256, 256, 0.5f, 10.0f);
  auto indices = generateTerrainIndices(256, 256);
  std::shared_ptr<GfxModel> terrainModel =
      std::make_shared<GfxModel>(gfxDevice, vertices, indices);
  auto terrain = GameObject::createGameObject();
  terrain.model = terrainModel;
  terrain.transform.rotation = {glm::pi<float>(), 0.0f, 0.0f};
  std::shared_ptr<GfxModel> chunkModel = Chunk::getTestChunkModel(gfxDevice);
  auto chunk = GameObject::createGameObject();
  chunk.model = chunkModel;

  gameObjects.emplace(terrain.getId(), std::move(terrain));
  gameObjects.emplace(chunk.getId(), std::move(chunk));
}
