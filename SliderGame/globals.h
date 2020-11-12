#pragma once

#include <d3d9.h>
#include <utility> // std::pair
#include <vector>
#include <string>

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300
#define DIRECTINPUT_VERSION 0x0800

#define max(a,b) (((a) > (b)) ? (a) : (b))

namespace globals
{
	extern LPDIRECT3D9 d3d;
	extern LPDIRECT3DDEVICE9 d3d_device;
	extern D3DPRESENT_PARAMETERS d3d_params;

	extern bool exit;
}