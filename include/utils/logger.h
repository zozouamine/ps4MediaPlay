#pragma once
#include <string>

class Logger {
public:
    static void init();
    static void info(const char* fmt, ...);
    static void warn(const char* fmt, ...);
    static void error(const char* fmt, ...);
    static void debug(const char* fmt, ...);
};
