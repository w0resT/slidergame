#include "helpers.h"
#include "../resource.h"
#include "../wndproc.h"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ATOM helpers::register_window(HINSTANCE hInstance, LPCTSTR lpzClassName)
{
	WNDCLASS window_class = { 0 };
	window_class.lpfnWndProc = (WNDPROC)WndProc;
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.hInstance = hInstance;
	window_class.lpszClassName = lpzClassName;
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;
	window_class.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0);

	return RegisterClass(&window_class);
}

bool helpers::create_device_d3d(HWND hWnd)
{
	if ((globals::d3d = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&globals::d3d_params, sizeof(globals::d3d_params));
	globals::d3d_params.Windowed = TRUE;
	globals::d3d_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	globals::d3d_params.BackBufferFormat = D3DFMT_UNKNOWN;
	globals::d3d_params.EnableAutoDepthStencil = TRUE;
	globals::d3d_params.AutoDepthStencilFormat = D3DFMT_D16;
	globals::d3d_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (globals::d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &globals::d3d_params, &globals::d3d_device) < 0)
		return false;

	return true;
}

void helpers::cleanup_device_d3d()
{
	if (globals::d3d_device) { globals::d3d_device->Release(); globals::d3d_device = NULL; }
	if (globals::d3d) { globals::d3d->Release(); globals::d3d = NULL; }
}

void helpers::reset_device()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	HRESULT m_hResult = globals::d3d_device->Reset(&globals::d3d_params);
	if (m_hResult == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}