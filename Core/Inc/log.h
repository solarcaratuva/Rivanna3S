#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdarg.h>

#include "pinmap.h"
#include "UART.h"

// GCC/Clang: enable printf-style format checking.
#if defined(__GNUC__) || defined(__clang__)
  #define LOG_PRINTF_ATTR(fmt_idx, va_idx) __attribute__((format(printf, fmt_idx, va_idx)))
#else
  #define LOG_PRINTF_ATTR(fmt_idx, va_idx)
#endif

typedef enum {
    DEBUG_LVL = 0,
    INFO_LVL = 1,
    WARN_LVL  = 2,
    FAULT_LVL = 3
} LogLevel;

extern volatile LogLevel g_log_level;

void log_configure(LogLevel level, Pin tx, Pin rx, uint32_t baudrate);

void log(LogLevel level, const char* file, int line, const char* fmt, ...) LOG_PRINTF_ATTR(4,5);

#define log_debug(...) log(DEBUG_LVL, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log(INFO_LVL,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log(WARN_LVL,  __FILE__, __LINE__, __VA_ARGS__)
#define log_fault(...) log(FAULT_LVL, __FILE__, __LINE__, __VA_ARGS__)

#endif
