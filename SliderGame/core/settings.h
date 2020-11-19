#pragma once
#include "../globals.h"

class c_settings
{
public:
	struct s_gui_settings
	{
		float color_first_pl[3];
		float color_second_pl[3];
	};

	struct s_game_settings
	{
		bool game_mode; // 0 - player2play, 1 - player2PC
		int difficulty_level;
	};

	c_settings() { default_settings(); }
	~c_settings() {}

	void default_settings();

	s_gui_settings gui;
	s_game_settings game;
}; 
extern c_settings g_settings;