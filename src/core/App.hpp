#pragma once
#include "NonCopyable.hpp"
#include "Window.hpp"
#include <memory>

class App : NonCopyable {
public:
  App();

  void run();

private:
  std::unique_ptr<Window> m_window;
};