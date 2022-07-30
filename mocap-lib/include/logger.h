#ifndef LOGGER_H
#define LOGGER_H

#include <string_view>
// #include <format>
#include <fmt/core.h>
// #ifndef __unix__
#define _LOGGER_USE_MACRO_
// #endif

#ifndef _LOGGER_USE_MACRO_
#include <source_location>
#endif
namespace logger {


    enum level {
        DEBUG=0, INFO=1, WARN=2, CRITICAL=3, ERROR=4, CONFIG=5
    };

    void set_level(level lvl);
    void set_log_file(const char * filename);


    #ifndef _LOGGER_USE_MACRO_
    
    void log(level lvl, const char * msg, const std::source_location location = std::source_location::current());

    void debug(const char * msg, const std::source_location location = std::source_location::current());
    void info(const char * msg, const std::source_location location = std::source_location::current());
    void crit(const char * msg, const std::source_location location = std::source_location::current());
    void warn(const char * msg, const std::source_location location = std::source_location::current());
    void err(const char * msg, const std::source_location location = std::source_location::current());

    #else
    // USE MACROS

    std::string _format(const char * formatted, ...);
    void _log(level lvl, std::string_view msg, const char* funct_name, const char * file_name, const int line);

    #define debug(msg, ...) _log(logger::DEBUG, fmt::format(msg, ##__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__)
    #define info(msg, ...) _log(logger::INFO, fmt::format(msg, ##__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__)
    #define crit(msg, ...) _log(logger::CRITICAL, fmt::format(msg, ##__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__)
    #define warn(msg, ...) _log(logger::WARN, fmt::format(msg, ##__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__)
    #define err(msg, ...) _log(logger::ERROR, fmt::format(msg, ##__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__)
    #define lconfig(msg, ...) _log(logger::CONFIG, fmt::format(msg, ##__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__)
    #endif
};

#endif