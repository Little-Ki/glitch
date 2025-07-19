#pragma once

#include <string>
#include <iostream>

namespace cl::console {
    enum class ConsoleColor {
        BLACK,
        DARKBLUE,
        DARKGREEN,
        DARKCYAN,
        DARKRED,
        DARKPINK,
        DARKYELLOW,
        LIGHTGREY,
        DARKGREY,
        BLUE,
        GREEN,
        CYAN,
        RED,
        PINK,
        YELLOW,
        WHITE
    };

    bool allocate(const std::string &title);

    void detach();

    void setColor(ConsoleColor color);

    void print(const char *format);

    template <typename T, typename... Args>
    void print(const char *format, T value, Args... args) {
        for (; *format != '\0'; format++) {
            if (*format == '%') {
                std::cout << value;
                print(format + 1, args...); 
                return;
            }
            std::cout << *format;
        }
    }
}