#include "../stdafx.h"
#include "memory.h"
#include <map>

#pragma warning(disable:4191) //Fuck FARPROC casting

std::vector<Memory::UNLINKED_MODULE> Memory::m_unlinkedModules;

void Memory::RelinkModuleToPEB(HMODULE hModule) {
	auto it = find_if(m_unlinkedModules.begin(), m_unlinkedModules.end(), FindModuleHandle(hModule));

	if (it == m_unlinkedModules.end()) {
		return;
	}

	RELINK((*it).Entry->InLoadOrderLinks, (*it).RealInLoadOrderLinks);
	RELINK((*it).Entry->InInitializationOrderLinks, (*it).RealInInitializationOrderLinks);
	RELINK((*it).Entry->InMemoryOrderLinks, (*it).RealInMemoryOrderLinks);
	m_unlinkedModules.erase(it);
}

bool Memory::UnlinkModuleFromPEB(HMODULE hModule) {
	auto it = find_if(m_unlinkedModules.begin(), m_unlinkedModules.end(), FindModuleHandle(hModule));
	if (it != m_unlinkedModules.end()) {
		return false;
	}
	PPEB pPEB = (PPEB)__readgsqword(0x60);

	PLIST_ENTRY CurrentEntry = pPEB->Ldr->InLoadOrderModuleList.Flink;
	PLDR_DATA_TABLE_ENTRY Current = NULL;

	while (CurrentEntry != &pPEB->Ldr->InLoadOrderModuleList && CurrentEntry != NULL) {

		Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		if (Current->DllBase == hModule) {

			UNLINKED_MODULE CurrentModule = { 0 };
			CurrentModule.hModule = hModule;
			CurrentModule.RealInLoadOrderLinks = Current->InLoadOrderLinks.Blink->Flink;
			CurrentModule.RealInInitializationOrderLinks = Current->InInitializationOrderLinks.Blink->Flink;
			CurrentModule.RealInMemoryOrderLinks = Current->InMemoryOrderLinks.Blink->Flink;
			CurrentModule.Entry = Current;
			m_unlinkedModules.push_back(CurrentModule);

			UNLINK(Current->InLoadOrderLinks);
			UNLINK(Current->InInitializationOrderLinks);
			UNLINK(Current->InMemoryOrderLinks);

			break;
		}
		CurrentEntry = CurrentEntry->Flink;
	}
	return true;
}

static std::map<HMODULE, BYTE[0x1000]> originalHeaderBytes;
void Memory::FakePeHeader(HMODULE hModule) {
	void * pKernel32 = reinterpret_cast<void*>(GetModuleHandleA("kernel32.dll"));
	DWORD dwOriginal = 0;
	HANDLE proc = GetCurrentProcess();
	VirtualProtectEx(proc, hModule, 0x1000, PAGE_EXECUTE_READWRITE, &dwOriginal);
	uint8_t wot[0x1000];
	memcpy(&wot[0], hModule, 0x1000);
	for(auto i = 0; i < 0x1000; i++) {
		wot[i] ^= 0x69;
	}
	WriteProcessMemory(proc, hModule, pKernel32, 0x1000, nullptr);
	VirtualProtectEx(proc, hModule, 0x1000, dwOriginal, &dwOriginal);
}

void Memory::RestorePeHeaders() {
	for(auto& mod : originalHeaderBytes) {
		uint8_t dexor[0x1000];
		for(auto i = 0; i < 0x1000; i++) {
			dexor[i] = mod.second[i] ^ 0x69;
		}

		memcpy(mod.first, dexor, 0x1000);
	}
}