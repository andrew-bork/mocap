/**
 * @file logger.cpp
 * @author andrew
 * @brief logger.cpp test script
 * @version 0.1
 * @date 2022-07-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <logger.h>
// #include <iostream>
// #include <cmath>
#include <fmt/core.h>
#include <cstdio>
#include <string>

void foobar(const char* msg, const char* str = __builtin_FUNCTION()){
    std::printf("%s (called by %s)\n", msg, str);
}

#define test(msg, ...) fmt::format(msg, ##__VA_ARGS__)

int main(){
    // std::cout << "bruh\n";
    // printf("bfuabfdsaj\n");
    // printf("%s\n", _SOURCE_LOCATION_);
    foobar("this is a message");
    std::string s = fmt::format("cock {}", "and balls");
    printf("%s\n", s.c_str());
    s = test("wow {} {}", 2.41221, "IT WORKS!");
    printf("%s\n", s.c_str());

    logger::debug("chugging along");
    logger::info("theres something you need to know!");
    logger::warn("expected a 0, but got a 1");
    logger::crit("FIRE FIRE FIRE");
    logger::err("something went wrong");

    logger::debug("testing formatting {}.", "test worked!");
    logger::info("testing formatting {}.", "test worked!");
    logger::warn("testing formatting {}.", "test worked!");
    logger::crit("testing formatting {}.", "test worked!");
    logger::err("testing formatting {}.", "test worked!");
}