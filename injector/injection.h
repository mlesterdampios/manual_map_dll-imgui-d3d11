#pragma once

#include <windows.h>
#include <string>

namespace injection {
	namespace manual_map { // note: requires message sent to window procedure, could be activated via SendMessageA or by hovering over the window with your mouse
		DWORD GetProcId(const wchar_t* procName);
		int inject(DWORD process_id, const wchar_t* dll_path);
		int inject(DWORD process_id, std::wstring dll_path);
	}
}