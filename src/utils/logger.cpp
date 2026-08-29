#include "utils/logger.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>

#ifndef PC_SIMULATOR
#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#ifndef u_short
typedef unsigned short u_short;
#endif
#ifndef u_int
typedef unsigned int u_int;
#endif
#include <orbis/libkernel.h>
#endif

void Logger::init() {
#ifdef PC_SIMULATOR
    printf("[Logger] Initialized (PC Simulator)\n");
#else
    // On PS4, log to /data/ps4MediaPlay.log and also sceKernelDebugOutText
    FILE* f = fopen("/data/ps4MediaPlay.log", "w");
    if (f) {
        fprintf(f, "=== PS4 Media Play Log Started ===\n");
        fclose(f);
    }
#endif
}

static void logInternal(const char* level, const char* fmt, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    // Timestamp
    time_t t = time(nullptr);
    struct tm* tm = localtime(&t);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm);

#ifdef PC_SIMULATOR
    printf("[%s] [%s] %s\n", timeStr, level, buffer);
#else
    // PS4 debug output
    char out[1152];
    snprintf(out, sizeof(out), "[%s] [%s] %s\n", timeStr, level, buffer);
    // sceKernelDebugOutText(0, out);
    printf("%s", out);

    FILE* f = fopen("/data/ps4MediaPlay.log", "a");
    if (f) { fprintf(f, "%s", out); fclose(f); }
#endif
}

void Logger::info(const char* fmt, ...) { va_list a; va_start(a, fmt); logInternal("INFO", fmt, a); va_end(a); }
void Logger::warn(const char* fmt, ...) { va_list a; va_start(a, fmt); logInternal("WARN", fmt, a); va_end(a); }
void Logger::error(const char* fmt, ...) { va_list a; va_start(a, fmt); logInternal("ERROR", fmt, a); va_end(a); }
void Logger::debug(const char* fmt, ...) { va_list a; va_start(a, fmt); logInternal("DEBUG", fmt, a); va_end(a); }
