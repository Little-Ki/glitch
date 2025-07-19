#include "menu.h"

#include <Windows.h>

#include "imgui.h"

namespace ct::menu {
	void render()
	{
		ImGui::SetNextWindowPos({ 100, 100 });
		ImGui::Begin("Hello");

		ImGui::Text("Hello");
		ImGui::Button("World!");

		ImGui::End();
	}
}