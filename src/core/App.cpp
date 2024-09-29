#include "App.hpp"
#include <memory>
#include <vector>

App::App() {
  m_window = std::make_unique<Window>(800, 600, "VulkanMine");
  m_renderDevice = std::make_unique<RenderDeviceVk>(m_window.get());
  m_renderer = std::make_unique<Renderer>(m_window.get(), m_renderDevice.get());
  m_chunkRenderSystem = std::make_unique<ChunkRenderSystem>(
      m_renderDevice.get(), m_renderer->getSwapChainRenderPass());
}

App::~App() { m_renderDevice->getDevice().waitIdle(); }

void App::run() {
  std::vector<Vertex> vertices = {
      {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Вершина 1 (красная)
      {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Вершина 2 (зеленая)
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}} // Вершина 3 (синяя)
  };
  std::vector<uint32_t> indices = {0, 1, 2};
  std::vector<Mesh<Vertex>> meshes = {};
  meshes.emplace_back(m_renderDevice.get(), vertices, indices);
  FrameData frameData = {
      .commandBuffer = nullptr,
      .meshes = std::move(meshes),
  };

  while (!m_window->shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = m_renderer->beginFrame()) {
      // int frameIndex = m_renderer->getFrameIndex();

      frameData.commandBuffer = commandBuffer;

      m_renderer->beginSwapChainRenderPass(commandBuffer);
      m_chunkRenderSystem->render(frameData);
      m_renderer->endSwapChainRenderPass(commandBuffer);
      m_renderer->endFrame();
    }
  }

  m_renderDevice->getDevice().waitIdle();
}
