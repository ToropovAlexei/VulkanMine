#include "Scene.hpp"
#include "../renderer/backend/SwapChainVk.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstddef>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

Scene::Scene(RenderDeviceVk *device, Renderer *renderer, Keyboard *keyboard,
             Mouse *mouse)
    : m_device{device}, m_keyboard{keyboard}, m_mouse{mouse},
      m_renderer{renderer}, m_texture{device, "res/textures/dirt.png"} {
  globalPool = DescriptorPoolVk::Builder(m_device)
                   .setMaxSets(SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(vk::DescriptorType::eUniformBuffer,
                                SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(vk::DescriptorType::eCombinedImageSampler,
                                SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .build();

  // Обновленные вершины для куба с корректными UV-координатами
  std::vector<Vertex> vertices = {
      // Передняя грань
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}}, // Нижний левый
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},  // Нижний правый
      {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},   // Верхний правый
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},  // Верхний левый
      // Задняя грань
      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},  // Нижний левый
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}}, // Нижний правый
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},  // Верхний правый
      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},   // Верхний левый
      // Левая грань
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, // Нижний левый
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},  // Нижний правый
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},   // Верхний правый
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},  // Верхний левый
      // Правая грань
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},  // Нижний левый
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}}, // Нижний правый
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},  // Верхний правый
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},   // Верхний левый
      // Верхняя грань
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},  // Нижний левый
      {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},   // Нижний правый
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},  // Верхний правый
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}}, // Верхний левый
      // Нижняя грань
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}, // Нижний левый
      {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},  // Нижний правый
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},   // Верхний правый
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}}   // Верхний левый
  };

  std::vector<uint32_t> indices = {// Передняя грань
                                   0, 1, 2, 2, 3, 0,
                                   // Задняя грань
                                   4, 5, 6, 6, 7, 4,
                                   // Левая грань
                                   8, 9, 10, 10, 11, 8,
                                   // Правая грань
                                   12, 13, 14, 14, 15, 12,
                                   // Верхняя грань
                                   16, 17, 18, 18, 19, 16,
                                   // Нижняя грань
                                   20, 21, 22, 22, 23, 20};

  m_camera = std::make_unique<Camera>();

  m_gameObjects.push_back(std::make_shared<GameObject>(
      GameObject{.mesh = Mesh<Vertex>(m_device, vertices, indices),
                 .model = glm::mat4(1.0f)}));

  m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
      .mesh = Mesh<Vertex>(m_device, vertices, indices),
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f))}));
  m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
      .mesh = Mesh<Vertex>(m_device, vertices, indices),
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))}));
  m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
      .mesh = Mesh<Vertex>(m_device, vertices, indices),
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 2.0f))}));

  for (int x = 0; x < 25; x++) {
    for (int y = 0; y < 25; y++) {
      for (int z = 0; z < 25; z++) {
        m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
            .mesh = Mesh<Vertex>(m_device, vertices, indices),
            .model = glm::translate(glm::mat4(1.0f),
                                    glm::vec3(x * 2.0f, y * 2.0f, z * 2.0f))}));
      }
    }
  }

  m_globalBuffers.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < SwapChainVk::MAX_FRAMES_IN_FLIGHT; i++) {
    m_globalBuffers[i] = std::make_unique<BufferVk>(
        m_device, sizeof(GlobalUBO), 1, vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU); // TODO Check VMA_MEMORY_USAGE_CPU_TO_GPU
    m_globalBuffers[i]->map();
  }

  auto globalSetLayout =
      DescriptorSetLayoutVk::Builder(m_device)
          .addBinding(0, vk::DescriptorType::eUniformBuffer,
                      vk::ShaderStageFlagBits::eVertex |
                          vk::ShaderStageFlagBits::eFragment)
          .addBinding(1, vk::DescriptorType::eCombinedImageSampler,
                      vk::ShaderStageFlagBits::eFragment)
          .build();

  m_globalDescriptorSets.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < m_globalDescriptorSets.size(); i++) {
    auto bufferInfo = m_globalBuffers[i]->descriptorInfo();
    auto imgInfo = m_texture.getDescriptorInfo();
    DescriptorWriterVk(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imgInfo)
        .build(m_globalDescriptorSets[i]);
  }

  m_chunkRenderSystem = std::make_unique<ChunkRenderSystem>(
      m_device, m_renderer->getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout());
}

Scene::~Scene() { globalPool.reset(); }

void Scene::update(float dt) {
  m_camera->setProjection(75.0f, m_renderer->getAspectRatio(), 0.1f, 1000.0f);
  glm::vec3 movementDirection(0.0f);
  if (m_keyboard->isKeyPressed(GLFW_KEY_W)) {
    movementDirection += m_camera->getFront();
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_S)) {
    movementDirection -= m_camera->getFront();
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_A)) {
    movementDirection -= m_camera->getRight();
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_D)) {
    movementDirection += m_camera->getRight();
  }

  if (m_keyboard->isKeyPressed(GLFW_KEY_SPACE)) {
    movementDirection += glm::vec3(0.0f, 1.0f, 0.0f);
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_X)) {
    movementDirection += glm::vec3(0.0f, -1.0f, 0.0f);
  }

  if (glm::dot(movementDirection, movementDirection) >
      std::numeric_limits<float>::epsilon()) {
    m_camera->move(dt * 25.0f * glm::normalize(movementDirection));
  }

  m_camera->rotate(m_mouse->getDeltaX() * 0.05f, -m_mouse->getDeltaY() * 0.05f);
}

void Scene::render(vk::CommandBuffer commandBuffer) {
  m_ubo.projectionView =
      m_camera->getProjectionMatrix() * m_camera->getViewMatrix();

  auto frameIndex = m_renderer->getFrameIndex();
  FrameData frameData = {
      .commandBuffer = commandBuffer,
      .gameObjects = m_gameObjects,
      .globalDescriptorSet = m_globalDescriptorSets[frameIndex],
  };
  m_globalBuffers[frameIndex]->writeToBuffer(&m_ubo);
  m_globalBuffers[frameIndex]->flush();
  // frameData.gameObjects[0].model =
  //     glm::rotate(frameData.gameObjects[0].model, 0.00005f,
  //                 glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));

  m_chunkRenderSystem->render(frameData);
}

void Scene::renderUI() {
  ImGuiContext &g = *GImGui;
  ImGuiIO &io = g.IO;
  ImGui::Begin("Engine info");
  ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
              io.Framerate);
  ImGui::Text("%d vertices, %d indices (%d triangles)",
              io.MetricsRenderVertices, io.MetricsRenderIndices,
              io.MetricsRenderIndices / 3);
  ImGui::End();

  ImGui::Begin("Player");
  ImGui::Text("Position: (x: %.1f, y: %.1f, z: %.1f)",
              m_camera->getPosition().x, m_camera->getPosition().y,
              m_camera->getPosition().z);
  ImGui::Text("Rotation: (x: %.1f, y: %.1f, z: %.1f)", m_camera->getFront().x,
              m_camera->getFront().y, m_camera->getFront().z);
  ImGui::End();
}
