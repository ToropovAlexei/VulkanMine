#pragma once

#include "BufferVk.hpp"
#include "RenderDeviceVk.hpp"
#include <string>

class TextureVk : NonCopyable {
public:
  TextureVk(RenderDeviceVk *device, const std::string &filename);
  TextureVk(RenderDeviceVk *device, const std::vector<std::string> &filenames);
  ~TextureVk();

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
  static void loadTextureData(const std::string &filename, int &width,
                              int &height, int &channels, unsigned char *&data);
  void createTextureImage2D();
  void createTextureImage3D();
  void createTextureSampler();
  void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout);
  void copyBufferToImage(BufferVk &buffer, vk::Image image, uint32_t width,
                         uint32_t height, uint32_t depth = 1);
  void createImageView();

private:
  RenderDeviceVk *m_device;
  std::vector<std::string> m_filenames;

  vk::Image m_textureImage = VK_NULL_HANDLE;
  vk::ImageView m_imageView = VK_NULL_HANDLE;
  VmaAllocation m_textureImageAllocation = VK_NULL_HANDLE;
  vk::Sampler m_textureSampler = VK_NULL_HANDLE;
};
