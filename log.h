#ifndef LOG_H
#define LOG_H

#include <stddef.h>

enum logCodes {
    LDebug,
    LInfo,
    LWarn,
    LError,
    LFatal,
    LNoLog
};

int setLogMask(int mask);
int setLogFile(const char *file);

int logFunction(const char *file,
                size_t line,
                enum logCodes code,
                const char *fmt, ...);

#define LOG(CODE, ...) logFunction(__FILE__, __LINE__, CODE, ##__VA_ARGS__)

#endif
