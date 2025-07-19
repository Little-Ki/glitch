#pragma once
#include <stdint.h>

namespace cl::dasm {

	uint32_t asmlen(const uint8_t *ptr, bool x64);
	
}