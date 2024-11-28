#include "TestScene.hpp"
#include "../renderer/backend/SwapChainVk.hpp"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstddef>
#include <memory>
#include <tracy/Tracy.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

TestScene::TestScene(RenderDeviceVk *device, Renderer *renderer, Keyboard *keyboard, Mouse *mouse, Window *window)
    : m_device{device}, m_keyboard{keyboard}, m_mouse{mouse}, m_renderer{renderer}, m_window{window},
      m_playerController{{0, 5, 0}} {
  ZoneScoped;
  globalPool = DescriptorPoolVk::Builder(m_device)
                   .setMaxSets(SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(vk::DescriptorType::eUniformBuffer, SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .build();

  m_camera = std::make_unique<Camera>();
  m_camera->setPosition({0.0f, 128.0f, 0.0f});

  m_globalBuffers.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < SwapChainVk::MAX_FRAMES_IN_FLIGHT; i++) {
    m_globalBuffers[i] =
        std::make_unique<BufferVk>(m_device, sizeof(GlobalUBO), 1, vk::BufferUsageFlagBits::eUniformBuffer,
                                   VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
  }

  auto globalSetLayout = DescriptorSetLayoutVk::Builder(m_device)
                             .addBinding(0, vk::DescriptorType::eUniformBuffer,
                                         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
                             .build();

  m_globalDescriptorSets.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < m_globalDescriptorSets.size(); i++) {
    auto bufferInfo = m_globalBuffers[i]->descriptorInfo();
    DescriptorWriterVk(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(m_globalDescriptorSets[i]);
  }

  m_gridRenderSystem = std::make_unique<GridRenderSystem>(m_device, m_renderer->getSwapChainRenderPass(),
                                                          globalSetLayout->getDescriptorSetLayout());
}

TestScene::~TestScene() {
  ZoneScoped;
  globalPool.reset();
}

void TestScene::update(float dt) {
  ZoneScoped;
  if (m_camera->getAspectRatio() != m_renderer->getAspectRatio()) {
    m_camera->setProjection(75.0f, m_renderer->getAspectRatio(), 0.1f, 2000.0f);
  }
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

  if (m_keyboard->isKeyJustPressed(GLFW_KEY_C)) {
    if (m_window->isCursorHidden()) {
      m_window->showCursor();
    } else {
      m_window->hideCursor();
    }
  }

  if (glm::dot(movementDirection, movementDirection) > std::numeric_limits<float>::epsilon()) {
    m_playerController.move(dt * 20.0f * glm::normalize(movementDirection));
  }

  if (m_playerController.getPosInChunk() != m_camera->getPosition()) {
    m_camera->setPosition(m_playerController.getPosInChunk());
  }
  auto yaw = m_mouse->getDeltaX() * 0.05f;
  auto pitch = -m_mouse->getDeltaY() * 0.05f;
  if (yaw != 0.0f || pitch != 0.0f) {
    m_camera->rotate(yaw, pitch);
  }
}

void TestScene::render(vk::CommandBuffer commandBuffer) {
  ZoneScoped;
  m_ubo.projectionView = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
  m_ubo.view = m_camera->getViewMatrix();
  m_ubo.projection = m_camera->getProjectionMatrix();
  m_ubo.dayTime = static_cast<float>(0);

  auto frameIndex = m_renderer->getFrameIndex();
  FrameData frameData = {
      .commandBuffer = commandBuffer,
      .playerX = m_playerController.getChunkX(),
      .playerZ = m_playerController.getChunkZ(),
      .globalDescriptorSet = m_globalDescriptorSets[frameIndex],
      .frameIndex = frameIndex,
  };

  m_globalBuffers[frameIndex]->writeToBuffer(&m_ubo);
  m_globalBuffers[frameIndex]->flush();

  m_gridRenderSystem->render(frameData);
}

void TestScene::renderUI() {
  ZoneScoped;
  ImGuiContext &g = *GImGui;
  ImGuiIO &io = g.IO;
  ImGui::Begin("Engine info");
  ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
  ImGui::End();

  ImGui::Begin("Player");
  ImGui::Text("Position: x: %.1f, y: %.1f, z: %.1f", m_playerController.getWorldX(), m_playerController.getWorldY(),
              m_playerController.getWorldZ());
  ImGui::Text("Rotation: x: %.1f, y: %.1f, z: %.1f", m_camera->getFront().x, m_camera->getFront().y,
              m_camera->getFront().z);
  ImGui::Text("Chunk: %d, %d", m_playerController.getChunkX(), m_playerController.getChunkZ());
  ImGui::End();
}
