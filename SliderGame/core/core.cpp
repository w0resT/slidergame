#include "core.h"

namespace core
{
	std::vector<std::pair<ImVec2, ImVec2>> made_moves;
	std::vector<std::pair<int, int>> made_moves_idx;
	std::vector<bool> made_moves_player;
	std::pair<int, int> headtail;
	std::string message;
	bool map_dot[25];
	ImVec2 map_dot_pos[25];
	bool player;

	// PC
	int possible_moves[4];
	int best_way_h;
	int best_way_t;
	int best_score_h;
	int best_score_t;
	bool pc_map_dot[25];
}

void core::reset_data()
{
	for (int i = 0; i < 25; ++i)
	{
		map_dot[i] = false;
		pc_map_dot[i] = false;
		map_dot_pos[i] = ImVec2(0, 0);
	}

	made_moves.clear();
	made_moves_idx.clear();
	made_moves_player.clear();
	message.clear();
	headtail.first = -1;
	headtail.second = -1;
	player = false; // first player move

	// PC
	possible_moves[0] = -5; // Up
	possible_moves[1] = 5; // Down
	possible_moves[2] = 1; // Right
	possible_moves[3] = -1; // Left
	best_way_h = 0;
	best_way_t = 0;
	best_score_h = 0;
	best_score_t = 0;
}

// idx1 - from; idx2  - to
bool core::is_possible_move(int idx1, int idx2, int _type)
{
	// Function to chek the border crossing
	// True - we go beyond the borders
	// False - all ok
	auto check_bound = [](int idx1, int idx2) -> bool
	{
		if ((idx1 >= 0 && idx1 <= 4) && (idx2 >= 0 && idx2 <= 4)
			|| (idx1 >= 5 && idx1 <= 9) && (idx2 >= 5 && idx2 <= 9)
			|| (idx1 >= 10 && idx1 <= 14) && (idx2 >= 10 && idx2 <= 14)
			|| (idx1 >= 15 && idx1 <= 19) && (idx2 >= 15 && idx2 <= 19)
			|| (idx1 >= 20 && idx1 <= 24) && (idx2 >= 20 && idx2 <= 24))
			return true;

		return false;
	};

	// Deafult mode
	if (!_type)
	{
		// Если ход не с головы/хвоста
		if (headtail.first != idx1 && headtail.second != idx1)
		{
			message.append("You can only moves from the head or from the tail!");
			return false;
		}

		// Выходим за границы поля
		if (idx2 > 24 || idx2 < 0)
			return false;

		// Если ходим не вверх\вниз\вправо\влево
		// idx1 - 5  Up
		// idx1 + 5  Down
		// idx1 + 1  Right
		// idx1 - 1  Left
		if ((idx2 != (idx1 - 5)) 
			&& (idx2 != (idx1 + 5))
			&& (idx2 != (idx1 + 1))
			&& (idx2 != (idx1 - 1)))
		{
			message.append("You can only move up, down, right or left!");
			return false;
		}

		// Мы не можем перескакивать использую границы
		if (idx2 == (idx1 + 1) && !check_bound(idx2, idx1)
			|| idx2 == (idx1 - 1) && !check_bound(idx2, idx1))
		{
			message.append("You can only move up, down, right or left!");
			return false;
		}

		// Назад ходить нельзя
		if (!made_moves_idx.empty() 
			&& !(idx2 == headtail.first && idx1 == headtail.second && made_moves_idx.size() != 1)
			&& (idx2 == made_moves_idx.back().first || idx2 == made_moves_idx.back().second))
		{
			message.append("You can't move back!");
			return false;
		}
	}
	else // Условия для PC
	{
		// Выходим за пределы поля
		if (idx2 > 24 || idx2 < 0)
			return false;

		// Ходим не вверх/вниз/влево/вправо
		if (idx2 != (idx1 - 5) && idx2 != (idx1 + 5) && idx2 != (idx1 + 1) && idx2 != (idx1 - 1))
			return false;

		// Если хотим перескачить на другую строчку при движении влево/вправо
		if (idx2 == (idx1 + 1) && !check_bound(idx2, idx1) || idx2 == (idx1 - 1) && !check_bound(idx2, idx1))
			return false;
	}
	return true;
}

bool core::check_state(int idx1, int idx2)
{
	if (map_dot[idx2]) // Ходим в уже использованную точку => проигрыш
		return false;

	return true;
}

int core::evaluation_recursion(int curr_idx, int level, int prev_idx)
{
	int curr_level = level;
	int return_value = 0;

	// Max level = 3
	if (curr_level > 3)
		return return_value;

	// If you came to an already marked point
	if (!core::check_state(prev_idx, curr_idx))
		return return_value;

	for (int i = 0; i < 4; ++i)
	{
		// If we're going to go backwards
		if ( (i == 0 && curr_idx - prev_idx == 5)
			|| (i == 1 && curr_idx - prev_idx == -5)
			|| (i == 2 && curr_idx - prev_idx == -1)
			|| (i == 3 && curr_idx - prev_idx == 1))
		{
			continue;
		}

		if (core::is_possible_move(curr_idx, (curr_idx + core::possible_moves[i]), 1) && core::check_state(prev_idx, (curr_idx + core::possible_moves[i])))
		{
			return_value += evaluation_recursion((curr_idx + core::possible_moves[i]), (curr_level + 1), curr_idx);
			switch (curr_level)
			{
			case 1:
				return_value += 1;
				break;
			case 2:
				return_value += 3;
				break;
			case 3:
				return_value += 9;
				break;
			default:
				break;
			}
		}
	}

	return return_value;
}

int core::evaluation(int curr_idx, int level, int ht) // ht: 0 - Head, 1 - Tail
{
	int return_value = 0;

	/*
		level 1 - 1
		level 2 - 3
		level 3 - 9
		idx1 - 5  Up	0
		idx1 + 5  Down	1
		idx1 + 1  Right	2
		idx1 - 1  Left	3
	*/

	int up_score = 0, down_score = 0, right_score = 0, left_score = 0;

	// Up
	if(is_possible_move(curr_idx, (curr_idx + possible_moves[0]), 1))
		up_score += evaluation_recursion((curr_idx + possible_moves[0]), 1, curr_idx);

	// Down
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[1]), 1))
		down_score += evaluation_recursion((curr_idx + possible_moves[1]), 1, curr_idx);

	// Right
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[2]), 1))
		right_score += evaluation_recursion((curr_idx + possible_moves[2]), 1, curr_idx);

	// Left
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[3]), 1))
		left_score += evaluation_recursion((curr_idx + possible_moves[3]), 1, curr_idx);

	int max_1, max_2;
	int temp_way_1 = 0, temp_way_2 = 0;

	max_1 = max(up_score, down_score);
	temp_way_1 = max_1 == up_score ? 1 : 2;

	max_2 = max(right_score, left_score);
	temp_way_2 = max_2 == right_score ? 3 : 4;

	// Sets best way head, best way tail and best score for both
	if (max_1 >= max_2)
	{
		return_value = max_1;
		ht > 0 ? (best_way_t = max_1 == 0 ? 0 : temp_way_1) : (best_way_h = max_1 == 0 ? 0 : temp_way_1);
	}
	else
	{
		return_value = max_2;
		ht > 0 ? (best_way_t = max_2 == 0 ? 0 : temp_way_2) : (best_way_h = max_2 == 0 ? 0 : temp_way_2);
	}

	return return_value;
}
