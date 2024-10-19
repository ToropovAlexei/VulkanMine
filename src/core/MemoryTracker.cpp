#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#include <cstdlib>

void *operator new(std::size_t count) {
  void *ptr = malloc(count);
  TracyAlloc(ptr, count);
  return ptr;
}

void operator delete(void *ptr) noexcept {
  TracyFree(ptr);
  free(ptr);
}

void *operator new[](std::size_t count) {
  void *ptr = malloc(count);
  TracyAlloc(ptr, count);
  return ptr;
}

void operator delete[](void *ptr) noexcept {
  TracyFree(ptr);
  free(ptr);
}
#endif
