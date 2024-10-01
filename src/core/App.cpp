#include "App.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

App::App() {
  m_window = std::make_unique<Window>(800, 600, "VulkanMine");
  m_renderDevice = std::make_unique<RenderDeviceVk>(m_window.get());
  m_renderer = std::make_unique<Renderer>(m_window.get(), m_renderDevice.get());
  m_keyboard = std::make_unique<Keyboard>(m_window->getGLFWwindow());
  m_mouse = std::make_unique<Mouse>(m_window->getGLFWwindow());
  m_scene = std::make_unique<Scene>(m_renderDevice.get(), m_renderer.get(),
                                    m_keyboard.get(), m_mouse.get());
}

App::~App() { m_renderDevice->getDevice().waitIdle(); }

void App::run() {
  m_window->hideCursor();

  m_timer.reset();
  while (!m_window->shouldClose()) {
    m_keyboard->update();
    m_mouse->update();
    glfwPollEvents();
    m_timer.update();
    float deltaTime = m_timer.getDeltaTime();
    m_scene->update(deltaTime);

    if (auto commandBuffer = m_renderer->beginFrame()) {
      m_renderer->beginSwapChainRenderPass(commandBuffer);
      m_scene->render(commandBuffer);
      m_renderer->endSwapChainRenderPass(commandBuffer);
      m_renderer->endFrame();
    }
  }

  m_renderDevice->getDevice().waitIdle();
}
