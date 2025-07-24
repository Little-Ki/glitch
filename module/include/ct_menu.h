#pragma once

#include <tuple>

namespace ct::menu {
	void render();

	bool install();

	void uninstall();

	void watch(void* hwnd);

	void unwatch();

	std::pair<int, int> size();
}