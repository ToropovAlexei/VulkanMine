#include "App.h"
#include <iostream>
// #include <tracy/Tracy.hpp>

int main() {
  try {
    App{}.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}