#include "pch.h"
#include "Render.h"
#include "Utils.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dwmapi.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Dwmapi.lib")

WNDCLASSEX wc;
HWND drawHwnd;
GameScreen ScreenInfo;
LPDIRECT3D9 pD3D = NULL;
LPDIRECT3DDEVICE9 pD3dDevice = NULL;
D3DPRESENT_PARAMETERS d3dParam{};

void InitScreenInfo()
{
	HWND GameHwnd = FindTopGameWindow();
	if (RECT rect; GameHwnd && GetWindowRect(GameHwnd, &rect))
	{
		ScreenInfo.windowWidth = rect.right - rect.left;
		ScreenInfo.windowHeight = rect.bottom - rect.top;
		ScreenInfo.positionX = rect.left;
		ScreenInfo.positionY = rect.top;
	}
}

bool CreateDeviceD3D(HWND hWnd)
{
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
	{
		return false;
	}
	d3dParam.Windowed = TRUE;
	d3dParam.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dParam.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dParam.EnableAutoDepthStencil = TRUE;
	d3dParam.AutoDepthStencilFormat = D3DFMT_D16;
	d3dParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dParam.BackBufferFormat = D3DFMT_A8R8G8B8;
	return (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dParam, &pD3dDevice) >= 0);
}

void CleanupDeviceD3D()
{
	if (pD3dDevice)
	{
		pD3dDevice->Release();
		pD3dDevice = NULL;
	}
	if (pD3D)
	{
		pD3D->Release();
		pD3D = NULL;
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
	static MARGINS zero = { -1, -1, -1, -1 };
	switch (msg)
	{
	case WM_PAINT:
		DwmExtendFrameIntoClientArea(hWnd, &zero);
		break;
	case WM_SIZE:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Render::Init()
{
	//获取当前游戏窗口信息，注意当前版本不支持游戏窗口变动
	InitScreenInfo();

	//创建一个顶层窗口覆盖游戏窗口用于绘制
	wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"b", NULL };
	RegisterClassEx(&wc);
	drawHwnd = CreateWindow(wc.lpszClassName, L"a", WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE, ScreenInfo.positionX,
		ScreenInfo.positionY, ScreenInfo.windowWidth, ScreenInfo.windowHeight, NULL, NULL, wc.hInstance, NULL);
	SetWindowLong(drawHwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetLayeredWindowAttributes(drawHwnd, 0, 255, LWA_ALPHA);
	UpdateWindow(drawHwnd);

	//D3D初始化
	if (!CreateDeviceD3D(drawHwnd))
	{
		CleanupDeviceD3D();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		return;
	}

	//IMGUI初始化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	ImGui_ImplWin32_Init(drawHwnd);
	ImGui_ImplDX9_Init(pD3dDevice);

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
}

void Render::NewFrame()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2((float)ScreenInfo.windowWidth, (float)ScreenInfo.windowHeight), ImGuiCond_Always);
	ImGui::Begin(" ", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);
	SetWindowPos(drawHwnd, HWND_TOPMOST, ScreenInfo.positionX, ScreenInfo.positionY,
		ScreenInfo.windowWidth, ScreenInfo.windowHeight, SWP_NOSIZE);
}

void Render::ShowFrame()
{
	ImGui::End();
	ImGui::EndFrame();
	pD3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pD3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pD3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
	D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f), (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
	pD3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
	if (pD3dDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		pD3dDevice->EndScene();
	}
	HRESULT result = pD3dDevice->Present(NULL, NULL, NULL, NULL);
}

void Render::Release()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(drawHwnd);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

void Render::Line(ImVec2 a, ImVec2 b, ImColor color, float thickness)
{
	ImGui::GetWindowDrawList()->AddLine(a, b, color, thickness);
}

/*
	 -------
	|		|
	|		|
	|		|
	 -------
*/
void Render::DrawBox(ImColor color, int x, int y, int w, int h)
{
	Line(ImVec2((float)x, (float)y), ImVec2((float)x + w, (float)y), color, 2.0f);
	Line(ImVec2((float)x, (float)y), ImVec2((float)x, (float)y + h), color, 2.0f);
	Line(ImVec2((float)x + w, (float)y), ImVec2((float)x + w, (float)y + h), color, 2.0f);
	Line(ImVec2((float)x, (float)y + h), ImVec2((float)x + w, (float)y + h), color, 2.0f);
}