#include "../stdafx.h"
#include "mainmenu.h"
#include "../callbacks/callbacks.h"

void MAINMENU::ShenMenu() {
	if (GetAsyncKeyState(menuKey) & 1) {
		isShowMenu = !isShowMenu;
	}

	if (isShowMenu) {

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;

		//ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(800, 800));
		ImGui::Begin(XS("SH3N"), &isShowMenu, window_flags);

		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar(XS("Tabs"), tab_bar_flags))
		{

			for (auto const& [key, val] : CALLBACKS_INSTANCE->TabItemsContent)
			{
				if (ImGui::BeginTabItem(key.c_str()))
				{
					val();
					ImGui::EndTabItem();
				}
			}

			if (ImGui::BeginTabItem(XS("CREDITS")))
			{
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), XS("Unknowncheats"));
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), XS("sh3n"));

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();

		//ImGui::ShowDemoWindow(&isShowMenu);
	}
}

bool MAINMENU::getIsShowMenu() {
	return isShowMenu;
}