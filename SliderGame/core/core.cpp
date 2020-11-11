#include "core.h"

namespace core
{
	std::vector<std::pair<ImVec2, ImVec2>> made_moves;
	std::vector<std::pair<int, int>> made_moves_idx;
	bool map_dot[25];
	ImVec2 map_dot_pos[25];
}

void core::reset_data()
{
	for (int i = 0; i < 25; ++i)
	{
		map_dot[i] = false;
		map_dot_pos[i] = ImVec2(0, 0);
	}

	made_moves.clear();
	made_moves_idx.clear();
}