#pragma once
#include "../globals.h"
#include "../core/core.h"

namespace gui
{
	enum msg_type
	{
		STATE_MSG = 0,
		ERROR_MSG
	};

	extern bool setup_window(HINSTANCE m_hInstance);
	extern void release_window(HINSTANCE m_hInstance);
	extern void main();

	extern void main_window(c_core &game);
	extern void game_window(c_core &game);
	extern void message_window(c_core &game, msg_type msg_type);
	extern void info_window();
	extern void setting_window();

	extern void load_style();
	extern std::string get_way_by_idx(int idx);
	extern std::string get_level_by_idx(int idx);

	extern LPCTSTR lpz_class;
	extern HWND hwnd;

	extern bool show_new_game_window;
	extern bool show_message_window;
	extern bool show_info_window;
	extern bool show_setting_window;

	extern ImGuiWindowFlags window_flags;
}