#include "SwapChainVk.hpp"

#include "../../core/Logger.hpp"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>

SwapChainVk::SwapChainVk(RenderDeviceVk *device, vk::Extent2D extent)
    : m_device{device}, m_windowExtent{extent} {
  init();
}

SwapChainVk::SwapChainVk(RenderDeviceVk *device, vk::Extent2D extent,
                         std::shared_ptr<SwapChainVk> previousSwapChain)
    : m_device{device}, m_windowExtent{extent},
      m_oldSwapChain{previousSwapChain} {
  init();

  m_oldSwapChain = nullptr;
}

void SwapChainVk::init() {
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

SwapChainVk::~SwapChainVk() {
  for (auto imageView : m_swapChainImageViews) {
    m_device->getDevice().destroyImageView(imageView);
  }
  m_swapChainImageViews.clear();

  if (m_swapChain != nullptr) {
    m_device->getDevice().destroySwapchainKHR(m_swapChain);
    m_swapChain = nullptr;
  }

  for (size_t i = 0; i < m_depthImages.size(); i++) {
    m_device->getDevice().destroyImageView(m_depthImageViews[i]);
    vmaDestroyImage(m_device->getAllocator(), m_depthImages[i],
                    m_depthImageMemorys[i]);
  }

  for (auto framebuffer : m_swapChainFramebuffers) {
    m_device->getDevice().destroyFramebuffer(framebuffer);
  }

  m_device->getDevice().destroyRenderPass(m_renderPass);

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    m_device->getDevice().destroySemaphore(m_renderFinishedSemaphores[i]);
    m_device->getDevice().destroySemaphore(m_imageAvailableSemaphores[i]);
    m_device->getDevice().destroyFence(m_inFlightFences[i]);
  }
}

vk::Result SwapChainVk::acquireNextImage(uint32_t *imageIndex) {
  auto waitResult = m_device->getDevice().waitForFences(
      1, &m_inFlightFences[m_currentFrame], VK_TRUE,
      std::numeric_limits<uint64_t>::max());

  if (waitResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to wait for fence!");
  }

  auto result = m_device->getDevice().acquireNextImageKHR(
      m_swapChain, std::numeric_limits<uint64_t>::max(),
      m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, imageIndex);

  return result;
}

