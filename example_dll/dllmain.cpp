#include "stdafx.h"
#include "mainmenu/mainmenu.h"
#include "callbacks/callbacks.h"
#include "h4x/scripts/mainh4x.hpp"

HWND g_hWnd;

ID3D11Device *device = nullptr;
ID3D11DeviceContext *immediateContext = nullptr;
ID3D11RenderTargetView *renderTargetView = nullptr;

HRESULT(*D3D11PSSetShaderResourcesOriginal)(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews) = nullptr;
HRESULT(*DrawIndexedOriginal)(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) = nullptr;

//HRESULT(*D3D11CreateQueryOriginal)(ID3D11Device* pDevice, const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery) = nullptr;

HRESULT(*PresentOriginal)(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) = nullptr;
HRESULT(*ResizeOriginal)(IDXGISwapChain *swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;
//HRESULT(*ClearRenderTargetViewOriginal)(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4]) = nullptr;

std::atomic_bool running;
uintptr_t initThreadHandle;
HMODULE g_hModule;

PVOID present;
PVOID resize;
//PVOID clearRenderTargetView;

//PVOID d3D11CreateQuery;

PVOID d3D11PSSetShaderResources;
PVOID drawIndexed;

WNDPROC	oWndProc;

void InitCallBacks() {
	CALLBACKS_INSTANCE->AddOnImguiInitCallBacks(XS("MAINH4X"), MAINH4X::OnImguiInit);
	CALLBACKS_INSTANCE->AddTabItemsContent(XS("MAINH4X"), MAINH4X::Draw);
	CALLBACKS_INSTANCE->AddOnPresentCallBacks(XS("MAINH4X_OnPresent"), MAINH4X::OnPresent);
}

void CleanupImgui() {
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

	renderTargetView->Release();
	immediateContext->Release();
	device->Release();
	device = nullptr;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (MAINMENU_INSTANCE->getIsShowMenu()) {
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
		return true;
	}
	return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

UINT pssrStartSlot;
D3D11_SHADER_RESOURCE_VIEW_DESC Descr;
HRESULT D3D11PSSetShaderResourcesHook(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
	pssrStartSlot = StartSlot;

	for (UINT j = 0; j < NumViews; j++)
	{
		ID3D11ShaderResourceView* pShaderResView = ppShaderResourceViews[j];
		if (pShaderResView)
		{
			pShaderResView->GetDesc(&Descr);

			if ((Descr.ViewDimension == D3D11_SRV_DIMENSION_BUFFER) || (Descr.ViewDimension == D3D11_SRV_DIMENSION_BUFFEREX))
			{
				continue; //Skip buffer resources
			}
		}
	}

	return D3D11PSSetShaderResourcesOriginal(pContext, StartSlot, NumViews, ppShaderResourceViews);
}

HRESULT DrawIndexedHook(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	return DrawIndexedOriginal(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

/*HRESULT D3D11CreateQueryHook(ID3D11Device* pDevice, const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery)
{
	if (pQueryDesc->Query == D3D11_QUERY_OCCLUSION)
	{
		D3D11_QUERY_DESC oqueryDesc = CD3D11_QUERY_DESC();
		(&oqueryDesc)->MiscFlags = pQueryDesc->MiscFlags;
		(&oqueryDesc)->Query = D3D11_QUERY_TIMESTAMP;

		return D3D11CreateQueryOriginal(pDevice, &oqueryDesc, ppQuery);
	}

	return D3D11CreateQueryOriginal(pDevice, pQueryDesc, ppQuery);
}*/

HRESULT PresentHook(IDXGISwapChain *swapChain, UINT syncInterval, UINT flags) {
	if (!device) {
		swapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID *>(&device));
		device->GetImmediateContext(&immediateContext);

		ID3D11Texture2D *renderTarget = nullptr;
		swapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID *>(&renderTarget));
		device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView);
		renderTarget->Release();

		DXGI_SWAP_CHAIN_DESC desc;
		swapChain->GetDesc(&desc);

		g_hWnd = desc.OutputWindow;

		oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->AddFontDefault();
		io.Fonts->Build();
		io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

		RECT rect;
		if (GetWindowRect(g_hWnd, &rect))
		{
			RENDERER_INSTANCE.win_width = rect.right - rect.left;
			RENDERER_INSTANCE.win_height = rect.bottom - rect.top;
		}

		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(g_hWnd);
		ImGui_ImplDX11_Init(device, immediateContext);

		for (auto const& [key, val] : CALLBACKS_INSTANCE->OnImguiInitCallBacks)
		{
			val();
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	auto _flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::GetStyle().AntiAliasedFill = true;
	ImGui::GetStyle().AntiAliasedLines = true;

	ImGui::Begin(XS("##overlay"), nullptr, _flags);

	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

	MAINMENU_INSTANCE->ShenMenu();

	for (auto const& [key, val] : CALLBACKS_INSTANCE->OnPresentCallBacks)
	{
		val();
	}

	ImGui::GetForegroundDrawList()->PushClipRectFullScreen();
	ImGui::End();
	ImGui::EndFrame();

	// Rendering
	ImGui::Render();
	immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return PresentOriginal(swapChain, syncInterval, flags);
}

HRESULT ResizeHook(IDXGISwapChain *swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
	CleanupImgui();
	return ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}

/*HRESULT ClearRenderTargetViewHook(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
	return ClearRenderTargetViewOriginal(pContext, pRenderTargetView, ColorRGBA);
}*/

void OnExit() noexcept
{
	if (running)
	{
		running = false;
		MH_DisableHook(d3D11PSSetShaderResources);
		MH_DisableHook(drawIndexed);
		//MH_DisableHook(d3D11CreateQuery);
		MH_DisableHook(present);
		MH_DisableHook(resize);
		//MH_DisableHook(clearRenderTargetView);
		MH_Uninitialize();

		CleanupImgui();

		WaitForSingleObject((HANDLE)initThreadHandle, INFINITE);
		CloseHandle((HANDLE)initThreadHandle);
	}
	FreeLibraryAndExitThread(g_hModule, 0);
}

VOID Main() {
	//VirtualizerStart();

	std::atexit(OnExit);

	CALLBACKS_INSTANCE = new CALLBACKS();
	MAINMENU_INSTANCE = new MAINMENU();

	InitCallBacks();

	IDXGISwapChain      *swapChain    = nullptr;
	ID3D11Device        *device       = nullptr;
	ID3D11DeviceContext *context      = nullptr;

	g_hWnd = GetForegroundWindow();

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	DXGI_SWAP_CHAIN_DESC sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.Windowed = ((GetWindowLongPtr(g_hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		sd.BufferDesc.Width = 1;
		sd.BufferDesc.Height = 1;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 1;
	}

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &sd, &swapChain, &device, &obtainedLevel, &context);
	if (FAILED(hr))
	{
		MessageBoxA(0, XS("Failed to create device and swapchain."), XS("Fatal Error"), MB_ICONERROR);
		return;
	}

	auto table_swapChain = *reinterpret_cast<PVOID **>(swapChain);
	present = table_swapChain[8];
	resize = table_swapChain[13];
	//clearRenderTargetView = table_swapChain[50];

	auto table_device = *reinterpret_cast<PVOID**>(device);
	//d3D11CreateQuery = table_device[24];

	auto table_context = *reinterpret_cast<PVOID**>(context);
	d3D11PSSetShaderResources = table_context[8];
	drawIndexed = table_context[12];

	context->Release();
	device->Release();
	swapChain->Release();

	MH_Initialize();

	MH_CreateHook(d3D11PSSetShaderResources, D3D11PSSetShaderResourcesHook, reinterpret_cast<PVOID*>(&D3D11PSSetShaderResourcesOriginal));
	MH_EnableHook(d3D11PSSetShaderResources);

	MH_CreateHook(drawIndexed, DrawIndexedHook, reinterpret_cast<PVOID*>(&DrawIndexedOriginal));
	MH_EnableHook(drawIndexed);

	//MH_CreateHook(d3D11CreateQuery, D3D11CreateQueryHook, reinterpret_cast<PVOID*>(&D3D11CreateQueryOriginal));
	//MH_EnableHook(d3D11CreateQuery);

	MH_CreateHook(present, PresentHook, reinterpret_cast<PVOID *>(&PresentOriginal));
	MH_EnableHook(present);

	MH_CreateHook(resize, ResizeHook, reinterpret_cast<PVOID *>(&ResizeOriginal));
	MH_EnableHook(resize);

	//MH_CreateHook(clearRenderTargetView, ClearRenderTargetViewHook, reinterpret_cast<PVOID*>(&ClearRenderTargetViewOriginal));
	//MH_EnableHook(clearRenderTargetView);
	
	running = true;

	//VirtualizerEnd();
}

BOOL APIENTRY DllMain(HMODULE _module, DWORD reason, LPVOID reserved) {

	DisableThreadLibraryCalls(_module);
	Memory::UnlinkModuleFromPEB(_module);
	Memory::FakePeHeader(_module);

	if (reason == DLL_PROCESS_ATTACH)
	{
		g_hModule = _module;
		//initThreadHandle = _beginthreadex(nullptr, 0, (_beginthreadex_proc_type)Main, NULL, 0, nullptr);
		Main();
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		OnExit();
	}

	return TRUE;
}