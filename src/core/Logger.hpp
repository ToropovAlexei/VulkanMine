#pragma once

#include <mutex>
#include <string>

#ifndef NDEBUG
enum class LogLevel { ERROR, WARNING, INFO, DEBUG };

class Logger {
public:
  static void error(const std::string &message);
  static void warning(const std::string &message);
  static void info(const std::string &message);
  static void debug(const std::string &message);

private:
  static void log(LogLevel level, const std::string &message);
  static std::string formatLog(LogLevel level, const std::string &message);
  static std::string getLevelString(LogLevel level);
  static std::string getCurrentTime();

private:
  static std::mutex m_mutex;
};
#endif