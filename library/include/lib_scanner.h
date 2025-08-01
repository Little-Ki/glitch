#pragma once
#include <stdint.h>
#include <string>
#include <vector>

namespace cl::scanner {

    class Pattern {
    private:
        std::vector<Element> _elements;

    public:
        struct Element {
            const uint8_t byte{0};
            const bool is_wild{true};
        };

    public:
        Pattern(const std::string &pattern);

        bool empty() const { return _elements.empty(); }

        auto begin() const { return _elements.cbegin(); }
        auto end() const { return _elements.cend(); }
    };

    uintptr_t search(uint8_t *begin, uint8_t *end, const Pattern &pattern, uint32_t offset, bool safe = false);

    template <typename T = uintptr_t>
    T search(uint8_t *begin, uint8_t *end, const std::string &pattern, uint32_t offset, bool safe = false) {
        return reinterpret_cast<T>(search(begin, end, pattern, offset, safe));
    }

}