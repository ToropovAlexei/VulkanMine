#include "App.hpp"
#include <memory>

App::App() {
  m_window = std::make_unique<Window>(800, 600, "VulkanMine");
  m_renderDevice = std::make_unique<RenderDeviceVk>(m_window.get());
  m_renderer = std::make_unique<Renderer>(m_window.get(), m_renderDevice.get());
  m_chunkRenderSystem = std::make_unique<ChunkRenderSystem>(
      m_renderDevice.get(), m_renderer->getSwapChainRenderPass());
}

App::~App() { m_renderDevice->getDevice().waitIdle(); }

void App::run() {
  while (!m_window->shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = m_renderer->beginFrame()) {
      // int frameIndex = m_renderer->getFrameIndex();

      FrameData frameData;
      frameData.commandBuffer = commandBuffer;
      // frameData.meshes = {};

      m_renderer->beginSwapChainRenderPass(commandBuffer);
      m_chunkRenderSystem->render(frameData);
      m_renderer->endSwapChainRenderPass(commandBuffer);
      m_renderer->endFrame();
    }
  }
}
