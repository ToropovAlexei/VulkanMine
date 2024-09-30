#include "App.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <utility>
#include <vector>

App::App() {
  m_window = std::make_unique<Window>(800, 600, "VulkanMine");
  m_renderDevice = std::make_unique<RenderDeviceVk>(m_window.get());
  m_renderer = std::make_unique<Renderer>(m_window.get(), m_renderDevice.get());
  m_chunkRenderSystem = std::make_unique<ChunkRenderSystem>(
      m_renderDevice.get(), m_renderer->getSwapChainRenderPass());
  m_keyboard = std::make_unique<Keyboard>(m_window->getGLFWwindow());
  m_mouse = std::make_unique<Mouse>(m_window->getGLFWwindow());
}

App::~App() { m_renderDevice->getDevice().waitIdle(); }

void App::run() {
  // std::vector<Vertex> vertices = {
  //     // Позиции              // Цвета
  //     {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0
  //     {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  // 1
  //     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},   // 2
  //     {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},  // 3
  //     {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},  // 4
  //     {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},   // 5
  //     {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // 6
  //     {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}}    // 7
  // };
  std::vector<Vertex> vertices = {{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
                                  {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
                                  {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
                                  {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
                                  {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
                                  {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
                                  {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
                                  {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}}};

  std::vector<uint32_t> indices = {0, 1, 3, 3, 1, 2, 1, 5, 2, 2, 5, 6,
                                   5, 4, 6, 6, 4, 7, 4, 0, 7, 7, 0, 3,
                                   3, 2, 7, 7, 2, 6, 4, 5, 0, 0, 5, 1};

  // std::vector<uint32_t> indices = {// Задняя грань
  //                                  0, 1, 2, 2, 3, 0,
  //                                  // Передняя грань
  //                                  4, 5, 6, 6, 7, 4,
  //                                  // Левая грань
  //                                  0, 3, 7, 7, 4, 0,
  //                                  // Правая грань
  //                                  1, 5, 6, 6, 2, 1,
  //                                  // Нижняя грань
  //                                  0, 1, 5, 5, 4, 0,
  //                                  // Верхняя грань
  //                                  3, 2, 6, 6, 7, 3};
  std::vector<Mesh<Vertex>> meshes = {};
  std::vector<PushConstantData> pushConstants = {};
  GameObject gameObject = {
      .mesh = Mesh<Vertex>(m_renderDevice.get(), vertices, indices),
      .model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)),
  };
  std::vector<GameObject> gameObjects;
  gameObjects.emplace_back(std::move(gameObject));
  FrameData frameData = {.commandBuffer = nullptr,
                         .gameObjects = std::move(gameObjects)};

  while (!m_window->shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = m_renderer->beginFrame()) {
      // int frameIndex = m_renderer->getFrameIndex();
      frameData.gameObjects[0].model =
          glm::rotate(frameData.gameObjects[0].model, 0.00005f,
                      glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));

      frameData.commandBuffer = commandBuffer;

      m_renderer->beginSwapChainRenderPass(commandBuffer);
      m_chunkRenderSystem->render(frameData);
      m_renderer->endSwapChainRenderPass(commandBuffer);
      m_renderer->endFrame();
    }
  }

  m_renderDevice->getDevice().waitIdle();
}
