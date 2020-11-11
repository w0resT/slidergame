#pragma once
#include "../globals.h"

namespace helpers
{
	extern ATOM register_window(HINSTANCE hInstance, LPCTSTR lpzClassName);
	extern bool create_device_d3d(HWND hWnd);
	extern void cleanup_device_d3d();
	extern void reset_device();
}