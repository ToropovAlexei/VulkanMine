#include "App.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

App::App() {
  ZoneScoped;
  m_window = std::make_unique<Window>(800, 600, "VulkanMine");
  m_renderDevice = std::make_unique<RenderDeviceVk>(m_window.get());
  m_renderer = std::make_unique<Renderer>(m_window.get(), m_renderDevice.get());
  m_keyboard = std::make_unique<Keyboard>(m_window->getGLFWwindow());
  m_mouse = std::make_unique<Mouse>(m_window->getGLFWwindow());
  m_scene =
      std::make_unique<Scene>(m_renderDevice.get(), m_renderer.get(), m_keyboard.get(), m_mouse.get(), m_window.get());
  initImGUI();
}

App::~App() {
  ZoneScoped;
  cleanupImGUI();
}

void App::run() {
  ZoneScoped;
  m_window->hideCursor();

  m_timer.reset();

  while (!m_window->shouldClose()) {
    ZoneScopedN("Main Loop");
    m_keyboard->update();
    m_mouse->update();
    glfwPollEvents();
    m_timer.update();
    float deltaTime = m_timer.getDeltaTime();
    m_scene->update(deltaTime);

    {
      ZoneScopedN("Prepare UI");
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      m_scene->renderUI();
      ImGui::Render();
    }

    if (auto commandBuffer = m_renderer->beginFrame()) {
      m_renderer->beginSwapChainRenderPass(commandBuffer);
      m_scene->render(commandBuffer);
      {
        ZoneScopedN("Render UI");
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
      }
      m_renderer->endSwapChainRenderPass(commandBuffer);
      m_renderer->endFrame();
    }
  }

  m_renderDevice->getDevice().waitIdle();
}

void App::initImGUI() {
  ZoneScoped;
  vk::DescriptorPoolSize pool_sizes[] = {{vk::DescriptorType::eSampler, 1000},
                                         {vk::DescriptorType::eCombinedImageSampler, 1000},
                                         {vk::DescriptorType::eSampledImage, 1000},
                                         {vk::DescriptorType::eStorageImage, 1000},
                                         {vk::DescriptorType::eUniformTexelBuffer, 1000},
                                         {vk::DescriptorType::eStorageTexelBuffer, 1000},
                                         {vk::DescriptorType::eUniformBuffer, 1000},
                                         {vk::DescriptorType::eStorageBuffer, 1000},
                                         {vk::DescriptorType::eUniformBufferDynamic, 1000},
                                         {vk::DescriptorType::eStorageBufferDynamic, 1000},
                                         {vk::DescriptorType::eInputAttachment, 1000}};

  vk::DescriptorPoolCreateInfo pool_info = {};
  pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  pool_info.maxSets = 1000 * static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
  pool_info.poolSizeCount = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
  pool_info.pPoolSizes = pool_sizes;

  m_imGuiDescriptorPool = m_renderDevice->getDevice().createDescriptorPool(pool_info);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForVulkan(m_window->getGLFWwindow(), true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = m_renderDevice->getInstance();
  init_info.PhysicalDevice = m_renderDevice->getPhysicalDevice();
  init_info.Device = m_renderDevice->getDevice();
  init_info.QueueFamily = m_renderDevice->findQueueFamilies().graphicsFamily.value();
  init_info.Queue = m_renderDevice->getGraphicsQueue();
  init_info.PipelineCache = nullptr;
  init_info.DescriptorPool = m_imGuiDescriptorPool;
  init_info.RenderPass = m_renderer->getSwapChainRenderPass();
  init_info.Subpass = 0;
  init_info.MinImageCount = SwapChainVk::MAX_FRAMES_IN_FLIGHT;
  init_info.ImageCount = SwapChainVk::MAX_FRAMES_IN_FLIGHT;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = [](VkResult err) {
    if (err != VK_SUCCESS) {
      throw std::runtime_error("Vulkan error in ImGui!");
    }
  };
  ImGui_ImplVulkan_Init(&init_info);
}

void App::cleanupImGUI() {
  ZoneScoped;
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  m_renderDevice->getDevice().destroyDescriptorPool(m_imGuiDescriptorPool);
}
