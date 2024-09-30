#pragma once

#include "../input/Keyboard.hpp"
#include "../input/Mouse.hpp"
#include "../renderSystems/ChunkRenderSystem.hpp"
#include "../renderer/backend/RenderDeviceVk.hpp"
#include "../renderer/backend/Renderer.hpp"
#include "NonCopyable.hpp"
#include "Timer.hpp"
#include "Window.hpp"
#include <memory>

class App : NonCopyable {
public:
  App();
  ~App();

  void run();

private:
  std::unique_ptr<Window> m_window;
  std::unique_ptr<RenderDeviceVk> m_renderDevice;
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<ChunkRenderSystem> m_chunkRenderSystem;
  std::unique_ptr<Keyboard> m_keyboard;
  std::unique_ptr<Mouse> m_mouse;
  Timer m_timer;
};