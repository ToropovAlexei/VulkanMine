#pragma once

#include "RenderDeviceVk.hpp"
#include <string>

class TextureVk {
public:
  TextureVk(RenderDeviceVk *device, const std::string &filePath);
  ~TextureVk();

  void createTextureImage();
  void createTextureSampler();

  vk::Image getImage() const { return m_textureImage; }
  vk::Sampler getSampler() const { return m_textureSampler; }
  VmaAllocation getAllocation() const { return m_textureImageAllocation; }
  vk::DescriptorImageInfo getDescriptorInfo() const {
    return vk::DescriptorImageInfo{
        .sampler = m_textureSampler,
        .imageView = m_imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
    };
  }

private:
  void loadTextureData(int &width, int &height, int &channels,
                       unsigned char *&data);
  void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout);
  void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width,
                         uint32_t height);
  void createImageView();

private:
  RenderDeviceVk *m_device;
  std::string m_filename;

  vk::Image m_textureImage = VK_NULL_HANDLE;
  vk::ImageView m_imageView = VK_NULL_HANDLE;
  VmaAllocation m_textureImageAllocation = VK_NULL_HANDLE;
  vk::Sampler m_textureSampler = VK_NULL_HANDLE;
};
