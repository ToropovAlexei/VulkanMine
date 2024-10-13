#include "TextureVk.hpp"
#include "../../assets/Image.hpp"
#include "BufferVk.hpp"
#include <cmath>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

TextureVk::TextureVk(RenderDeviceVk *device, const std::string &filename)
    : m_device(device), m_filenames({filename}) {
  createTextureImage2D();
  createTextureSampler();
  createImageView();
}

TextureVk::TextureVk(RenderDeviceVk *device,
                     const std::vector<std::string> &filenames)
    : m_device{device}, m_filenames{filenames} {
  createTextureImage2DArrayWithMipmaps();
  createTextureSampler();
  createImageView();
}

TextureVk::~TextureVk() {
  if (m_imageView) {
    m_device->getDevice().destroyImageView(m_imageView);
  }
  if (m_textureImage) {
    vmaDestroyImage(m_device->getAllocator(), m_textureImage,
                    m_textureImageAllocation);
  }
  if (m_textureSampler) {
    m_device->getDevice().destroySampler(m_textureSampler);
  }
}

void TextureVk::createTextureImage2D() {
  Image img{m_filenames[0]};

  // Создание текстурного изображения
  vk::ImageCreateInfo imageInfo{};
  imageInfo.setImageType(vk::ImageType::e2D)
      .setFormat(vk::Format::eR8G8B8A8Srgb)
      .setExtent(vk::Extent3D{static_cast<uint32_t>(img.width()),
                              static_cast<uint32_t>(img.height()), 1})
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setTiling(vk::ImageTiling::eOptimal)
      .setUsage(vk::ImageUsageFlagBits::eTransferDst |
                vk::ImageUsageFlagBits::eSampled);

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  if (vmaCreateImage(m_device->getAllocator(),
                     reinterpret_cast<const VkImageCreateInfo *>(&imageInfo),
                     &allocCreateInfo,
                     reinterpret_cast<VkImage *>(&m_textureImage),
                     &m_textureImageAllocation, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image!");
  }

  // Копирование данных в изображение
  transitionImageLayout(m_textureImage, vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eTransferDstOptimal);

  // Создание временного буфера для загрузки текстуры
  vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(img.size());
  BufferVk stagingBuffer(m_device, bufferSize, 1,
                         vk::BufferUsageFlagBits::eTransferSrc,
                         VMA_MEMORY_USAGE_CPU_ONLY);

  stagingBuffer.map();
  stagingBuffer.writeToBuffer(img.data());
  stagingBuffer.unmap(); // OPTIONAL as unmapped when destroying

  copyBufferToImage(stagingBuffer, m_textureImage,
                    static_cast<uint32_t>(img.width()),
                    static_cast<uint32_t>(img.height()));

  transitionImageLayout(m_textureImage, vk::ImageLayout::eTransferDstOptimal,
                        vk::ImageLayout::eShaderReadOnlyOptimal);
}

void TextureVk::createTextureSampler() {
  vk::SamplerCreateInfo samplerInfo{};
  samplerInfo.setMagFilter(vk::Filter::eNearest)
      .setMinFilter(vk::Filter::eLinear)
      .setAddressModeU(vk::SamplerAddressMode::eRepeat)
      .setAddressModeV(vk::SamplerAddressMode::eRepeat)
      .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
      .setAnisotropyEnable(VK_TRUE)
      .setMaxAnisotropy(16)
      .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
      .setUnnormalizedCoordinates(VK_FALSE)
      .setCompareEnable(VK_FALSE)
      .setCompareOp(vk::CompareOp::eAlways)
      .setMipmapMode(vk::SamplerMipmapMode::eLinear)
      .setMinLod(0.0f)
      .setMaxLod(static_cast<float>(m_mipLevels))
      .setMipLodBias(0.0f);

  m_textureSampler = m_device->getDevice().createSampler(samplerInfo);
}

void TextureVk::transitionImageLayout(vk::Image image,
                                      vk::ImageLayout oldLayout,
                                      vk::ImageLayout newLayout) {
  // Создание командного буфера
  vk::CommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

  // Установка барьеров (pipeline barriers)
  vk::ImageMemoryBarrier barrier{};
  barrier.setOldLayout(oldLayout)
      .setNewLayout(newLayout)
      .setImage(image)
      .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, m_mipLevels, 0,
                            static_cast<uint32_t>(m_filenames.size())});

  // Определение нужных флагов
  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined &&
      newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  }
  // Другие переходы можно добавить по мере необходимости

  commandBuffer.pipelineBarrier(sourceStage, destinationStage,
                                vk::DependencyFlags{}, nullptr, nullptr,
                                barrier);

  // Завершение командного буфера
  m_device->endSingleTimeCommands(commandBuffer);
}

