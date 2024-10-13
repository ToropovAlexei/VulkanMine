#pragma once

#include "../input/Keyboard.hpp"
#include "../input/Mouse.hpp"
#include "../renderSystems/ChunkRenderSystem.hpp"
#include "../renderer/backend/DescriptorsVk.hpp"
#include "../renderer/backend/Renderer.hpp"
#include "../world/BlocksManager.hpp"
#include "../world/TextureAtlas.hpp"
#include "Camera.hpp"
#include <memory>

struct GlobalUBO {
  glm::mat4 projectionView;
};

class Scene {
public:
  Scene(RenderDeviceVk *device, Renderer *renderer, Keyboard *keyboard,
        Mouse *mouse);
  ~Scene();

  void update(float dt);
  void render(vk::CommandBuffer commandBuffer);
  void renderUI();

private:
  RenderDeviceVk *m_device;
  Keyboard *m_keyboard;
  Mouse *m_mouse;
  std::unique_ptr<DescriptorPoolVk> globalPool{};
  std::unique_ptr<ChunkRenderSystem> m_chunkRenderSystem;
  std::unique_ptr<Camera> m_camera;
  Renderer *m_renderer;
  std::vector<vk::DescriptorSet> m_globalDescriptorSets;
  std::vector<std::unique_ptr<BufferVk>> m_globalBuffers;
  GlobalUBO m_ubo;
  std::vector<std::shared_ptr<Chunk>> m_chunks;
  TextureAtlas m_textureAtlas;
  BlocksManager m_blocksManager;
};