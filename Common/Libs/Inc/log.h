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

/**
 * @brief Logging severity levels in ascending order of importance
 */
typedef enum {
    DEBUG_LVL = 0,  ///< Debug messages for development and troubleshooting
    INFO_LVL = 1,   ///< Informational messages about normal operation
    WARN_LVL  = 2,  ///< Warning messages for potentially problematic situations
    FAULT_LVL = 3   ///< Critical fault/error messages
} LogLevel;

/**
 * @brief Global log level threshold - messages below this level are filtered out
 */
extern volatile LogLevel g_log_level;

/**
 * @brief Configure the logging system with UART parameters
 * @param level Minimum log level to output (messages below this are suppressed)
 * @param tx UART transmit pin for log output
 * @param rx UART receive pin (typically unused for logging)
 * @param baudrate UART baud rate (e.g., 115200)
 * 
 * Must be called before using any logging functions. Initializes the underlying
 * UART peripheral for serial output.
 */
void log_configure(LogLevel level, Pin tx, Pin rx, uint32_t baudrate);

/**
 * @brief Core logging function with level, file, and line information
 * @param level Severity level of this message
 * @param file Source file name (typically __FILE__)
 * @param line Line number (typically __LINE__)
 * @param fmt Printf-style format string
 * @param ... Variable arguments matching format specifiers
 * 
 * Not typically called directly - use the log_debug/info/warn/fault macros instead.
 * Outputs formatted messages with level prefix and source location if level >= g_log_level.
 */
void log(LogLevel level, const char* file, int line, const char* fmt, ...) LOG_PRINTF_ATTR(4,5);

/**
 * @def log_debug
 * @brief Log a debug message with automatic file/line information
 * @param ... Printf-style format string and arguments
 */
#define log_debug(...) log(DEBUG_LVL, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @def log_info
 * @brief Log an informational message with automatic file/line information
 * @param ... Printf-style format string and arguments
 */
#define log_info(...)  log(INFO_LVL,  __FILE__, __LINE__, __VA_ARGS__)

/**
 * @def log_warn
 * @brief Log a warning message with automatic file/line information
 * @param ... Printf-style format string and arguments
 */
#define log_warn(...)  log(WARN_LVL,  __FILE__, __LINE__, __VA_ARGS__)

/**
 * @def log_fault
 * @brief Log a fault/error message with automatic file/line information
 * @param ... Printf-style format string and arguments
 */
#define log_fault(...) log(FAULT_LVL, __FILE__, __LINE__, __VA_ARGS__)

#endif
