#pragma once
#include <optional>
#include "offsets.hpp"
#include "../../utils/Vector.h"
#include "../../imgui/Renderer.h"

struct view_matrix_t {
	float* operator[ ](int index) {
		return matrix[index];
	}

	float matrix[4][4];
};

class CGlobalVarsBase {
public:
	float real_time;
	std::int32_t frame_count;
	std::uint8_t padding_0[0x8];
	std::int32_t max_clients;
	float interval_per_tick;
	std::uint8_t padding_1[0x14];
	float current_time;
	float current_time_2;
	std::uint8_t padding_2[0xC];
	std::int32_t tick_count;
	float interval_per_tick_2;
	std::uint8_t padding_3[0x138];
	std::uint64_t current_map;
	std::uint64_t current_map_name;

};

class CGame
{
private:
	struct
	{
		uintptr_t ClientDLL;
		uintptr_t EntityList;
		uintptr_t EntityListEntry;
		uintptr_t LocalController;
		uintptr_t LocalPawn;
		uintptr_t GlobalVarsBase;
		uintptr_t ViewMatrix;
	}Address;

public:

	bool inline InitAddress() {
		this->Address.ClientDLL = reinterpret_cast<uintptr_t>(GetModuleHandleA(XS("client.dll")));

		this->Address.EntityList = GetClientDLLAddress() + Offset::dwEntityList;
		this->Address.LocalController = GetClientDLLAddress() + Offset::dwLocalPlayerController;
		this->Address.LocalPawn = GetClientDLLAddress() + Offset::dwLocalPlayerPawn;
		this->Address.GlobalVarsBase = GetClientDLLAddress() + Offset::dwGlobalVars;
		this->Address.ViewMatrix = GetClientDLLAddress() + Offset::dwViewMatrix;

		return this->Address.ClientDLL != 0;
	}

	uintptr_t inline GetClientDLLAddress() {
		return this->Address.ClientDLL;
	}

	uintptr_t inline GetEntityListAddress() {
		return this->Address.EntityList;
	}

	uintptr_t inline GetEntityListEntry() {
		return this->Address.EntityListEntry;
	}

	uintptr_t inline GetLocalControllerAddress() {
		return this->Address.LocalController;
	}

	uintptr_t inline GetViewMatrix() {
		return this->Address.ViewMatrix;
	}

	uintptr_t inline GetLocalPawnAddress() {
		return this->Address.LocalPawn;
	}

	CGlobalVarsBase inline *GetCGlobalVarsBase() {
		uintptr_t _GlobalVarsBase = 0;
		_GlobalVarsBase = *(uintptr_t*)this->Address.GlobalVarsBase;

		return (CGlobalVarsBase*)(_GlobalVarsBase);
	}
};

inline CGame gGame;