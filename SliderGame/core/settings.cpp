#include "settings.h"

c_settings g_settings;

void c_settings::default_settings()
{
	// gui
	gui.color_first_pl[0] = 0.314f;
	gui.color_first_pl[1] = 0.51f;
	gui.color_first_pl[2] = 0.f;

	gui.color_second_pl[0] = 0.f;
	gui.color_second_pl[1] = 0.51f;
	gui.color_second_pl[2] = 1.f;

	// game
	game.game_mode = 1;
}
