#pragma once

#ifndef NDEBUG
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

enum class LogLevel { ERROR, WARNING, INFO, DEBUG };

class Logger {
public:
  static void error(const std::string &message) {
    log(LogLevel::ERROR, message);
  };
  static void warning(const std::string &message) {
    log(LogLevel::WARNING, message);
  };
  static void info(const std::string &message) {
    log(LogLevel::INFO, message);
  };
  static void debug(const std::string &message) {
    log(LogLevel::DEBUG, message);
  };

private:
  static void log(LogLevel level, const std::string &message) {
    std::string output = formatLog(level, message);

    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << output << std::endl;
  };
  static std::string formatLog(LogLevel level, const std::string &message) {
    std::ostringstream formattedMessage;
    formattedMessage << "[" << getCurrentTime() << "] " << "["
                     << getLevelString(level) << "] " << message;
    return formattedMessage.str();
  };
  static std::string getLevelString(LogLevel level) {
    switch (level) {
    case LogLevel::ERROR:
      return "\033[31mERROR\033[0m"; // Red color
    case LogLevel::WARNING:
      return "\033[33mWARNING\033[0m"; // Yellow color
    case LogLevel::INFO:
      return "\033[34mINFO\033[0m"; // Blue color
    case LogLevel::DEBUG:
      return "\033[37mDEBUG\033[0m"; // White/Gray color
    default:
      return "UNKNOWN";
    }
  };
  static std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return oss.str();
  };

private:
  static std::mutex m_mutex;
};

#endif