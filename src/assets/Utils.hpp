#pragma once

#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif
#include <filesystem>

static std::filesystem::path getExecutablePath() {
  char buffer[1024];

#if defined(_WIN32)
  DWORD size = GetModuleFileNameA(NULL, buffer, sizeof(buffer));
  if (size == 0) {
    std::cerr << "Error getting executable path on Windows" << std::endl;
    return "";
  }
#elif defined(__linux__)
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len == -1) {
    std::cerr << "Error getting executable path on Linux" << std::endl;
    return "";
  }
#else
#error Unsupported platform
#endif

  buffer[len] = '\0';
  return std::filesystem::path(buffer);
}

static std::filesystem::path getExecutableDir() { return getExecutablePath().parent_path(); }

static std::filesystem::path getShadersPath() { return getExecutableDir() / "shaders"; }

static std::filesystem::path getResourcesPath() { return getExecutableDir() / "res"; }

static std::filesystem::path getBlocksPath() { return getResourcesPath() / "blocks"; }

static std::filesystem::path getTexturesPath() { return getResourcesPath() / "textures"; }