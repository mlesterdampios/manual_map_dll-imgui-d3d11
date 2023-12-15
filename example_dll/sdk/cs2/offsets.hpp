#pragma once
#include <Windows.h>

namespace Offset
{
	inline size_t dwEntityList = 0x17C18E0;
	inline size_t dwLocalPlayerController = 0x1810EE0;
	inline size_t dwLocalPlayerPawn = 0x16C8ED8;
	inline size_t dwGlobalVars = 0x16BDE28;
	inline size_t dwViewMatrix = 0x1820100;

	// https://github.com/a2x/cs2-dumper/blob/main/generated/client.dll.hpp
	struct
	{
		size_t m_flDetectedByEnemySensorTime = 0x13E4; // m_flDetectedByEnemySensorTime
		size_t m_hPlayerPawn = 0x7EC;
	} Entity;

	// https://github.com/a2x/cs2-dumper/blob/main/generated/client.dll.hpp
	struct
	{
		size_t m_iHealth = 0x32C; // m_iHealth  
		size_t m_pGameSceneNode = 0x310; // m_pGameSceneNode  
		size_t BoneArray = 0x1E0; // BoneArray 
		size_t m_iTeamNum = 0x3BF; // m_iTeamNum
		size_t m_vOldOrigin = 0x1224; // m_vOldOrigin
	} Player;
}