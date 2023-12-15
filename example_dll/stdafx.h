#pragma once

#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <atomic>
#include <string>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>

#include <MinHook.h>
#pragma comment(lib, "minhook.lib")

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "memory/memory.h"

#include "utils/xorstr.hpp"

#include "virtualizer/virtualizersdk.h"