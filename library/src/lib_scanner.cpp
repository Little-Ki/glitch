#include "lib_scanner.h"
#include "lib_memory.h"

#include <algorithm>
#include <cstring>
#include <cctype>
#include <cassert>

using namespace cl::memory;

namespace cl::scanner {

	Pattern::Pattern(const std::string& pattern) {
		uint8_t value = 0;

		auto xdigit = [](char c) {
			c = std::tolower(c);
			return std::isxdigit(c) ?
				((c >= '0' && c <= '9') ? (c - '0') : (c - 'a' + 10)) : 0;
			};

		for (const auto& c : pattern) {
			if (c == '?') {
				_elements.push_back({});
			}
			else if (std::isxdigit(c)) {
				value = (value << 4) | xdigit(c);
			}
			else {
				_elements.push_back({ value });
				value = 0U;
			}
		}

		if (value) {
			_elements.push_back({ value });
		}
	}

	uintptr_t search(uint8_t* begin, uint8_t* end, const Pattern& pattern, uint32_t offset)
	{
		assert(begin <= end && !pattern.empty());

		auto it = std::search(
			begin,
			end,
			pattern.begin(),
			pattern.end(),
			[](const uint8_t a, const Pattern::Element& b) {
				return b.is_wild || b.byte == a;
			});

		if (it == end) {
			return 0;
		}

		return reinterpret_cast<uintptr_t>(it) + offset;
	}

	uintptr_t search_s(uint8_t* begin, uint8_t* end, const Pattern& pattern, uint32_t offset)
	{
		assert(begin <= end && !pattern.empty());

		auto it = std::search(
			Address<uint8_t>(begin),
			Address<uint8_t>(end),
			pattern.begin(),
			pattern.end(),
			[](const uint8_t* a, const Pattern::Element& b) {
				return isValid(a) && (b.is_wild || b.byte == *a);
			});

		if (it == end) {
			return 0;
		}

		return reinterpret_cast<uintptr_t>(*it) + offset;
	}
}