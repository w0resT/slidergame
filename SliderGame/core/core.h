#pragma once
#include "../globals.h"

namespace core
{
	extern void reset_data();
	extern bool is_possible_move(int idx1, int idx2, int _type = 0);
	extern bool check_state(int idx1, int idx2);

	extern int evaluation(int curr_idx, int level = 0, int ht = 0);
	extern int evaluation_recursion(int curr_idx, int level, int prev_idx);

	extern std::vector<std::pair<ImVec2, ImVec2>> made_moves;
	extern std::vector<std::pair<int, int>> made_moves_idx;
	extern std::vector<bool> made_moves_player;
	extern std::pair<int, int> headtail;
	extern std::string message;
	extern bool map_dot[25];
	extern ImVec2 map_dot_pos[25];
	extern bool player; // false - first player move, true - second player move

	// For PC
	extern int possible_moves[4];
	extern int best_way_h;
	extern int best_way_t;
	extern int best_score_h;
	extern int best_score_t;
	extern bool pc_map_dot[25];
}