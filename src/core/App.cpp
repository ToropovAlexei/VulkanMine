#include "App.hpp"
#include "../renderer/backend/DescriptorsVk.hpp"
#include "Camera.hpp"
#include "Logger.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <utility>
#include <vector>

struct GlobalUBO {
  glm::mat4 projectionView;
};

App::App() {
  m_window = std::make_unique<Window>(800, 600, "VulkanMine");
  m_renderDevice = std::make_unique<RenderDeviceVk>(m_window.get());
  m_renderer = std::make_unique<Renderer>(m_window.get(), m_renderDevice.get());
  m_keyboard = std::make_unique<Keyboard>(m_window->getGLFWwindow());
  m_mouse = std::make_unique<Mouse>(m_window->getGLFWwindow());
  globalPool = DescriptorPoolVk::Builder(m_renderDevice.get())
                   .setMaxSets(SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(vk::DescriptorType::eUniformBuffer,
                                SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .build();
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
  Camera camera{};

  std::vector<Mesh<Vertex>> meshes = {};
  std::vector<PushConstantData> pushConstants = {};
  GameObject gameObject = {
      .mesh = Mesh<Vertex>(m_renderDevice.get(), vertices, indices),
      .model = glm::mat4(1.0f),
  };
  std::vector<GameObject> gameObjects;
  gameObjects.emplace_back(std::move(gameObject));
  FrameData frameData = {.commandBuffer = nullptr,
                         .gameObjects = std::move(gameObjects)};

  std::vector<std::unique_ptr<BufferVk>> uboBuffers(
      SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < SwapChainVk::MAX_FRAMES_IN_FLIGHT; i++) {
    uboBuffers[i] = std::make_unique<BufferVk>(
        m_renderDevice.get(), sizeof(GlobalUBO), 1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU); // TODO Check VMA_MEMORY_USAGE_CPU_TO_GPU
    uboBuffers[i]->map();
  }

  auto globalSetLayout = DescriptorSetLayoutVk::Builder(m_renderDevice.get())
                             .addBinding(0, vk::DescriptorType::eUniformBuffer,
                                         vk::ShaderStageFlagBits::eVertex |
                                             vk::ShaderStageFlagBits::eFragment)
                             .build();

  std::vector<vk::DescriptorSet> globalDescriptorSets(
      SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    DescriptorWriterVk(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  m_chunkRenderSystem = std::make_unique<ChunkRenderSystem>(
      m_renderDevice.get(), m_renderer->getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout());

  GlobalUBO ubo = {};

  m_timer.reset();
  while (!m_window->shouldClose()) {
    glfwPollEvents();
    m_timer.update();
    float deltaTime = m_timer.getDeltaTime();

    glm::vec3 movementDirection(0.0f);
    if (m_keyboard->isKeyPressed(GLFW_KEY_W)) {
      movementDirection += camera.getFront();
    }
    if (m_keyboard->isKeyPressed(GLFW_KEY_S)) {
      movementDirection -= camera.getFront();
    }
    if (m_keyboard->isKeyPressed(GLFW_KEY_A)) {
      movementDirection -= camera.getRight();
    }
    if (m_keyboard->isKeyPressed(GLFW_KEY_D)) {
      movementDirection += camera.getRight();
    }

    if (glm::dot(movementDirection, movementDirection) >
        std::numeric_limits<float>::epsilon()) {
      camera.move(deltaTime * 100.0f * glm::normalize(movementDirection));
    }
    camera.setProjection(glm::radians(50.0f), m_renderer->getAspectRatio(),
                         0.1f, 1000.0f);

    ubo.projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();

    Logger::debug("Camera position: " + std::to_string(camera.getPosition().x) +
                  ", " + std::to_string(camera.getPosition().y) + ", " +
                  std::to_string(camera.getPosition().z));

    if (auto commandBuffer = m_renderer->beginFrame()) {
      auto frameIndex = m_renderer->getFrameIndex();
      frameData.globalDescriptorSet = globalDescriptorSets[frameIndex];
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();
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
