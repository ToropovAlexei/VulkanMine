#include "TextureVk.hpp"
#include "BufferVk.hpp"
#include <stb_image.h>
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
  createTextureImage3D();
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
  int width, height, channels;
  unsigned char *data = nullptr;
  loadTextureData(m_filenames[0], width, height, channels, data);

  // Создание текстурного изображения
  vk::ImageCreateInfo imageInfo{};
  imageInfo.setImageType(vk::ImageType::e2D)
      .setFormat(vk::Format::eR8G8B8A8Srgb)
      .setExtent(vk::Extent3D{static_cast<uint32_t>(width),
                              static_cast<uint32_t>(height), 1})
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
  vk::DeviceSize bufferSize =
      static_cast<vk::DeviceSize>(width * height * channels);
  BufferVk stagingBuffer(m_device, bufferSize, 1,
                         vk::BufferUsageFlagBits::eTransferSrc,
                         VMA_MEMORY_USAGE_CPU_ONLY);

  stagingBuffer.map();
  stagingBuffer.writeToBuffer(data);
  stagingBuffer.unmap(); // OPTIONAL as unmapped when destroying

  copyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height));

  transitionImageLayout(m_textureImage, vk::ImageLayout::eTransferDstOptimal,
                        vk::ImageLayout::eShaderReadOnlyOptimal);

  stbi_image_free(data);
}

void TextureVk::createTextureImage3D() {
  std::vector<unsigned char *> dataLayers;
  int width, height, channels;

  // Загрузка каждой 2D текстуры
  for (const auto &filename : m_filenames) {
    unsigned char *data = nullptr;
    loadTextureData(filename, width, height, channels, data);
    if (!data) {
      throw std::runtime_error("Failed to load texture image: " + filename);
    }
    dataLayers.push_back(data);
  }

  // Создание текстурного изображения
  vk::ImageCreateInfo imageInfo{};
  imageInfo.setImageType(vk::ImageType::e3D)
      .setFormat(vk::Format::eR8G8B8A8Srgb)
      .setExtent(vk::Extent3D{static_cast<uint32_t>(width),
                              static_cast<uint32_t>(height),
                              static_cast<uint32_t>(dataLayers.size())})
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
  vk::DeviceSize bufferSize =
      static_cast<vk::DeviceSize>(width * height * channels) *
      static_cast<vk::DeviceSize>(dataLayers.size());
  BufferVk stagingBuffer(m_device, bufferSize, 1,
                         vk::BufferUsageFlagBits::eTransferSrc,
                         VMA_MEMORY_USAGE_CPU_ONLY);

  stagingBuffer.map();
  vk::DeviceSize size = static_cast<vk::DeviceSize>(width * height * 4);
  for (size_t i = 0; i < dataLayers.size(); i++) {
    stagingBuffer.writeToBuffer(dataLayers[i], size, i * size);
    stbi_image_free(dataLayers[i]); // Освобождение памяти после копирования
  }
  stagingBuffer.unmap(); // OPTIONAL as unmapped when destroying

  copyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height),
                    static_cast<uint32_t>(dataLayers.size()));

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
      .setMaxLod(0.0f);

  m_textureSampler = m_device->getDevice().createSampler(samplerInfo);
}

void TextureVk::loadTextureData(const std::string &filename, int &width,
                                int &height, int &channels,
                                unsigned char *&data) {
  data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
  if (!data) {
    throw std::runtime_error("Failed to load texture image: " + filename);
  }
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
      .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

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
                                          : vk::ImageViewType::e3D,
      .format = vk::Format::eR8G8B8A8Srgb,
      .components = {vk::ComponentSwizzle::eIdentity,
                     vk::ComponentSwizzle::eIdentity,
                     vk::ComponentSwizzle::eIdentity,
                     vk::ComponentSwizzle::eIdentity},
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  m_imageView = m_device->getDevice().createImageView(viewInfo);
}
