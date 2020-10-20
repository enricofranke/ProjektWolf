#include "includes.h"

//read system memory
template<typename T> T RPM(uintptr_t adress) {
	try { return *(T*)adress; }
	catch (...) { return T(); }
}

//wirtes memory to system
template<typename T> void WPM(uintptr_t adress, T value) {
	try { *(T*)adress = value; }
	catch (...) { return; }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);
}


uintptr_t gameModule;
bool init = false;
//init falue that winsdow is closed on Open
bool show = false;

//defualt settings
static int fov = 90;


long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!init)
	{
		gameModule = (DWORD)GetModuleHandle("client.sll");
		InitImGui(pDevice);
		init = true;
	}
	
	//Closes Cheat with "END" KEY
	if (GetAsyncKeyState(VK_END)) {
		kiero::shutdown();
		return 0;
	}
	
	//Switches Overlay Menu State
	if (GetAsyncKeyState(VK_F1)& 1) {
		show = !show;
	}

	//showes the Overlay or Hide
	if (show) {
		//get Propertys 
		//uintptr_t LocalPlayer = RPM<uintptr_t>(gameModule + dwLocalPlayer);


		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//begin new Overlay Window
		ImGui::Begin("Einstellungen");

		//new Slider for FOV min 180 max 180
		ImGui::SliderInt("FOV Settings", &fov, -180, 180);
		//WPM<int>(LocalPlayer + m_iDefualtFOV, fov);


		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
		{
			kiero::bind(42, (void**)& oEndScene, hkEndScene);
			do
				window = GetProcessWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}