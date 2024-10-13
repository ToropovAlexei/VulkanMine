#pragma once

#include <string>

class AssetsLoader {
public:
  AssetsLoader(const std::string &assetsPath);

  inline std::string getAssetsPath() noexcept { return m_assetsPath; }

private:
  std::string m_assetsPath;
};