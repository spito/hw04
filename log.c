#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

static const char *logFile = NULL;
static FILE *logStream = NULL;
static int logMask = L_INFO;

void logClose() {
    fclose(logStream);
}

int setLogMask(int mask) {
    if (mask < L_DEBUG || mask > L_NO_LOG)
        return -1;

    logMask = mask;
    return 0;
}

int setLogFile(const char *file) {
    if (!logFile)
        atexit(logClose);
    else
        fclose(logStream);

    logFile = file;
    logStream = fopen(logFile, "a");

    if (!logStream) {
        logStream = stderr;
        return -1;
    }
    return 0;
}

int logFunction(const char *file,
                size_t line,
                enum LogCodes code,
                const char *fmt, ...) {
    if (code < logMask || code == L_NO_LOG)
        return 1;
    if (!logStream)
        logStream = stderr;

    struct timeval now;
    gettimeofday(&now, NULL);
    char bufferTime[100];
    size_t rc = strftime(bufferTime, sizeof(bufferTime), "%F %T", localtime(&now.tv_sec));
    snprintf(bufferTime + rc, sizeof(bufferTime) - rc, ".%06ld", now.tv_usec);
 
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);
    char bufferLog[1 + vsnprintf(NULL, 0, fmt, args1)];
    va_end(args1);
    vsnprintf(bufferLog, sizeof(bufferLog), fmt, args2);
    va_end(args2);
 
    const char *kind;
    switch (code) {
    case L_DEBUG:   kind = "D"; break;
    case L_INFO:    kind = "I"; break;
    case L_WARN:    kind = "W"; break;
    case L_ERROR:   kind = "E"; break;
    case L_FATAL:   kind = "F"; break;
    default:        kind = "?"; break;
    }

    return fprintf(logStream,
                   "%s [%s]: %s {%s:%lu}\n",
                   bufferTime,
                   kind,
                   bufferLog,
                   file,
                   line);
}