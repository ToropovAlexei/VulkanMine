#include "RenderDeviceVk.hpp"
#include "../../core/Logger.hpp"
#include <iostream>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    //"VK_KHR_get_physical_device_properties2",
    "VK_KHR_maintenance1", "VK_KHR_maintenance3", "VK_KHR_draw_indirect_count",
    "VK_KHR_shader_subgroup_extended_types", "VK_EXT_descriptor_indexing",
    "VK_EXT_sampler_filter_minmax", "VK_EXT_host_query_reset",
    "VK_KHR_shader_float16_int8", "VK_KHR_shader_atomic_int64"};

#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    Logger::error(pCallbackData->pMessage);
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    Logger::warning(pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    Logger::info(pCallbackData->pMessage);
  } else {
    Logger::info(pCallbackData->pMessage);
  }

  return VK_FALSE;
}

RenderDeviceVk::RenderDeviceVk(Window *window) : m_window{window} {
  initVulkan();
  setupDebugMessenger();
  pickPhysicalDevice();
  createLogicalDevice();
  createAllocator();
  createCommandPool();
}

RenderDeviceVk::~RenderDeviceVk() {
  if (m_debugMessenger) {
    m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, dldi);
  }
  m_instance.destroy();
}

void RenderDeviceVk::initVulkan() {
#ifndef NDEBUG
  checkValidationLayerSupport();
#endif

  vk::ApplicationInfo appInfo = {
      .pApplicationName = "VulkanMine",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "VulkanMineEngine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  vk::InstanceCreateInfo createInfo = {
      .pApplicationInfo = &appInfo,
  };

  auto requiredExtensions = getRequiredExtensions();
  auto remainingRequiredExtensions = requiredExtensions;
  if (!checkInstanceExtensionSupport(remainingRequiredExtensions)) {
    for (auto &extension : remainingRequiredExtensions) {
      std::cerr << "Missing required extension: " << extension << std::endl;
    }

    throw std::runtime_error("Required extensions are missing!");
  }

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(requiredExtensions.size());
  createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifndef NDEBUG
  vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
  createInfo.ppEnabledLayerNames = validationLayers.data();
  populateDebugMessengerCreateInfo(debugCreateInfo);
  createInfo.pNext = &debugCreateInfo;
#endif

  m_instance = vk::createInstance(createInfo);
}

void RenderDeviceVk::setupDebugMessenger() {
#ifndef NDEBUG
  vk::DebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  dldi = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);

  m_debugMessenger =
      m_instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
#endif
}

void RenderDeviceVk::pickPhysicalDevice() {}

void RenderDeviceVk::createLogicalDevice() {}

void RenderDeviceVk::createAllocator() {}

void RenderDeviceVk::createCommandPool() {}

void RenderDeviceVk::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }
  }
}

bool RenderDeviceVk::checkInstanceExtensionSupport(
    std::vector<const char *> &requiredExtensions) {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());

  for (auto extension : extensions) {
    requiredExtensions.erase(
        std::remove_if(requiredExtensions.begin(), requiredExtensions.end(),
                       [&extension](const char *requiredExtension) {
                         return std::strcmp(requiredExtension,
                                            extension.extensionName) == 0;
                       }),
        requiredExtensions.end());
  }

  return requiredExtensions.empty();
}

std::vector<const char *> RenderDeviceVk::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions.push_back("VK_KHR_get_physical_device_properties2");
  extensions.push_back("VK_EXT_debug_report");
#endif

  return extensions;
};

void RenderDeviceVk::populateDebugMessengerCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo.messageSeverity =
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  createInfo.pfnUserCallback = debugCallback;
}