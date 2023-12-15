#pragma once
#include <cmath>
#include <imgui.h>
#include "../utils/Vector.h"

enum e_flags
{
	// circle
	circle_normal,
	circle_filled,
	//circle_3d,

	// rectangle
	rect_normal,
	rect_filled,

	// triangle
	triangle_normal,
	triangle_filled,

	// text
	text_normal,
	text_with_font
};

class RENDERER_BASE {
private:

public:

	int win_height = 0;
	int win_width = 0;

	virtual inline bool isOnScreen(Vec2* point) {};

	virtual inline void draw_text(float x, float y, const char* text, bool outlined,
		ImColor color = ImColor(255, 255, 255, 255), e_flags flags = text_normal, ImFont* font = nullptr, ...) {};

	virtual inline void draw_line(float x1, float y1, float x2, float y2,
		ImColor color = ImColor(255, 255, 255, 255), float thickness = 1.0f) {};

	virtual inline void draw_rect(float x, float y, float w, float h,
		ImColor color = ImColor(255, 255, 255, 255), e_flags flags = rect_normal, float rounding = 0.0f, uintptr_t points = 12, float thickness = 1.0f) {};

	virtual inline void draw_triangle(float x1, float y1, float x2, float y2,
		float x3, float y3, ImColor color = ImColor(255, 255, 255, 255), e_flags flags = triangle_normal, float thickness = 1.0f) {};

	virtual inline void draw_circle(Vec2 Position, float_t radius, ImColor color, e_flags flags, uintptr_t points, float_t thickness) {};
};

class RENDERER final : public RENDERER_BASE {
private:

public:

	inline bool isOnScreen(Vec2* point) override
	{
		return point->x > -0.f && point->x < win_width + 0.f && point->y > -0.f && point->y < win_height + 0.f;
	}

	inline void draw_text(float x, float y, const char* text, bool outlined,
		ImColor color = ImColor(255, 255, 255, 255), e_flags flags = text_normal, ImFont* font = nullptr, ...) override
	{
		switch (flags)
		{
		case text_normal:

			if (outlined)
			{
				ImGui::GetWindowDrawList()->AddText(ImVec2(x, y + 1.0f), ImColor(0, 0, 0, 255), text);
				ImGui::GetWindowDrawList()->AddText(ImVec2(x + 1.0f, y), ImColor(0, 0, 0, 255), text);
			}

			ImGui::GetWindowDrawList()->AddText(ImVec2(x, y), color, text);
			break;
		case text_with_font:

			if (outlined)
			{
				ImGui::GetWindowDrawList()->AddText(font, font->FontSize, ImVec2(x, y + 1.0f), ImColor(0, 0, 0, 255), text);
				ImGui::GetWindowDrawList()->AddText(font, font->FontSize, ImVec2(x + 1.0f, y), ImColor(0, 0, 0, 255), text);
			}

			ImGui::GetWindowDrawList()->AddText(font, font->FontSize, ImVec2(x, y), color, text);
			break;
		default:
			break;
		}
	}

	inline void draw_line(float x1, float y1, float x2, float y2,
		ImColor color = ImColor(255, 255, 255, 255), float thickness = 1.0f) override
	{
		ImGui::GetWindowDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
	}

	inline void draw_rect(float x, float y, float w, float h,
		ImColor color = ImColor(255, 255, 255, 255), e_flags flags = rect_normal, float rounding = 0.0f, uintptr_t points = 12, float thickness = 1.0f) override
	{
		switch (flags)
		{
		case rect_normal:
			ImGui::GetWindowDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, rounding, points, thickness);
			break;
		case rect_filled:
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, rounding, points);
			break;
		default:
			break;
		}
	}

	inline void draw_triangle(float x1, float y1, float x2, float y2,
		float x3, float y3, ImColor color = ImColor(255, 255, 255, 255), e_flags flags = triangle_normal, float thickness = 1.0f) override
	{
		switch (flags)
		{
		case rect_normal:
			ImGui::GetWindowDrawList()->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), color, thickness);
			break;
		case rect_filled:
			ImGui::GetWindowDrawList()->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), color);
			break;
		default:
			break;
		}
	}

	inline void draw_circle(Vec2 Position, float_t radius, ImColor color, e_flags flags, uintptr_t points, float_t thickness) override
	{
		switch (flags)
		{
		case circle_normal:
			ImGui::GetWindowDrawList()->AddCircle(ImVec2(Position.x, Position.y), radius, color, points, thickness);
			break;
		case circle_filled:
			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(Position.x, Position.y), radius, color, points);
			break;
		default:
			break;
		}
	}
};

inline RENDERER RENDERER_INSTANCE;