vk::Result SwapChainVk::submitCommandBuffers(const vk::CommandBuffer *buffers,
                                             uint32_t *imageIndex) {
  if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    auto result = m_device->getDevice().waitForFences(
        1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
      throw std::runtime_error("failed to wait for fence!");
    }
  }
  m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

  vk::SubmitInfo submitInfo = {};

  vk::Semaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
  vk::PipelineStageFlags waitStages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  vk::Semaphore signalSemaphores[] = {
      m_renderFinishedSemaphores[m_currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  auto resetResult =
      m_device->getDevice().resetFences(1, &m_inFlightFences[m_currentFrame]);

  if (resetResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to reset fence!");
  }

  m_device->getGraphicsQueue().submit(submitInfo,
                                      m_inFlightFences[m_currentFrame]);

  vk::PresentInfoKHR presentInfo = {
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphores,
  };

  vk::SwapchainKHR swapChains[] = {m_swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = imageIndex;

  auto result = m_device->getPresentQueue().presentKHR(presentInfo);

  m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void SwapChainVk::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = m_device->getSwapChainSupport();

  vk::SurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo = {
      .surface = m_device->getSurface(),
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment};

  QueueFamilyIndices indices = m_device->findQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain =
      m_oldSwapChain ? m_oldSwapChain->m_swapChain : VK_NULL_HANDLE;

  m_swapChain = m_device->getDevice().createSwapchainKHR(createInfo);

  // we only specified a minimum number of images in the swap chain, so the
  // implementation is allowed to create a swap chain with more. That's why
  // we'll first query the final number of images with vkGetSwapchainImagesKHR,
  // then resize the container and finally call it again to retrieve the
  // handles.
  m_swapChainImages = m_device->getDevice().getSwapchainImagesKHR(m_swapChain);

  m_swapChainImageFormat = surfaceFormat.format;
  m_swapChainExtent = extent;
}

void SwapChainVk::createImageViews() {
  m_swapChainImageViews.resize(m_swapChainImages.size());
  for (size_t i = 0; i < m_swapChainImages.size(); i++) {
    vk::ImageViewCreateInfo viewInfo{
        .image = m_swapChainImages[i],
        .viewType = vk::ImageViewType::e2D,
        .format = m_swapChainImageFormat,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};

    m_swapChainImageViews[i] = m_device->getDevice().createImageView(viewInfo);
  }
}

void SwapChainVk::createRenderPass() {
  vk::AttachmentDescription depthAttachment = {
      .format = findDepthFormat(),
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::AttachmentReference depthAttachmentRef = {
      .attachment = 1,
      .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::AttachmentDescription colorAttachment = {
      .format = getSwapChainImageFormat(),
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::ePresentSrcKHR};

  vk::AttachmentReference colorAttachmentRef = {
      .attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpass = {
      .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
      .pDepthStencilAttachment = &depthAttachmentRef,
  };

  vk::SubpassDependency dependency = {
      .srcSubpass = vk::SubpassExternal,
      .dstSubpass = 0,
      .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                      vk::PipelineStageFlagBits::eEarlyFragmentTests,
      .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                      vk::PipelineStageFlagBits::eEarlyFragmentTests,
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
                       vk::AccessFlagBits::eDepthStencilAttachmentWrite};

  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment,
                                                          depthAttachment};
  vk::RenderPassCreateInfo renderPassInfo = {
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency};

  m_renderPass = m_device->getDevice().createRenderPass(renderPassInfo);
}

void SwapChainVk::createFramebuffers() {
  m_swapChainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<vk::ImageView, 2> attachments = {m_swapChainImageViews[i],
                                                m_depthImageViews[i]};

    vk::Extent2D swapChainExtent = getSwapChainExtent();
    vk::FramebufferCreateInfo framebufferInfo = {
        .renderPass = m_renderPass,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = swapChainExtent.width,
        .height = swapChainExtent.height,
        .layers = 1};

    m_swapChainFramebuffers[i] =
        m_device->getDevice().createFramebuffer(framebufferInfo);
  }
}

void SwapChainVk::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  m_swapChainDepthFormat = depthFormat;
  vk::Extent2D swapChainExtent = getSwapChainExtent();

  m_depthImages.resize(imageCount());
  m_depthImageMemorys.resize(imageCount());
  m_depthImageViews.resize(imageCount());

  for (size_t i = 0; i < m_depthImages.size(); i++) {
    vk::ImageCreateInfo imageInfo = {
        .imageType = vk::ImageType::e2D,
        .format = depthFormat,
        .extent = {.width = swapChainExtent.width,
                   .height = swapChainExtent.height,
                   .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    m_device->createImageWithInfo(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY,
                                  m_depthImages[i], m_depthImageMemorys[i]);

    vk::ImageViewCreateInfo viewInfo = {
        .image = m_depthImages[i],
        .viewType = vk::ImageViewType::e2D,
        .format = depthFormat,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eDepth,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};

    m_depthImageViews[i] = m_device->getDevice().createImageView(viewInfo);
  }
}

void SwapChainVk::createSyncObjects() {
  m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo semaphoreInfo = {};

  vk::FenceCreateInfo fenceInfo = {.flags = vk::FenceCreateFlagBits::eSignaled};

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    m_imageAvailableSemaphores[i] =
        m_device->getDevice().createSemaphore(semaphoreInfo);
    m_renderFinishedSemaphores[i] =
        m_device->getDevice().createSemaphore(semaphoreInfo);
    m_inFlightFences[i] = m_device->getDevice().createFence(fenceInfo);
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
      Logger::info("Present mode: Mailbox");
      return availablePresentMode;
    }
  }

  Logger::info("Present mode: V-Sync");
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
SwapChainVk::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = m_windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

vk::Format SwapChainVk::findDepthFormat() {
  return m_device->findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
       vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
