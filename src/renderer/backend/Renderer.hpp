#pragma once

#include "RenderDeviceVk.hpp"
#include "SwapChainVk.hpp"
#include <memory>

class Renderer {
public:
  Renderer(Window *window, RenderDeviceVk *device);

private:
  void recreateSwapChain();

private:
  RenderDeviceVk *m_device;
  Window *m_window;
  std::unique_ptr<SwapChainVk> m_swapChain;
};