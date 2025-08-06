#include "ct_feature.h"

#include "imgui.h"

#include <cstdint>

namespace ct::feature {

	static void drawLine(const ImVec2& from, const ImVec2& to, uint32_t color, const float width = 1.0f) {
		auto drawList = ImGui::GetBackgroundDrawList();
		drawList->AddLine(from, to, color, width);
	}

	void install() {
		// TODO: setup stuffs like offsets
	}

	void execute() {
		// TODO: run cheat features

		drawLine({ 0.0f, 0.0f }, { 100.0f, 50.f }, 0xFF0000FF, 3.0f);
	};
}