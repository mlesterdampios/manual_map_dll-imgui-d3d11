#pragma once
#include <inttypes.h>
#include "../../sdk/cs2/game.hpp"
#include "../../imgui/Renderer.h"

class MAINH4X {
private:
	static inline bool isDoneInit = false;
	static inline bool g_en_glow = false;
	static inline bool g_en_bone = false;
	static inline bool g_en_line = false;
	static inline bool g_en_box = false;
public:

	static void inline ToggleButton(const char* str_id, bool* v)
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = ImGui::GetFrameHeight();
		float width = height * 1.55f;
		float radius = height * 0.50f;

		ImGui::InvisibleButton(str_id, ImVec2(width, height));
		if (ImGui::IsItemClicked()) *v = !*v;
		ImGuiContext& gg = *GImGui;
		float ANIM_SPEED = 0.085f;
		if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
			float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
		if (ImGui::IsItemHovered())
			draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
		else
			draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}

	static void inline HelpMarker(const char* desc)
	{
		ImGui::TextDisabled(XS("(?)"));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	static bool wrld_to_screen(Vec3& pos, Vec3& out, view_matrix_t matrix) {
		out.x = matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3];
		out.y = matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3];

		float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

		if (w < 0.01f)
			return false;

		float inv_w = 1.f / w;
		out.x *= inv_w;
		out.y *= inv_w;

		float x = RENDERER_INSTANCE.win_width * .5f;
		float y = RENDERER_INSTANCE.win_height * .5f;

		x += 0.5f * out.x * RENDERER_INSTANCE.win_width + 0.5f;
		y -= 0.5f * out.y * RENDERER_INSTANCE.win_height + 0.5f;

		out.x = x;
		out.y = y;

		return true;
	}

	static void inline OnImguiInit() {
		gGame.InitAddress();
		isDoneInit = true;
	}

	static void inline Draw() {
		ToggleButton(XS("g_en_glow"), &g_en_glow);
		ImGui::SameLine();
		ImGui::Text(XS("[F1] Glow"));
		ImGui::SameLine();
		HelpMarker(XS("See player glow through walls."));

		ToggleButton(XS("g_en_bone"), &g_en_bone);
		ImGui::SameLine();
		ImGui::Text(XS("[F2] Bones"));
		ImGui::SameLine();
		HelpMarker(XS("See player bones through walls."));

		ToggleButton(XS("g_en_line"), &g_en_line);
		ImGui::SameLine();
		ImGui::Text(XS("[F3] Lines"));
		ImGui::SameLine();
		HelpMarker(XS("See player lines."));

		ToggleButton(XS("g_en_box"), &g_en_box);
		ImGui::SameLine();
		ImGui::Text(XS("[F4] Boxes"));
		ImGui::SameLine();
		HelpMarker(XS("See player box."));
	}

	static void inline OnPresent() {

		if (GetAsyncKeyState(VK_F1) & 1) {
			g_en_glow = !g_en_glow;
		}

		if (GetAsyncKeyState(VK_F2) & 1) {
			g_en_bone = !g_en_bone;
		}

		if (GetAsyncKeyState(VK_F3) & 1) {
			g_en_line = !g_en_line;
		}

		if (GetAsyncKeyState(VK_F4) & 1) {
			g_en_box = !g_en_box;
		}

		if (!isDoneInit)
			return;

		//g_glow
		if(g_en_glow || g_en_bone || g_en_line || g_en_box) {
			uintptr_t LocalControllerAddress = 0;
			LocalControllerAddress = *(uintptr_t*)(gGame.GetLocalControllerAddress());

			if (LocalControllerAddress != 0) {
				uintptr_t LocalPawnAddress = 0;
				LocalPawnAddress = *(uintptr_t*)(gGame.GetLocalPawnAddress());

				if (LocalPawnAddress != 0) {
					int localTeam = *(int*)(LocalPawnAddress + Offset::Player.m_iTeamNum);
					view_matrix_t view_matrix = *(view_matrix_t*)(gGame.GetViewMatrix());

					uintptr_t Entity = *(uintptr_t*)(gGame.GetEntityListAddress());
					if (Entity != 0) {

						CGlobalVarsBase* _CGlobalVarsBase = gGame.GetCGlobalVarsBase();

						for (int i = 0; i < 64; i++)
						{
							uintptr_t listEntity = *(uintptr_t*)(Entity + ((8 * (i & 0x7FFF) >> 9) + 16));
							if (listEntity == 0)
								continue;

							uintptr_t entityController = *(uintptr_t*)(listEntity + (120) * (i & 0x1FF));
							if (entityController == 0)
								continue;

							uintptr_t entityControllerPawn = *(uintptr_t*)(entityController + Offset::Entity.m_hPlayerPawn);
							if (entityControllerPawn == 0)
								continue;

							listEntity = *(uintptr_t*)(Entity + (0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 16));
							if (listEntity == 0)
								continue;

							uintptr_t entityPawn = *(uintptr_t*)(listEntity + (120) * (entityControllerPawn & 0x1FF));
							if (entityPawn == 0)
								continue;

							int health = *(int*)(entityPawn + Offset::Player.m_iHealth);
							if (health <= 0)
								continue;

							int playerTeam = *(int*)(entityPawn + Offset::Player.m_iTeamNum);
							if (playerTeam == localTeam)
								continue;

							if (g_en_glow) {
								if (_CGlobalVarsBase != 0) {
									float* p_reg = (float*)(entityPawn + Offset::Entity.m_flDetectedByEnemySensorTime);
									*p_reg = _CGlobalVarsBase->current_time + 1.f;
								}
							}

							if (g_en_bone || g_en_line || g_en_box)
							{
								uintptr_t gamescene = *(uintptr_t*)(entityPawn + Offset::Player.m_pGameSceneNode);
								if (gamescene != 0) {
									uintptr_t bonearray = *(uintptr_t*)(gamescene + Offset::Player.BoneArray);

									if (bonearray != 0) {
										Vec3 head = *(Vec3*)(bonearray + (6 * 32));
										Vec3 cou = *(Vec3*)(bonearray + (5 * 32));
										Vec3 shoulderR = *(Vec3*)(bonearray + (8 * 32));
										Vec3 shoulderL = *(Vec3*)(bonearray + (13 * 32));
										Vec3 brasR = *(Vec3*)(bonearray + (9 * 32));
										Vec3 brasL = *(Vec3*)(bonearray + (14 * 32));
										Vec3 handR = *(Vec3*)(bonearray + (11 * 32));
										Vec3 handL = *(Vec3*)(bonearray + (16 * 32));
										Vec3 cock = *(Vec3*)(bonearray + (0 * 32));
										Vec3 kneesR = *(Vec3*)(bonearray + (23 * 32));
										Vec3 kneesL = *(Vec3*)(bonearray + (26 * 32));
										Vec3 feetR = *(Vec3*)(bonearray + (24 * 32));
										Vec3 feetL = *(Vec3*)(bonearray + (27 * 32));

										Vec3 Ahead, Acou, AshoulderR, AshoulderL, AbrasR, AbrasL, AhandR, AhandL, Acock, AkneesR, AkneesL, AfeetR, AfeetL;

										bool shouldDraw = true;
										auto boneColor = ImColor(255, 0, 0, 255);

										if ((!wrld_to_screen(head, Ahead, view_matrix)) ||
											(!wrld_to_screen(cou, Acou, view_matrix)) ||
											(!wrld_to_screen(shoulderR, AshoulderR, view_matrix)) ||
											(!wrld_to_screen(shoulderL, AshoulderL, view_matrix)) ||
											(!wrld_to_screen(brasR, AbrasR, view_matrix)) ||
											(!wrld_to_screen(brasL, AbrasL, view_matrix)) ||
											(!wrld_to_screen(handL, AhandL, view_matrix)) ||
											(!wrld_to_screen(handR, AhandR, view_matrix)) ||
											(!wrld_to_screen(cock, Acock, view_matrix)) ||
											(!wrld_to_screen(kneesR, AkneesR, view_matrix)) ||
											(!wrld_to_screen(kneesL, AkneesL, view_matrix)) ||
											(!wrld_to_screen(feetR, AfeetR, view_matrix)) ||
											(!wrld_to_screen(feetL, AfeetL, view_matrix))
											) {
											shouldDraw = false;
										}

										if (shouldDraw && g_en_bone) {
											ImGui::GetForegroundDrawList()->AddCircle(ImVec2(Ahead.x, Ahead.y), 3, ImGui::ColorConvertFloat4ToU32(boneColor), 32, 1.0f);
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(Acou.x, Acou.y), ImVec2(Ahead.x, Ahead.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(Acou.x, Acou.y), ImVec2(AshoulderR.x, AshoulderR.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(Acou.x, Acou.y), ImVec2(AshoulderL.x, AshoulderL.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AbrasL.x, AbrasL.y), ImVec2(AshoulderL.x, AshoulderL.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AbrasR.x, AbrasR.y), ImVec2(AshoulderR.x, AshoulderR.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AbrasR.x, AbrasR.y), ImVec2(AhandR.x, AhandR.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AbrasL.x, AbrasL.y), ImVec2(AhandL.x, AhandL.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(Acou.x, Acou.y), ImVec2(Acock.x, Acock.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AkneesR.x, AkneesR.y), ImVec2(Acock.x, Acock.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AkneesL.x, AkneesL.y), ImVec2(Acock.x, Acock.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AkneesL.x, AkneesL.y), ImVec2(AfeetL.x, AfeetL.y), ImGui::ColorConvertFloat4ToU32(boneColor));
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(AkneesR.x, AkneesR.y), ImVec2(AfeetR.x, AfeetR.y), ImGui::ColorConvertFloat4ToU32(boneColor));
										}

										if (shouldDraw && g_en_line) {
											ImGui::GetForegroundDrawList()->AddLine(ImVec2(Acock.x, Acock.y), ImVec2((RENDERER_INSTANCE.win_width * .5f), RENDERER_INSTANCE.win_height), ImGui::ColorConvertFloat4ToU32(boneColor));
										}

										if (shouldDraw && g_en_box) {
											Vec3 pos = *(Vec3*)(entityPawn + Offset::Player.m_vOldOrigin);
											Vec3 Apos;
											if (wrld_to_screen(pos, Apos, view_matrix)) {
												float height = Apos.y - Ahead.y;
												float width = height / 2.4f;

												// Convert screen coordinates to ImGui coordinates
												ImVec2 startPos(Ahead.x - width / 2, Ahead.y);
												ImVec2 endPos(startPos.x + width, startPos.y + height);

												ImGui::GetForegroundDrawList()->AddRect(startPos, endPos, boneColor, 0.0f, 0, 1.0f);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
};