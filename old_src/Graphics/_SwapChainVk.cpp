#include "_SwapChainVk.hpp"

SwapChainVk::SwapChainVk(RenderDeviceVk *renderDevice)
    : m_renderDevice{renderDevice}, m_swapChain{VK_NULL_HANDLE} {}

SwapChainVk::~SwapChainVk() { cleanup(); }

void SwapChainVk::init(vk::Extent2D extent) {
  createSwapChain(extent);
  createImageViews();
}

void SwapChainVk::cleanup() {
  for (auto imageView : m_imageViews) {
    m_renderDevice->getDevice().destroyImageView(imageView);
  }
  if (m_swapChain) {
    m_renderDevice->getDevice().destroySwapchainKHR(m_swapChain);
  }
}

void SwapChainVk::createSwapChain(vk::Extent2D extent) {
  QueueFamilyIndices indices =
      m_renderDevice->findQueueFamilies(m_renderDevice->getPhysicalDevice());

  vk::SurfaceCapabilitiesKHR capabilities =
      m_renderDevice->getPhysicalDevice().getSurfaceCapabilitiesKHR(
          m_renderDevice->getSurface());
  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
      m_renderDevice->getPhysicalDevice().getSurfaceFormatsKHR(
          m_renderDevice->getSurface()));
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(
      m_renderDevice->getPhysicalDevice().getSurfacePresentModesKHR(
          m_renderDevice->getSurface()));
  m_extent = chooseSwapExtent(capabilities, extent);

  uint32_t imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo = {
      .surface = m_renderDevice->getSurface(),
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = m_extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
  };

  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
  }

  createInfo.preTransform = capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  m_swapChain = m_renderDevice->getDevice().createSwapchainKHR(createInfo);

  m_images = m_renderDevice->getDevice().getSwapchainImagesKHR(m_swapChain);
  m_imageFormat = surfaceFormat.format;
}

void SwapChainVk::createImageViews() {
  m_imageViews.resize(m_images.size());

  for (size_t i = 0; i < m_images.size(); i++) {
    vk::ImageViewCreateInfo createInfo = {
        .image = m_images[i],
        .viewType = vk::ImageViewType::e2D,
        .format = m_imageFormat,
        .components = {.r = vk::ComponentSwizzle::eIdentity,
                       .g = vk::ComponentSwizzle::eIdentity,
                       .b = vk::ComponentSwizzle::eIdentity,
                       .a = vk::ComponentSwizzle::eIdentity},
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};

    m_imageViews[i] = m_renderDevice->getDevice().createImageView(createInfo);
  }
}

vk::SurfaceFormatKHR SwapChainVk::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

vk::PresentModeKHR SwapChainVk::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    }
  }
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
SwapChainVk::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities,
                              vk::Extent2D extent) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = extent;
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
    return actualExtent;
  }
}
