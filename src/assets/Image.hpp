#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Image {
public:
  Image(const std::string &filename);
  Image(const Image &image, uint32_t mipLevel);

  inline int width() const noexcept { return m_width; }
  inline int height() const noexcept { return m_height; }
  inline int channels() const noexcept { return m_channels; }
  inline unsigned char *data() noexcept { return m_data.data(); }
  inline uint32_t size() const noexcept {
    return static_cast<uint32_t>(m_width * m_height * m_channels);
  }

private:
  std::vector<unsigned char> m_data;
  int m_width;
  int m_height;
  int m_channels;
};