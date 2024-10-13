#include "Image.hpp"
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stdexcept>

Image::Image(const std::string &filename) {
  stbi_set_flip_vertically_on_load(true);
  auto data = stbi_load(filename.c_str(), &m_width, &m_height, &m_channels, 0);
  if (!data) {
    throw std::runtime_error("Failed to load image: " + filename);
  }
  m_data =
      std::vector<unsigned char>(data, data + m_width * m_height * m_channels);
  stbi_image_free(data);
}

Image::Image(const Image &image, uint32_t mipLevel) {
  m_width = std::max(1, image.m_width >> mipLevel);
  m_height = std::max(1, image.m_height >> mipLevel);
  m_channels = image.m_channels;

  m_data.resize(static_cast<size_t>(m_width * m_height * m_channels));
  if (!stbir_resize_uint8_linear(
          image.m_data.data(), image.m_width, image.m_height, 0, m_data.data(),
          m_width, m_height, 0, static_cast<stbir_pixel_layout>(m_channels))) {
    throw std::runtime_error("Failed to generate mipmap level " +
                             std::to_string(mipLevel));
  }
}
