#include "Renderer.hpp"
#include "SwapChainVk.hpp"

Renderer::Renderer(Window *window, RenderDeviceVk *device)
    : m_device{device}, m_window{window} {
  recreateSwapChain();
}

void Renderer::recreateSwapChain() {
  auto extent = m_window->getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = m_window->getExtent();
    glfwWaitEvents();
  }
  m_device->getDevice().waitIdle();

  if (m_swapChain == nullptr) {
    m_swapChain = std::make_unique<SwapChainVk>(m_device, extent);
  } else {
    std::shared_ptr<SwapChainVk> oldSwapChain = std::move(m_swapChain);
    m_swapChain = std::make_unique<SwapChainVk>(m_device, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*m_swapChain.get())) {
      throw std::runtime_error("Swap chain image or depth format has changed!");
    }
  }
}