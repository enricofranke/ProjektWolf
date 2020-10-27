#include "includes.h"
#include "entity.h"
#include "styles.h"
#include<type_traits>
#include<cstdint>

//read system memory
template<typename T, typename = std::enable_if<std::is_trivially_copyable<T>::value>> 
auto RPM(const uintptr_t adress) noexcept -> T {
	return *reinterpret_cast<T*>(adress);
}

//wirtes memory to system
template<typename T, typename = std::enable_if<std::is_trivially_copyable<T>::value>>
void WPM(const uintptr_t adress, const T value) noexcept {
	*reinterpret_cast<T*>(adress) = value;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = nullptr;
WNDPROC oWndProc;
static HWND window = nullptr;

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);
}

//define
enum class SigOnState
{
	STATE_NONE = 0,
	CHALLENGE = 1,
	CONNECTED = 2,
	NEW = 3,
	PRESPAWN = 4,
	SPAWN = 5,
	FULL_CONNECTED = 6,
	CHANGELEVEL = 7
};

uintptr_t gameModule;
uintptr_t engineModule;
bool init = false;
//init falue that winsdow is closed on Open
bool show = false;
static int switchTabs = 3;
//defualt settings
static int fov = 90;
static bool WallsOn = false;
static bool aimBot = false;

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!init) {
		gameModule = static_cast<DWORD>(GetModuleHandle("client.dll"));
		engineModule = static_cast<DWORD>(GetModuleHandle("engine.dll"));
		InitImGui(pDevice);
		ImGui::SetNextWindowPos(ImVec2(650, 200));

		init = true;
		loadStyle();
	}
	
	//Closes Cheat with "END" KEY
	if (GetAsyncKeyState(VK_END)) {
		kiero::shutdown();
		return 0;
	}
	
	//Switches Overlay Menu State
	if (GetAsyncKeyState(VK_F1) & 1) {
		show = !show;
	}
	//get Propertys 
	const uintptr_t LocalPlayer = RPM<uintptr_t>(gameModule + signatures::dwLocalPlayer);
	const uintptr_t GlowObjectManager = RPM<uintptr_t>(gameModule + signatures::dwGlowObjectManager);
	const uintptr_t EntityList = RPM<uintptr_t>(gameModule + signatures::dwEntityList);

	if (aimBot) {

	//TODO Aimbot	


	}

	if (WallsOn) {
	
		if (LocalPlayer != 0 && GlowObjectManager != 0 && EntityList != 0) {

			int myTeamNum = *reinterpret_cast<int*>(LocalPlayer + netvars::m_iTeamNum);
			//The number of Players that are going to be checked with Team
			for (auto i = 0; i < 64; ++i) {

				auto* const entity = *reinterpret_cast<Entity**>(gameModule + signatures::dwEntityList + i * 0x10);
				if (entity == nullptr || entity->GetDormant()) continue;

				const int glowIndex = *reinterpret_cast<int*>(entity + netvars::m_iGlowIndex);
				const int entTeamNum = *reinterpret_cast<int*>(entity + netvars::m_iTeamNum);

				auto applyColor = [&](const float r, const float g, const float b) noexcept -> void {
					*reinterpret_cast<float*>((GlowObjectManager)+((glowIndex * 0x38) + 0x4)) = r;
					*reinterpret_cast<float*>((GlowObjectManager)+((glowIndex * 0x38) + 0x8)) = g;
					*reinterpret_cast<float*>((GlowObjectManager)+((glowIndex * 0x38) + 0xC)) = b;
					*reinterpret_cast<float*>((GlowObjectManager)+((glowIndex * 0x38) + 0x10)) = 1.7f;
				};

				if (entTeamNum == myTeamNum) {
					// Teammate:
					applyColor(.0f, 1.f, 0.f);
				}
				else {
					// Enemy:
					applyColor(.0f, .0f, 1.f);
				}
				*reinterpret_cast<bool*>((GlowObjectManager)+((glowIndex * 0x38) + 0x24)) = true;
				*reinterpret_cast<bool*>((GlowObjectManager)+((glowIndex * 0x38) + 0x25)) = false;

			}

		}
	}

	if (fov != 90) {
		WPM<int>(LocalPlayer + netvars::m_iFOV, fov);
	}

	//showes the Overlay or Hide
	if (show) {

		ImGui_ImplDX9_NewFrame();	
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//begin new Overlay Window
		// TODO check boolean
		ImGui::Begin("Projekt Wolf");

		if (ImGui::Button("Aimbot", ImVec2(100.0f, 0.0f)))
			switchTabs = 0;
		ImGui::SameLine(0.0, 2.0f);
		if (ImGui::Button("Visuals", ImVec2(100.0f, 0.0f)))
			switchTabs = 1;
		ImGui::SameLine(0.0, 2.0f);
		if (ImGui::Button("Test", ImVec2(100.0f, 0.0f)))
			switchTabs = 2;

		switch (switchTabs) {
		case 0:
			ImGui::Text("Test1");
			break;

		case 1:
			ImGui::BeginChild("child", ImVec2(100, 0), true);

			ImGui::Checkbox("WallHack", &WallsOn);
			ImGui::SameLine();
			ImGui::Text("TODO ADD KEYBIND TOOL");
			ImGui::Text("TODO ADD COLOR CHANGER");
			ImGui::EndChild();
			break;

		case 2:
			ImGui::Text("Test3");
			break;
		}


		ImGui::SliderInt("FOV Settings", &fov, 10, 150);

		

		ImGui::Text("Test");

		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return true; // skip to next window

	window = handle;
	return false; // window found abort search
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
			kiero::bind(42, reinterpret_cast<void**>(&oEndScene), hkEndScene);
			do
				window = GetProcessWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);
	return true;
}

bool WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
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
	return true;
}


