#include "injection.h"
#include "files.h"
#include "ntos.h"

#include <TlHelp32.h>

namespace injection {
	// manual map
	namespace manual_map {
		byte thread_hijack_shell[] = {
				0x51, // push rcx
				0x50, // push rax
				0x52, // push rdx
				0x48, 0x83, 0xEC, 0x20, // sub rsp, 0x20
				0x48, 0xB9, // movabs rcx, ->
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x48, 0xBA, // movabs rdx, ->
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x48, 0xB8, // movabs rax, ->
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xFF, 0xD0, // call rax
				0x48, 0xBA, // movabs rdx, ->
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x48, 0x89, 0x54, 0x24, 0x18, // mov qword ptr [rsp + 0x18], rdx
				0x48, 0x83, 0xC4, 0x20, // add rsp, 0x20
				0x5A, // pop rdx
				0x58, // pop rax
				0x59, // pop rcx
				0xFF, 0x64, 0x24, 0xE0 // jmp qword ptr [rsp - 0x20]
		};

		DWORD GetProcId(const wchar_t* procName)
		{
			DWORD procId = 0;
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

			if (hSnap != INVALID_HANDLE_VALUE) {

				PROCESSENTRY32W procEntry;
				procEntry.dwSize = sizeof(procEntry);

				if (Process32FirstW(hSnap, &procEntry))
				{
					do
					{
						if (!_wcsicmp(procEntry.szExeFile, procName))
						{
							procId = procEntry.th32ProcessID;
							break;
						}
					} while (Process32NextW(hSnap, &procEntry));
				}
				CloseHandle(hSnap);
			}
			return procId;
		}

		IMAGE_SECTION_HEADER* translate_raw_section(IMAGE_NT_HEADERS* nt, DWORD rva) {
			auto section = IMAGE_FIRST_SECTION(nt);
			for (auto i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) {
				if (rva >= section->VirtualAddress && rva < section->VirtualAddress + section->Misc.VirtualSize)
					return section;
			}

			return NULL;
		}

		void* translate_raw(char* base, IMAGE_NT_HEADERS* nt, DWORD rva) {
			auto section = translate_raw_section(nt, rva);
			if (!section)
				return NULL;

			return base + section->PointerToRawData + (rva - section->VirtualAddress);
		}

		template <class F>
		bool resolve_imports(char* base, IMAGE_NT_HEADERS* nt, F get_proc_address) {
			auto rva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
			if (!rva)
				return true;

			auto import = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(translate_raw(base, nt, rva));
			if (!import)
				return true;

			for (; import->FirstThunk; ++import) {
				auto module_name = reinterpret_cast<char*>(translate_raw(base, nt, import->Name));
				if (!module_name)
					break;

				for (auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(translate_raw(base, nt, import->FirstThunk)); thunk->u1.AddressOfData; ++thunk) {
					auto by_name = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(translate_raw(base, nt, static_cast<DWORD>(thunk->u1.AddressOfData)));
					thunk->u1.Function = get_proc_address(module_name, by_name->Name);
				}
			}

			return true;
		}

		void resolve_relocations(char* base, IMAGE_NT_HEADERS* nt, byte* mapped) {
			auto& base_relocation = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
			if (!base_relocation.VirtualAddress)
				return;

			auto reloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(translate_raw(base, nt, base_relocation.VirtualAddress));
			if (!reloc)
				return;

			for (auto current_size = 0UL; current_size < base_relocation.Size; ) {
				auto count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
				auto rdata = reinterpret_cast<WORD*>(reinterpret_cast<byte*>(reloc) + sizeof(IMAGE_BASE_RELOCATION));
				auto rbase = reinterpret_cast<byte*>(translate_raw(base, nt, reloc->VirtualAddress));

				for (auto i = 0UL; i < count; ++i, ++rdata) {
					auto data = *rdata;
					auto type = data >> 12;
					auto offset = data & 0xFFF;

					if (type == IMAGE_REL_BASED_DIR64)
						*reinterpret_cast<PBYTE*>(rbase + offset) += (mapped - reinterpret_cast<PBYTE>(nt->OptionalHeader.ImageBase));
				}

				current_size += reloc->SizeOfBlock;
				reloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(rdata);
			}
		}

		int inject(DWORD process_id, const wchar_t* dll_path) {
			auto buffer = files::load_binary(dll_path);
			auto dll = new char[buffer.length()]; memcpy(dll, buffer.data(), buffer.length());
			auto module = LoadLibraryExW(dll_path, NULL, DONT_RESOLVE_DLL_REFERENCES);

			IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(dll);
			if (dos->e_magic != IMAGE_DOS_SIGNATURE)
				return -1;

			IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uint64_t>(dll) + dos->e_lfanew);
			if (nt->Signature != IMAGE_NT_SIGNATURE)
				return -2;

			/* write image to process */

			TOKEN_PRIVILEGES priv = { 0 };
			HANDLE hToken = NULL;
			if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
				priv.PrivilegeCount = 1;
				priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

				if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
					AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

				CloseHandle(hToken);
			}

