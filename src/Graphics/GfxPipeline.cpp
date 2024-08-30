#include "GfxPipeline.h"
#include <fstream>
#include <iostream>

GfxPipeline::GfxPipeline(const std::string &vertFilePath,
                         const std::string &fragFilePath) {
  createGfxPipeline(vertFilePath, fragFilePath);
}

std::vector<char> GfxPipeline::readFile(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

void GfxPipeline::createGfxPipeline(const std::string &vertFilePath,
                                    const std::string &fragFilePath) {
  auto vertShaderCode = readFile(vertFilePath);
  auto fragShaderCode = readFile(fragFilePath);

  std::cout << "Vert Shader Code Size: " << vertShaderCode.size() << std::endl;
  std::cout << "Frag Shader Code Size: " << fragShaderCode.size() << std::endl;
}