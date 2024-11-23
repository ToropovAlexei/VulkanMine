#pragma once

#include "../input/Keyboard.hpp"
#include "../input/Mouse.hpp"
#include "../renderSystems/ChunkRenderSystem.hpp"
#include "../renderSystems/SkyboxRenderSystem.hpp"
#include "../renderer/backend/DescriptorsVk.hpp"
#include "../renderer/backend/Renderer.hpp"
#include "../world/BlocksManager.hpp"
#include "../world/ChunksManager.hpp"
#include "../world/PlayerController.hpp"
#include "../world/TextureAtlas.hpp"
#include "Camera.hpp"
#include "Window.hpp"
#include <memory>

struct GlobalUBO {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 projectionView;
  glm::vec3 cameraPosition;
  float dayTime;
};

class Scene {
public:
  Scene(RenderDeviceVk *device, Renderer *renderer, Keyboard *keyboard, Mouse *mouse, Window *window);
  ~Scene();

  void update(float dt);
  void render(vk::CommandBuffer commandBuffer);
  void renderUI();

private:
  RenderDeviceVk *m_device;
  Keyboard *m_keyboard;
  Mouse *m_mouse;
  Window *m_window;
  std::unique_ptr<DescriptorPoolVk> globalPool{};
  std::unique_ptr<ChunkRenderSystem> m_chunkRenderSystem;
  std::unique_ptr<SkyboxRenderSystem> m_skyboxRenderSystem;
  std::unique_ptr<Camera> m_camera;
  PlayerController m_playerController;
  Renderer *m_renderer;
  std::vector<vk::DescriptorSet> m_globalDescriptorSets;
  std::vector<std::unique_ptr<BufferVk>> m_globalBuffers;
  GlobalUBO m_ubo;
  TextureAtlas m_textureAtlas;
  BlocksManager m_blocksManager;
  ChunksManager m_chunksManager;
  FrameData m_prevFrameData;
  int m_dayTime = 9995;
};