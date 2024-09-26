#pragma once

#include "../renderer/backend/RenderDeviceVk.hpp"
#include "NonCopyable.hpp"
#include "Window.hpp"
#include <memory>

class App : NonCopyable {
public:
  App();

  void run();

private:
  std::unique_ptr<Window> m_window;
  std::unique_ptr<RenderDeviceVk> m_renderDevice;
};