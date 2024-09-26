#include "RenderDeviceVk.hpp"
#include "../../core/Logger.hpp"
#include <map>
#include <set>
#include <string>
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
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createAllocator();
  createCommandPool();
}

RenderDeviceVk::~RenderDeviceVk() {
  m_device.destroy();

  if (m_debugMessenger) {
    m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, dldi);
  }

  m_instance.destroySurfaceKHR(m_surface);
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
      Logger::error("Missing required extension: " + std::string(extension));
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

void RenderDeviceVk::createSurface() {
  m_window->createWindowSurface(m_instance, m_surface);
}

void RenderDeviceVk::pickPhysicalDevice() {
  auto devices = m_instance.enumeratePhysicalDevices();

  // Use an ordered map to automatically sort candidates by increasing score
  std::multimap<int, vk::PhysicalDevice> candidates;

  for (const auto &device : devices) {
    int score = rateDeviceSuitability(device);
    candidates.insert(std::make_pair(score, device));
  }

  // Check if the best candidate is suitable at all
  if (candidates.rbegin()->first > 0) {
    m_physicalDevice = candidates.rbegin()->second;
  } else {
    throw std::runtime_error("Failed to find a suitable GPU!");
  }
}

void RenderDeviceVk::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.transferFamily.value(),
                                            indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    queueCreateInfos.push_back({.queueFamilyIndex = queueFamily,
                                .queueCount = 1,
                                .pQueuePriorities = &queuePriority});
  }

  vk::PhysicalDeviceHostQueryResetFeaturesEXT resetFeatures = {.hostQueryReset =
                                                                   VK_TRUE};

  vk::PhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR
      shaderSubgroupFeatures = {
          .pNext = &resetFeatures,
          .shaderSubgroupExtendedTypes = VK_TRUE,
      };

  vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {
      .pNext = &shaderSubgroupFeatures,
      .runtimeDescriptorArray = VK_TRUE,
      .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
  };

  vk::PhysicalDeviceShaderAtomicInt64FeaturesKHR atomicInt64Features = {
      .pNext = &descriptorIndexingFeatures,
      .shaderBufferInt64Atomics = VK_TRUE,
  };

  vk::PhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features = {
      .pNext = &atomicInt64Features,
      .shaderFloat16 = VK_TRUE,
  };

  vk::PhysicalDeviceFeatures2 deviceFeatures = {
      .pNext = &float16Int8Features,
      .features = {.samplerAnisotropy = VK_TRUE,
                   .fragmentStoresAndAtomics = VK_TRUE,
                   .vertexPipelineStoresAndAtomics = VK_TRUE,
                   .shaderInt64 = VK_TRUE,
                   .multiDrawIndirect = VK_TRUE,
                   .drawIndirectFirstInstance = VK_TRUE,
                   .independentBlend = VK_TRUE,
                   .geometryShader = VK_TRUE,
                   .fillModeNonSolid = VK_TRUE,
                   .depthClamp = VK_TRUE,
                   .shaderStorageImageReadWithoutFormat = VK_TRUE,
                   .shaderImageGatherExtended = VK_TRUE},
  };
  // TODO Check if device features are supported
  // checkDeviceFeatureSupport(m_physicalDevice, deviceFeatures);

  vk::DeviceCreateInfo createInfo = {
      .pNext = &deviceFeatures,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .pEnabledFeatures = NULL,
  };

  std::vector<const char *> enabledExtensions;
  for (const char *extension : deviceExtensions) {
    enabledExtensions.push_back(extension);
  }

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(enabledExtensions.size());
  createInfo.ppEnabledExtensionNames = enabledExtensions.data();

#ifndef NDEBUG
  std::vector<const char *> enabledLayers;
  for (const char *layer : validationLayers) {
    enabledLayers.push_back(layer);
  }

  createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
  createInfo.ppEnabledLayerNames = enabledLayers.data();
#else
  createInfo.enabledLayerCount = 0;
#endif

  m_device = m_physicalDevice.createDevice(createInfo);
  m_graphicsQueue = m_device.getQueue(indices.graphicsFamily.value(), 0);
  m_transferQueue = m_device.getQueue(indices.transferFamily.value(), 0);
  m_presentQueue = m_device.getQueue(indices.presentFamily.value(), 0);
}

void RenderDeviceVk::createAllocator() {}

void RenderDeviceVk::createCommandPool() {}

void RenderDeviceVk::checkValidationLayerSupport() {
  auto availableLayers = vk::enumerateInstanceLayerProperties();

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
  auto extensions = vk::enumerateInstanceExtensionProperties();

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

int RenderDeviceVk::rateDeviceSuitability(const vk::PhysicalDevice &device) {
  vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
  vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

  int score = 0;

  // Discrete GPUs have a significant performance advantage
  if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    score += 1000;
  }

  // Maximum possible size of textures affects graphics quality
  score += deviceProperties.limits.maxImageDimension2D;

  // Make sure it supports the extensions we need
  bool extensionsSupported = checkDeviceExtensionSupport(device);
  if (!extensionsSupported) {
    return 0;
  }

  return score;
}

bool RenderDeviceVk::checkDeviceExtensionSupport(
    const vk::PhysicalDevice &device) {
  auto availableExtensions = device.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices
RenderDeviceVk::findQueueFamilies(const vk::PhysicalDevice device) {
  QueueFamilyIndices indices;

  auto queueFamilies = device.getQueueFamilyProperties();

  vk::QueueFlags transferQueueFlags = vk::QueueFlagBits::eTransfer;

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 &&
        queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = i;
      indices.graphicsFamilySupportsTimeStamps =
          queueFamily.timestampValidBits > 0;
    }

    if (queueFamily.queueCount > 0 &&
        (queueFamily.queueFlags & transferQueueFlags) == transferQueueFlags) {
      indices.transferFamily = i;
      indices.transferFamilySupportsTimeStamps =
          queueFamily.timestampValidBits > 0;
    }

    VkBool32 presentSupport = device.getSurfaceSupportKHR(i, m_surface);

    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilySupportsTimeStamps =
          queueFamily.timestampValidBits > 0;
    }

    if (indices.IsComplete()) {
      break;
    }

    i++;
  }

  return indices;
}