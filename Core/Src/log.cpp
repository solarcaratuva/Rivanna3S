#include "log.h"
#include "Clock.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

volatile LogLevel global_log_level = info;

static UART* log_uart = nullptr;

#define LOG_BUF_SIZE 256

static inline const char* lvl_tag(LogLevel l) { // static inline so only in this file
    switch (l) {
        case debug: return "DEBUG";
        case info:  return "INFO";
        case warn:  return "WARN";
        default:    return "FAULT";
    }
}

void log_configure(LogLevel level, Pin tx, Pin rx, uint32_t baudrate) {
    global_log_level = level;
    static UART instance(tx, rx, baudrate); // static global var so that it is only usable in this file
    log_uart = &instance;
}

void log(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (!log_uart || level < global_log_level) 
        return;

    char buf[LOG_BUF_SIZE];

    uint32_t time = Clock::get_current_time();

    unsigned h  = (time / 3600000) % 24; // wraps every 24 hours
    unsigned m  = (time / 60000)  % 60;
    unsigned s =  (time / 1000) % 60;

    char prefix[LOG_BUF_SIZE];
    int prefix_size = std::snprintf(prefix, LOG_BUF_SIZE, "%02u:%02u:%02u %s %s:%d: ", h, m, s, lvl_tag(level), file, line);
    int buf_prefix_size = std::snprintf(buf, LOG_BUF_SIZE, "%s", prefix);

    if (prefix_size < 0 || buf_prefix_size < 0) { // if -1, error occured
        char err_prefix[LOG_BUF_SIZE];
        std::snprintf(err_prefix, LOG_BUF_SIZE, "%sLog Prefix Error\n", prefix);

        log_uart->write(reinterpret_cast<uint8_t*>(err_prefix), strlen(err_prefix));
        return;
    }

    // note that we dont check if prefix buffer overflows, since it only could happen if file name or file path is super large or very large line number, etc.

    va_list var_args;
    va_start(var_args, fmt);
    int message_size = std::vsnprintf(buf + prefix_size, LOG_BUF_SIZE - prefix_size, fmt, var_args);
    va_end(var_args);

    if (message_size < 0) {
        char err_prefix[LOG_BUF_SIZE];
        std::snprintf(err_prefix, LOG_BUF_SIZE, "%sLog Error\n", prefix);

        log_uart->write(reinterpret_cast<uint8_t*>(err_prefix), strlen(err_prefix));
        return;
    }

    int total = prefix_size + message_size;

    if (total > LOG_BUF_SIZE - 2) {
        char warn_prefix[LOG_BUF_SIZE];
        std::snprintf(warn_prefix, LOG_BUF_SIZE, "%sLog Overflow\n", prefix);

        log_uart->write(reinterpret_cast<uint8_t*>(warn_prefix), strlen(warn_prefix));

        // If overflow then, overwrite last char (before the trailing '\0') with a '\n'
        buf[strlen(buf) - 1] = '\n';
    }
    else{ 
        // If no overflow, replace trailing '\0' with '\n\0' if there is space
        buf[strlen(buf) + 1] = '\0';
        buf[strlen(buf)] = '\n';
    }

    log_uart->write(reinterpret_cast<uint8_t*>(buf), strlen(buf));
    return;
}
