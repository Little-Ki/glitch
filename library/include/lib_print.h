#pragma once

#include <Windows.h>

#include <cstdint>
#include <iostream>
#include <type_traits>

namespace cl::print {

    constexpr uint16_t CONSOLE_BLACK = 0;
    constexpr uint16_t CONSOLE_DARKBLUE = 1;
    constexpr uint16_t CONSOLE_DARKGREEN = 2;
    constexpr uint16_t CONSOLE_DARKCYAN = 3;
    constexpr uint16_t CONSOLE_DARKRED = 4;
    constexpr uint16_t CONSOLE_DARKPINK = 5;
    constexpr uint16_t CONSOLE_DARKYELLOW = 6;
    constexpr uint16_t CONSOLE_LIGHTGREY = 7;
    constexpr uint16_t CONSOLE_DARKGREY = 8;
    constexpr uint16_t CONSOLE_BLUE = 9;
    constexpr uint16_t CONSOLE_GREEN = 10;
    constexpr uint16_t CONSOLE_CYAN = 11;
    constexpr uint16_t CONSOLE_RED = 12;
    constexpr uint16_t CONSOLE_PINK = 13;
    constexpr uint16_t CONSOLE_YELLOW = 14;
    constexpr uint16_t CONSOLE_WHITE = 15;

    template <typename S, typename = typename std::enable_if_t<std::is_integral_v<S>>>
    void print(const S* format) {
        std::cout << format;
    }
    
    template <typename S, typename = typename std::enable_if_t<std::is_integral_v<S>>, typename T,  typename... Args>
    void print(const S *format, T value, Args... args) {
        for (; *format != '\0'; format++) {
            if (*format == '%') {
                std::cout << value;
                print(format + 1, args...); 
                return;
            }
            std::cout << *format;
        }
    }

    template <uint16_t C = 0, typename S, typename = typename std::enable_if_t<std::is_integral_v<S>>, typename... Args>
    void print(const S* format, Args... args) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), C);
        print(format, args...);
    }
}