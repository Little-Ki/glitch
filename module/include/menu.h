#pragma once

namespace ct::menu {
	void render();

	bool install();

	void uninstall();

	void watch(void* hwnd);

	void unwatch();
}