#pragma once

#include "Logger.hpp"

#ifdef NDEBUG
#define LOG_ERROR(message)
#define LOG_WARNING(message)
#define LOG_INFO(message)
#define LOG_DEBUG(message)
#else
#define LOG_ERROR(message) Logger::error(message)
#define LOG_WARNING(message) Logger::warning(message)
#define LOG_INFO(message) Logger::info(message)
#define LOG_DEBUG(message) Logger::debug(message)
#endif