#pragma once
#include "../globals.h"

namespace gui
{
	extern bool setup_window(HINSTANCE m_hInstance);
	extern void release_window(HINSTANCE m_hInstance);
	extern void main();

	extern void main_window();

	extern void load_style();

	extern LPCTSTR lpz_class;
	extern HWND hwnd;
}