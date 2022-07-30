// clang++ ./src/logger.cpp ./tools/logger.cpp -Iinclude -o ./bin/logger
#include <cstdio>
// #include <time.h>
#include <logger.h>
#include <ctime>
// #include <time.h>
// #include <format>
#include <string>
#include <fmt/core.h>
#include <mutex>
// #include <core.h>
// #include <mutex.h>

void logger::set_log_file(const char * filename){

}

static logger::level allowed = logger::INFO;
// static char buf20A[20];
// static char buf20B[20];

static const char * titles [6] = {"debug", "info", "WARNING", "CRITICAL", "ERROR", "config"};

#define RESET "\x1b[0m"
// static const char* colors [5] = {"\x1b[37;49m", "\x1b[36;49m", "\x1b[33;49m", "\x1b[35;49m", "\x1b[31;49m"};

static const char* colors [6] = {"\x1b[37m", "\x1b[36m", "\x1b[33m", "\x1b[31m", "\x1b[31m", "\x1b[32m"};

inline void localtime(tm &bruh){
    time_t now = time(0);
    char buf100[100];
    #if defined(__unix__)
        localtime_r(&now, &bruh);
    #elif defined(_MSC_VER)
        localtime_s(&bruh, &now);
    #endif
}

#ifndef _LOGGER_USE_MACRO_
void logger::debug(const char * msg, const std::source_location location) {
    log(level::DEBUG, msg, location);
}
void logger::info(const char * msg, const std::source_location location) {
    log(level::INFO, msg, location);
}
void logger::crit(const char * msg, const std::source_location location) {
    log(level::CRITICAL, msg, location);
}
void logger::warn(const char * msg, const std::source_location location) {
    log(level::WARN, msg, location);
}
void logger::err(const char * msg, const std::source_location location) {
    log(level::ERROR, msg, location);
}
void logger::log(level lvl, const char * msg, const std::source_location location){
    if(lvl >= allowed){
        tm tstruct;
        char buf100[100];
        localtime(tstruct);
        strftime(buf100, 100, "%Y-%m-%d %X", &tstruct);
        printf("%s [%s] - %s - %s:%d:%d \"%s\" - %s %s\n",colors[lvl], buf100, titles[lvl], location.file_name(), location.line(), location.column(), location.function_name(), msg, RESET);
    }
}
#endif

// std::string logger::_format(const char * fmt, ...){
//     va_list args;
//     va_start(args, formatted);
//     std::string s = std::vformat(fmt, args);
//     va_end(args);
//     return s;
// }


static std::mutex logger_mutex;
// static std::unique_lock<std::mutex> logger_lock(logger_mutex, std::defer_lock);

void logger::_log(level lvl, std::string_view msg, const char* funct_name, const char * file_name, const int line){
    if(lvl >= allowed){
        std::lock_guard<std::mutex> lock(logger_mutex);
        // logger_lock.lock();
        tm tstruct;
        char buf100[100];
        localtime(tstruct);
        strftime(buf100, 100, "%Y-%m-%d %X", &tstruct);
        fmt::print("{} [{}] - {} - \"{}:{}\" function \"{}\" - {} {}\n",colors[lvl], buf100, titles[lvl], file_name, line, funct_name, msg, RESET);
        // logger_lock.unlock();
    }
}

void logger::set_level(logger::level lvl){
    allowed = lvl;
}