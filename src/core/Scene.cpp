#include "Scene.hpp"
#include "../renderer/backend/SwapChainVk.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <memory>

Scene::Scene(RenderDeviceVk *device, Renderer *renderer, Keyboard *keyboard,
             Mouse *mouse)
    : m_device{device}, m_keyboard{keyboard}, m_mouse{mouse},
      m_renderer{renderer} {
  globalPool = DescriptorPoolVk::Builder(m_device)
                   .setMaxSets(SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(vk::DescriptorType::eUniformBuffer,
                                SwapChainVk::MAX_FRAMES_IN_FLIGHT)
                   .build();

  std::vector<Vertex> vertices = {{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                  {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                  {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
                                  {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
                                  {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
                                  {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
                                  {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
                                  {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}}};

  std::vector<uint32_t> indices = {0, 1, 3, 3, 1, 2, 1, 5, 2, 2, 5, 6,
                                   5, 4, 6, 6, 4, 7, 4, 0, 7, 7, 0, 3,
                                   3, 2, 7, 7, 2, 6, 4, 5, 0, 0, 5, 1};

  m_camera = std::make_unique<Camera>();

  m_gameObjects.push_back(std::make_shared<GameObject>(
      GameObject{.mesh = Mesh<Vertex>(m_device, vertices, indices),
                 .model = glm::mat4(1.0f)}));

  m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
      .mesh = Mesh<Vertex>(m_device, vertices, indices),
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f))}));
  m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
      .mesh = Mesh<Vertex>(m_device, vertices, indices),
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))}));
  m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
      .mesh = Mesh<Vertex>(m_device, vertices, indices),
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 2.0f))}));

  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 10; y++) {
      for (int z = 0; z < 10; z++) {
        m_gameObjects.push_back(std::make_shared<GameObject>(GameObject{
            .mesh = Mesh<Vertex>(m_device, vertices, indices),
            .model = glm::translate(glm::mat4(1.0f),
                                    glm::vec3(x * 3.0f, y * 3.0f, z * 3.0f))}));
      }
    }
  }

  m_globalBuffers.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < SwapChainVk::MAX_FRAMES_IN_FLIGHT; i++) {
    m_globalBuffers[i] = std::make_unique<BufferVk>(
        m_device, sizeof(GlobalUBO), 1, vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU); // TODO Check VMA_MEMORY_USAGE_CPU_TO_GPU
    m_globalBuffers[i]->map();
  }

  auto globalSetLayout = DescriptorSetLayoutVk::Builder(m_device)
                             .addBinding(0, vk::DescriptorType::eUniformBuffer,
                                         vk::ShaderStageFlagBits::eVertex |
                                             vk::ShaderStageFlagBits::eFragment)
                             .build();

  m_globalDescriptorSets.resize(SwapChainVk::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < m_globalDescriptorSets.size(); i++) {
    auto bufferInfo = m_globalBuffers[i]->descriptorInfo();
    DescriptorWriterVk(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_globalDescriptorSets[i]);
  }

  m_chunkRenderSystem = std::make_unique<ChunkRenderSystem>(
      m_device, m_renderer->getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout());
}

Scene::~Scene() { globalPool.reset(); }

void Scene::update(float dt) {
  m_camera->setProjection(glm::radians(50.0f), m_renderer->getAspectRatio(),
                          0.1f, 1000.0f);
  glm::vec3 movementDirection(0.0f);
  if (m_keyboard->isKeyPressed(GLFW_KEY_W)) {
    movementDirection += m_camera->getFront();
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_S)) {
    movementDirection -= m_camera->getFront();
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_A)) {
    movementDirection -= m_camera->getRight();
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_D)) {
    movementDirection += m_camera->getRight();
  }

  if (m_keyboard->isKeyPressed(GLFW_KEY_SPACE)) {
    movementDirection += m_camera->getUp();
  }
  if (m_keyboard->isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
    movementDirection -= m_camera->getUp();
  }

  if (glm::dot(movementDirection, movementDirection) >
      std::numeric_limits<float>::epsilon()) {
    m_camera->move(dt * 25.0f * glm::normalize(movementDirection));
  }

  m_camera->rotate(m_mouse->getDeltaX() * 0.001f,
                   m_mouse->getDeltaY() * 0.001f);
}

void Scene::render(vk::CommandBuffer commandBuffer) {
  m_ubo.projectionView =
      m_camera->getProjectionMatrix() * m_camera->getViewMatrix();

  auto frameIndex = m_renderer->getFrameIndex();
  FrameData frameData = {
      .commandBuffer = commandBuffer,
      .gameObjects = m_gameObjects,
      .globalDescriptorSet = m_globalDescriptorSets[frameIndex],
  };
  m_globalBuffers[frameIndex]->writeToBuffer(&m_ubo);
  m_globalBuffers[frameIndex]->flush();
  // frameData.gameObjects[0].model =
  //     glm::rotate(frameData.gameObjects[0].model, 0.00005f,
  //                 glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));

  m_chunkRenderSystem->render(frameData);
}