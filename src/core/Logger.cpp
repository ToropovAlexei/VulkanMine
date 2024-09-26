#include "Logger.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>

std::mutex Logger::m_mutex;

void Logger::error(const std::string &message) {
  log(LogLevel::ERROR, message);
}

void Logger::warning(const std::string &message) {
  log(LogLevel::WARNING, message);
}

void Logger::info(const std::string &message) { log(LogLevel::INFO, message); }

void Logger::debug(const std::string &message) {
  log(LogLevel::DEBUG, message);
}

void Logger::log(LogLevel level, const std::string &message) {
  std::string output = formatLog(level, message);

  std::lock_guard<std::mutex> lock(m_mutex);
  std::cout << output << std::endl;
}

std::string Logger::formatLog(LogLevel level, const std::string &message) {
  std::ostringstream formattedMessage;
  formattedMessage << "[" << getCurrentTime() << "] " << "["
                   << getLevelString(level) << "] " << message;
  return formattedMessage.str();
}

std::string Logger::getLevelString(LogLevel level) {
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
}

std::string Logger::getCurrentTime() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  return oss.str();
}