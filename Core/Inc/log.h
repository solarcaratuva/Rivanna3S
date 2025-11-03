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
    debug = 0,
    info  = 1,
    warn  = 2,
    fault = 3
} LogLevel;

extern volatile LogLevel g_log_level;

void log_configure(LogLevel level, Pin tx, Pin rx, uint32_t baudrate);

void log(LogLevel level, const char* file, int line, const char* fmt, ...) LOG_PRINTF_ATTR(4,5);

#define log_debug(...) do { if (g_log_level <= debug) log(debug, __FILE__, __LINE__, __VA_ARGS__); } while (0)
#define log_info(...)  do { if (g_log_level <= info ) log(info,  __FILE__, __LINE__, __VA_ARGS__); } while (0)
#define log_warn(...)  do { if (g_log_level <= warn ) log(warn,  __FILE__, __LINE__, __VA_ARGS__); } while (0)
#define log_fault(...) do { if (g_log_level <= fault) log(fault, __FILE__, __LINE__, __VA_ARGS__); } while (0)

#endif
