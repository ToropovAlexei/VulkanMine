#include "core/App.hpp"
#include <cstdlib>
#include <iostream>

int main() {
  try {
    App{}.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}