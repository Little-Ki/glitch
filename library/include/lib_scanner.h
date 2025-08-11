#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace cl::scanner {

    class Pattern {
    public:
        struct Element {
            const uint8_t byte{0};
            const bool is_wild{true};
        };

    private:
        std::vector<Element> _elements;

    public:
        Pattern(const std::string &pattern);

        bool empty() const { return _elements.empty(); }

        auto begin() const { return _elements.cbegin(); }
        auto end() const { return _elements.cend(); }
    };

    void *search(void *begin, void *end, const Pattern &pattern, uint32_t offset);

    template <typename T = uintptr_t>
    T search(void *begin, void *end, const std::string &pattern, uint32_t offset) {
        return reinterpret_cast<T>(search(begin, end, pattern, offset));
    }

}