			BOOLEAN bl;
			RtlAdjustPrivilege(20, TRUE, FALSE, &bl);

			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
			if (!process) {
				DWORD Err = GetLastError();
				//printf("OpenProcess failed: 0x%X\n", Err);
				return -9;
			}

			//Finding a thread to hijack
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			THREADENTRY32 te32;
			te32.dwSize = sizeof(te32);

			Thread32First(hSnap, &te32);

			while (Thread32Next(hSnap, &te32))
			{
				if (te32.th32OwnerProcessID == process_id)
				{
					//ILog("Target thread found. Thread ID: %d\n", te32.th32ThreadID);
					break;
				}
			}

			CloseHandle(hSnap);

			void* image = VirtualAllocEx(process, NULL, nt->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!image) {
				CloseHandle(process);
				return -3;
			}

			// resolve imports
			if (!resolve_imports(dll, nt, [](const char* module_name, const char* function_name) { return reinterpret_cast<uint64_t>(GetProcAddress(LoadLibraryA(module_name), function_name)); }))
				return -4;

			// resolve relocations
			resolve_relocations(dll, nt, reinterpret_cast<byte*>(image));

			// copy headers to image
			WriteProcessMemory(process, image, dll, nt->OptionalHeader.SizeOfHeaders, NULL);

			// copy sections to image
			IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
			for (size_t i = 0; i < nt->FileHeader.NumberOfSections; i++) {
				WriteProcessMemory(process, reinterpret_cast<void*>(reinterpret_cast<uint64_t>(image) + section[i].VirtualAddress),
					reinterpret_cast<void*>(reinterpret_cast<uint64_t>(dll) + section[i].PointerToRawData), section[i].SizeOfRawData, NULL);
			}

			/* initialize security cookie */

			auto init_cookie = [&]() {
				if (!nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress)
					return 1;
				
				auto load_cfg = reinterpret_cast<IMAGE_LOAD_CONFIG_DIRECTORY64*>(reinterpret_cast<uint64_t>(dll) + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress);
				uint64_t security_cookie = load_cfg->SecurityCookie;
				if (!security_cookie)
					return 1;

				// taken from blackbone, credits to darth
				FILETIME sys_time = {};
				LARGE_INTEGER performance_count = { {} };

				GetSystemTimeAsFileTime(&sys_time);
				QueryPerformanceCounter(&performance_count);

				uint64_t cookie = process_id ^ te32.th32ThreadID ^ reinterpret_cast<uintptr_t>(&security_cookie);

				cookie ^= *reinterpret_cast<uint64_t*>(&sys_time);
				cookie ^= (performance_count.QuadPart << 32) ^ performance_count.QuadPart;
				cookie &= 0xFFFFFFFFFFFF;
				if (cookie == 0x2B992DDFA232)
					cookie++;

				return WriteProcessMemory(process, reinterpret_cast<void*>(reinterpret_cast<uint64_t>(image) + (cookie - reinterpret_cast<uint64_t>(dll))), &cookie, sizeof(void*), NULL);
			};

			init_cookie();

			/* execute image */

			void* loader = VirtualAllocEx(process, NULL, sizeof(thread_hijack_shell), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!loader)
				return -5;

			HANDLE thread = OpenThread(THREAD_ALL_ACCESS, false, te32.th32ThreadID);
			auto shell = thread_hijack_shell;
			memcpy(shell + 9, &image, sizeof(void*));

			void* reason = reinterpret_cast<void*>(DLL_PROCESS_ATTACH);
			memcpy(shell + 19, &reason, sizeof(void*));

			//void* entry_point = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(image) + (reinterpret_cast<char*>(GetProcAddress(module, "DllMain")) - reinterpret_cast<char*>(module)));
			void* entry_point = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(image) + nt->OptionalHeader.AddressOfEntryPoint);
			memcpy(shell + 29, &entry_point, sizeof(void*));

			CONTEXT ctx = {}; ctx.ContextFlags = CONTEXT_FULL;
			if (SuspendThread(thread) == -1 || !GetThreadContext(thread, &ctx)) {
				VirtualFreeEx(process, image, NULL, MEM_RELEASE); VirtualFreeEx(process, loader, NULL, MEM_RELEASE);
				CloseHandle(process); CloseHandle(thread);
				return -6;
			}

			memcpy(shell + 41, &ctx.Rip, sizeof(void*));
			if (!WriteProcessMemory(process, loader, shell, sizeof(thread_hijack_shell), NULL)) {
				VirtualFreeEx(process, image, NULL, MEM_RELEASE); VirtualFreeEx(process, loader, NULL, MEM_RELEASE);
				CloseHandle(process); CloseHandle(thread);
				return -7;
			}

			ctx.Rip = reinterpret_cast<uint64_t>(loader);
			if (!SetThreadContext(thread, &ctx) || ResumeThread(thread) == -1) {
				VirtualFreeEx(process, image, NULL, MEM_RELEASE); VirtualFreeEx(process, loader, NULL, MEM_RELEASE);
				CloseHandle(process); CloseHandle(thread);
				return -8;
			}

			CloseHandle(process); CloseHandle(thread);

			return 1;
		}

		int inject(DWORD process_id, std::wstring dll_path) {
			return inject(process_id, dll_path.c_str());
		}
	}
};