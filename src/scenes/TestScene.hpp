#pragma once

#include "../core/Camera.hpp"
#include "../core/Window.hpp"
#include "../input/Keyboard.hpp"
#include "../input/Mouse.hpp"
#include "../renderSystems/GridRenderSystem.hpp"
#include "../renderer/backend/DescriptorsVk.hpp"
#include "../renderer/backend/Renderer.hpp"
#include "../world/PlayerController.hpp"
#include <memory>

struct GlobalUBO {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 projectionView;
  float dayTime;
};

class TestScene {
public:
  TestScene(RenderDeviceVk *device, Renderer *renderer, Keyboard *keyboard, Mouse *mouse, Window *window);
  ~TestScene();

  void update(float dt);
  void render(vk::CommandBuffer commandBuffer);
  void renderUI();

private:
  RenderDeviceVk *m_device;
  Keyboard *m_keyboard;
  Mouse *m_mouse;
  Window *m_window;
  std::unique_ptr<DescriptorPoolVk> globalPool{};
  std::unique_ptr<GridRenderSystem> m_gridRenderSystem;
  std::unique_ptr<Camera> m_camera;
  PlayerController m_playerController;
  Renderer *m_renderer;
  std::vector<vk::DescriptorSet> m_globalDescriptorSets;
  std::vector<std::unique_ptr<BufferVk>> m_globalBuffers;
  GlobalUBO m_ubo;
  FrameData m_prevFrameData;
};