#include "Scene.hpp"
#include "../renderer/backend/SwapChainVk.hpp"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstddef>
#include <memory>
#include <string>
#include <tracy/Tracy.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

Scene::Scene(RenderDeviceVk *device, Renderer *renderer, Keyboard *keyboard, Mouse *mouse)
    : m_device{device}, m_keyboard{keyboard}, m_mouse{mouse}, m_renderer{renderer},
      m_textureAtlas{device, "res/textures"}, m_blocksManager{"res/blocks", m_textureAtlas},
      m_playerController{{0, 128, 0}}, m_chunksManager{m_blocksManager, m_textureAtlas, m_playerController} {
  ZoneScoped;
  globalPool = DescriptorPoolVk::Builder(m_device)
                   .setMaxSets(SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(vk::DescriptorType::eUniformBuffer, SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(vk::DescriptorType::eCombinedImageSampler, SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .build();

  m_camera = std::make_unique<Camera>();
  m_camera->setPosition({0.0f, 128.0f, 0.0f});

  m_globalBuffers.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < SwapChainVk::MAX_FRAMES_IN_FLIGHT; i++) {
    m_globalBuffers[i] =
        std::make_unique<BufferVk>(m_device, sizeof(GlobalUBO), 1, vk::BufferUsageFlagBits::eUniformBuffer,
                                   VMA_MEMORY_USAGE_CPU_TO_GPU); // TODO Check VMA_MEMORY_USAGE_CPU_TO_GPU
    m_globalBuffers[i]->map();
  }

  auto globalSetLayout =
      DescriptorSetLayoutVk::Builder(m_device)
          .addBinding(0, vk::DescriptorType::eUniformBuffer,
                      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
          .addBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
          .build();

  m_globalDescriptorSets.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < m_globalDescriptorSets.size(); i++) {
    auto bufferInfo = m_globalBuffers[i]->descriptorInfo();
    auto imgInfo = m_textureAtlas.getTexture().getDescriptorInfo();
    DescriptorWriterVk(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imgInfo)
        .build(m_globalDescriptorSets[i]);
  }

  m_chunkRenderSystem = std::make_unique<ChunkRenderSystem>(m_device, m_renderer->getSwapChainRenderPass(),
                                                            globalSetLayout->getDescriptorSetLayout());
}

Scene::~Scene() {
  ZoneScoped;
  globalPool.reset();
}

void Scene::update(float dt) {
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

  if (glm::dot(movementDirection, movementDirection) > std::numeric_limits<float>::epsilon()) {
    m_playerController.move(dt * 2500.0f * glm::normalize(movementDirection));
  }

  if (m_playerController.getPosInChunk() != m_camera->getPosition()) {
    m_camera->setPosition(m_playerController.getPosInChunk());
  }
  auto yaw = m_mouse->getDeltaX() * 0.05f;
  auto pitch = -m_mouse->getDeltaY() * 0.05f;
  if (yaw != 0.0f || pitch != 0.0f) {
    m_camera->rotate(yaw, pitch);
  }
  m_chunksManager.forEachChunk([this](std::shared_ptr<Chunk> chunk) {
    ZoneScopedN("Generate Mesh");
    if (chunk && chunk->getMesh() == nullptr || chunk->isMeshOutdated()) {
      chunk->generateMesh(m_device);
    }
  });
}

void Scene::render(vk::CommandBuffer commandBuffer) {
  ZoneScoped;
  m_ubo.projectionView = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();

  auto frameIndex = m_renderer->getFrameIndex();
  m_chunksManager.updateFrustum(m_camera->getFrustum());
  FrameData frameData = {
      .commandBuffer = commandBuffer,
      .chunks = m_chunksManager.getChunksToRender(),
      .playerX = m_playerController.getChunkX(),
      .playerZ = m_playerController.getChunkZ(),
      .globalDescriptorSet = m_globalDescriptorSets[frameIndex],
      .frameIndex = frameIndex,
  };
  m_globalBuffers[frameIndex]->writeToBuffer(&m_ubo);
  m_globalBuffers[frameIndex]->flush();
  // frameData.gameObjects[0].model =
  //     glm::rotate(frameData.gameObjects[0].model, 0.00005f,
  //                 glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));

  m_chunkRenderSystem->render(frameData);
}

void Scene::renderUI() {
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
