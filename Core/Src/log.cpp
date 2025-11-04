#include "log.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

volatile LogLevel g_log_level = info;

static UART* s_uart = nullptr;

#ifndef LOG_BUF_SIZE
#define LOG_BUF_SIZE 256
#endif

// Optional level tag.
static inline const char* lvl_tag(LogLevel l) {
    switch (l) {
        case debug: return "DEBUG";
        case info:  return "INPUT";
        case warn:  return "WARNING";
        default:    return "FAULT";
    }
}

void log_configure(LogLevel level, Pin tx, Pin rx, uint32_t baudrate) {
    g_log_level = level;
    static UART instance(tx, rx, baudrate);
    s_uart = &instance;
}

void log(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (!s_uart || level < g_log_level) return;

    char buf[LOG_BUF_SIZE];

    int prefix_size = std::snprintf(buf, LOG_BUF_SIZE, "ADD CLOCK HERE [%s] %s:%d: ", lvl_tag(level), file, line);

    if (prefix_size < 0) {
        char err_prefix[] = "Log Prefix Error\n";
        s_uart->write(reinterpret_cast<uint8_t*>(err_prefix), strlen(err_prefix));
        return;
    }

    if (prefix_size >= LOG_BUF_SIZE) {
        char warn_prefix[] = "Log Prefix Overflow in next log\n";
        s_uart->write(reinterpret_cast<uint8_t*>(warn_prefix), sizeof(warn_prefix) - 1);

        buf[LOG_BUF_SIZE - 2] = '\n';
        
        s_uart->write(reinterpret_cast<uint8_t*>(buf), LOG_BUF_SIZE - 1);
        return;
    }

    va_list var_args;
    va_start(var_args, fmt);
    int message_size = std::vsnprintf(buf + prefix_size, LOG_BUF_SIZE - prefix_size, fmt, var_args);
    va_end(var_args);

    if (message_size < 0) {
        char err_prefix[] = "Log Message Error\n";
        s_uart->write(reinterpret_cast<uint8_t*>(err_prefix), sizeof(err_prefix));
        return;
    }

    int total = prefix_size + message_size;

    if (total >= LOG_BUF_SIZE - 1) {
        char warn_prefix[] = "Log Message Overflow in next log\n";
        s_uart->write(reinterpret_cast<uint8_t*>(warn_prefix), sizeof(warn_prefix));

        // overwrite chars with null terminator
        buf[LOG_BUF_SIZE - 2] = '\n';

        total = LOG_BUF_SIZE - 1;
    }
    else {
        buf[total++] = '\n';
        buf[total] = '\0';
    }
        
    s_uart->write(reinterpret_cast<uint8_t*>(buf), strlen(buf));
    return;
}
