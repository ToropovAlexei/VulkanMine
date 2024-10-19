#include "core/App.hpp"
#include <cstdlib>
#include <iostream>
#include <tracy/Tracy.hpp>

int main() {
  ZoneScoped;
  try {
    App{}.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}