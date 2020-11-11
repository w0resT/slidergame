#pragma once
#include "../globals.h"

namespace core
{
	extern void reset_data();

	extern std::vector<std::pair<ImVec2, ImVec2>> made_moves;
	extern std::vector<std::pair<int, int>> made_moves_idx;
	extern bool map_dot[25];
	extern ImVec2 map_dot_pos[25];
}