void TextureVk::copyBufferToImage(BufferVk &buffer, vk::Image image,
                                  uint32_t width, uint32_t height,
                                  uint32_t depth) {
  vk::CommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

  vk::BufferImageCopy region{};
  region.setBufferOffset(0)
      .setBufferRowLength(0)
      .setBufferImageHeight(0)
      .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
      .setImageOffset({0, 0, 0})
      .setImageExtent({width, height, depth});

  commandBuffer.copyBufferToImage(buffer.getBuffer(), image,
                                  vk::ImageLayout::eTransferDstOptimal, region);

  m_device->endSingleTimeCommands(commandBuffer);
}

void TextureVk::createImageView() {
  vk::ImageViewCreateInfo viewInfo = {
      .image = m_textureImage,
      .viewType = m_filenames.size() == 1 ? vk::ImageViewType::e2D
                                          : vk::ImageViewType::e2DArray,
      .format = vk::Format::eR8G8B8A8Srgb,
      .components = {vk::ComponentSwizzle::eIdentity,
                     vk::ComponentSwizzle::eIdentity,
                     vk::ComponentSwizzle::eIdentity,
                     vk::ComponentSwizzle::eIdentity},
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = m_mipLevels,
                           .baseArrayLayer = 0,
                           .layerCount =
                               static_cast<uint32_t>(m_filenames.size())}};

  m_imageView = m_device->getDevice().createImageView(viewInfo);
}

uint32_t TextureVk::calculateMipLevels(uint32_t width, uint32_t height) {
  return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) +
         1;
}

void TextureVk::createTextureImage2DArrayWithMipmaps() {
  std::vector<std::vector<Image>> mipDataLayers;

  for (const auto &filename : m_filenames) {
    Image img{filename};

    std::vector<Image> mipmaps;
    mipmaps.push_back(img);
    m_mipLevels = calculateMipLevels(static_cast<uint32_t>(img.width()),
                                     static_cast<uint32_t>(img.height()));
    for (uint32_t mip = 1; mip < m_mipLevels; mip++) {
      mipmaps.push_back({img, mip});
    }
    mipDataLayers.push_back(mipmaps);
  }

  uint32_t width = static_cast<uint32_t>(mipDataLayers[0][0].width());
  uint32_t height = static_cast<uint32_t>(mipDataLayers[0][0].height());
  int channels = mipDataLayers[0][0].channels();

  vk::ImageCreateInfo imageInfo{};
  imageInfo.setImageType(vk::ImageType::e2D)
      .setFormat(vk::Format::eR8G8B8A8Srgb)
      .setExtent(vk::Extent3D{width, height, 1})
      .setMipLevels(m_mipLevels)
      .setArrayLayers(static_cast<uint32_t>(m_filenames.size()))
      .setSamples(vk::SampleCountFlagBits::e1)
      .setTiling(vk::ImageTiling::eOptimal)
      .setUsage(vk::ImageUsageFlagBits::eTransferDst |
                vk::ImageUsageFlagBits::eSampled);

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  if (vmaCreateImage(m_device->getAllocator(),
                     reinterpret_cast<const VkImageCreateInfo *>(&imageInfo),
                     &allocCreateInfo,
                     reinterpret_cast<VkImage *>(&m_textureImage),
                     &m_textureImageAllocation, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image!");
  }

  transitionImageLayout(m_textureImage, vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eTransferDstOptimal);

  for (size_t layer = 0; layer < mipDataLayers.size(); layer++) {
    for (uint32_t mip = 0; mip < m_mipLevels; mip++) {
      uint32_t mipWidth = std::max(1u, static_cast<uint32_t>(width >> mip));
      uint32_t mipHeight = std::max(1u, static_cast<uint32_t>(height >> mip));

      vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(
          mipWidth * mipHeight * static_cast<uint32_t>(channels));
      BufferVk stagingBuffer(m_device, bufferSize, 1,
                             vk::BufferUsageFlagBits::eTransferSrc,
                             VMA_MEMORY_USAGE_CPU_ONLY);

      stagingBuffer.map();
      stagingBuffer.writeToBuffer(mipDataLayers[layer][mip].data(), bufferSize,
                                  0);
      stagingBuffer.unmap();

      vk::BufferImageCopy region{};
      region.setBufferOffset(0)
          .setBufferRowLength(0)
          .setBufferImageHeight(0)
          .setImageSubresource({vk::ImageAspectFlagBits::eColor, mip,
                                static_cast<uint32_t>(layer), 1})
          .setImageOffset({0, 0, 0})
          .setImageExtent({mipWidth, mipHeight, 1});

      vk::CommandBuffer commandBuffer = m_device->beginSingleTimeCommands();
      commandBuffer.copyBufferToImage(stagingBuffer.getBuffer(), m_textureImage,
                                      vk::ImageLayout::eTransferDstOptimal,
                                      region);
      m_device->endSingleTimeCommands(commandBuffer);
    }
  }

  transitionImageLayout(m_textureImage, vk::ImageLayout::eTransferDstOptimal,
                        vk::ImageLayout::eShaderReadOnlyOptimal);
}