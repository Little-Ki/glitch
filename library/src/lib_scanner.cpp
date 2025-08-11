#include "lib_scanner.h"
#include "lib_memory.h"

#include <algorithm>
#include <sstream>
#include <cassert>
#include <cctype>
#include <stdexcept>
#include <string>

namespace cl::scanner {

    Pattern::Pattern(const std::string &pattern) {
        std::istringstream iss(pattern);
        std::string token;

        while (iss >> token) {
            if (token.size() != 2 ||
                !std::isxdigit(token[0]) ||
                !std::isxdigit(token[1])) {

                if (token == "??" ||
                    (token.size() == 2 && token[0] == '?') ||
                    (token.size() == 2 && token[1] == '?')) {
                    _elements.push_back(Element{0, true});
                    continue;
                }

                throw std::runtime_error("Invalid pattern token: " + token);
            }

            uint8_t byte = static_cast<uint8_t>(std::stoul(token, nullptr, 16));
            _elements.push_back(Element{byte, false});
        }
    }

    void *search(void *begin, void *end, const Pattern &pattern, uint32_t offset) {
        assert(begin <= end && !pattern.empty());

        auto it = std::search(
            reinterpret_cast<uint8_t *>(begin),
            reinterpret_cast<uint8_t *>(end),
            pattern.begin(), pattern.end(),
            [](const uint8_t a, const Pattern::Element &b) {
                return b.is_wild || b.byte == a;
            });

        if (it == end)
            return nullptr;

        return it + offset;
    }

}