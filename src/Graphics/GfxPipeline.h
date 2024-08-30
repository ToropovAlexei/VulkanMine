#pragma once

#include <string>
#include <vector>

class GfxPipeline {
public:
  GfxPipeline(const std::string &vertFilePath, const std::string &fragFilePath);

private:
  static std::vector<char> readFile(const std::string &filePath);

  void createGfxPipeline(const std::string &vertFilePath,
                         const std::string &fragFilePath);
};