#ifndef LOG_H
#define LOG_H

#include <stddef.h>

enum LogCodes {
    L_DEBUG,
    L_INFO,
    L_WARN,
    L_ERROR,
    L_FATAL,
    L_NO_LOG
};

int setLogMask(int mask);
int setLogFile(const char *file);

int logFunction(const char *file,
                 size_t line,
                 enum LogCodes code,
                 const char *fmt, ...);

#define LOG(CODE, ...) logFunction(__FILE__, __LINE__, CODE, ##__VA_ARGS__)

#endif
