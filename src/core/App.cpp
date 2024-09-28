#include "App.hpp"
#include <memory>

App::App() {
  m_window = std::make_unique<Window>(800, 600, "VulkanMine");
  m_renderDevice = std::make_unique<RenderDeviceVk>(m_window.get());
  m_renderer = std::make_unique<Renderer>(m_window.get(), m_renderDevice.get());
}

App::~App() { m_renderDevice->getDevice().waitIdle(); }

void App::run() {
  while (!m_window->shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = m_renderer->beginFrame()) {
      int frameIndex = m_renderer->getFrameIndex();

      m_renderer->beginSwapChainRenderPass(commandBuffer);
      m_renderer->endSwapChainRenderPass(commandBuffer);
      m_renderer->endFrame();
    }
  }
}
