#include "core.h"
#include "../gui/gui.h"

void c_core::on_move(int n, int payload_n)
{
	static bool first_ = false;

	auto curr = current_session;

	if (!curr->message.empty())
		curr->message.clear();

	if (curr->headtail.first == -1)
	{
		curr->headtail.first = payload_n;
		curr->headtail.second = n;
		first_ = true;
	}

	if (is_possible_move(payload_n, n))
	{
		if (!check_state(payload_n, n))
		{
			curr->message.append("You lose.");
			gui::show_message_window = true;
			set_game_active(false);
		}

		if (!first_)
		{
			if (payload_n == curr->headtail.first)
				curr->headtail.first = n;

			if (payload_n == curr->headtail.second)
				curr->headtail.second = n;
		}

		first_ = false;

		curr->map_dot[n] = true;
		curr->map_dot[payload_n] = true;

		if (get_game_active())
			curr->player = !curr->player;

		std::pair<ImVec2, ImVec2> temp;
		temp.first = curr->map_dot_pos[payload_n];
		temp.second = curr->map_dot_pos[n];
		curr->made_moves.push_back(temp);

		std::pair<int, int> temp_2;
		temp_2.first = payload_n;
		temp_2.second = n;
		curr->made_moves_idx.push_back(temp_2);

		curr->made_moves_player.push_back(get_game_active() == true ? curr->player : !curr->player);

		calculate_score();
	}
}

void c_core::reset_data()
{
	auto curr = current_session;

	for (int i = 0; i < 25; ++i)
	{
		curr->map_dot[i] = false;
		curr->map_dot_pos[i] = ImVec2(0, 0);
	}

	curr->best_way_h = 0;
	curr->best_way_t = 0;
	curr->best_score_h = 0;
	curr->best_score_t = 0;

	curr->made_moves.clear();
	curr->made_moves_idx.clear();
	curr->made_moves_player.clear();
	curr->message.clear();
	curr->headtail.first = -1;
	curr->headtail.second = -1;

	curr->player = false; // first player move
}

void c_core::calculate_score()
{
	current_session->best_score_h = evaluation(current_session->headtail.second, 0, 0);
	current_session->best_score_t = evaluation(current_session->headtail.first, 0, 1);
}

bool c_core::is_possible_move(int _from, int _to, int _type)
{
	// Function to chek the border crossing
	// True - we go beyond the borders
	// False - all ok
	auto check_bound = [](int _from, int _to) -> bool
	{
		if ((_from >= 0 && _from <= 4) && (_to >= 0 && _to <= 4)
			|| (_from >= 5 && _from <= 9) && (_to >= 5 && _to <= 9)
			|| (_from >= 10 && _from <= 14) && (_to >= 10 && _to <= 14)
			|| (_from >= 15 && _from <= 19) && (_to >= 15 && _to <= 19)
			|| (_from >= 20 && _from <= 24) && (_to >= 20 && _to <= 24))
			return true;

		return false;
	};

	auto curr = current_session;

	// Deafult mode
	if (!_type)
	{
		// If the move is not from the head/tail
		if (curr->headtail.first != _from && curr->headtail.second != _from)
		{
			curr->message.append("You can only moves from the head or from the tail!");
			return false;
		}

		// Going beyond boundaries of field
		if (_to > 24 || _to < 0)
			return false;

		// If we are not going up/down/right/left
		// idx1 - 5  Up
		// idx1 + 5  Down
		// idx1 + 1  Right
		// idx1 - 1  Left
		if ((_to != (_from - 5)) && (_to != (_from + 5)) && (_to != (_from + 1)) && (_to != (_from - 1)))
		{
			curr->message.append("You can only move up, down, right or left!");
			return false;
		}

		// If we want to jump to another line using borders
		if (_to == (_from + 1) && !check_bound(_to, _from) || _to == (_from - 1) && !check_bound(_to, _from))
		{
			curr->message.append("You can only move up, down, right or left!");
			return false;
		}

		// We can't move back 
		if (!curr->made_moves_idx.empty()
			&& !(_to == curr->headtail.first && _from == curr->headtail.second && curr->made_moves_idx.size() != 1)
			&& (_to == curr->made_moves_idx.back().first || _to == curr->made_moves_idx.back().second))
		{
			curr->message.append("You can't move back!");
			return false;
		}
	}
	else // For PC or evaluation
	{
		// Going beyond boundaries of field
		if (_to > 24 || _to < 0)
			return false;

		// If we are not going up/down/right/left
		if (_to != (_from - 5) && _to != (_from + 5) && _to != (_from + 1) && _to != (_from - 1))
			return false;

		// If we want to jump to another line using borders
		if (_to == (_from + 1) && !check_bound(_to, _from) || _to == (_from - 1) && !check_bound(_to, _from))
			return false;
	}
	return true;
}

bool c_core::check_state(int _from, int _to)
{
	if (current_session->map_dot[_to]) // We go to the already used point = lose
		return false;

	return true;
}

int c_core::evaluation(int curr_idx, int level, int ht)
{
	int return_value = 0;
	int up_score = 0, down_score = 0, right_score = 0, left_score = 0;

	auto curr = current_session;

	// Up
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[0]), 1))
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
		ht > 0 ? (curr->best_way_t = max_1 == 0 ? 0 : temp_way_1) : (curr->best_way_h = max_1 == 0 ? 0 : temp_way_1);
	}
	else
	{
		return_value = max_2;
		ht > 0 ? (curr->best_way_t = max_2 == 0 ? 0 : temp_way_2) : (curr->best_way_h = max_2 == 0 ? 0 : temp_way_2);
	}

	return return_value;
}

int c_core::evaluation_recursion(int curr_idx, int level, int prev_idx)
{
	int curr_level = level;
	int return_value = 0;

	// Max level = 3
	if (curr_level > 3)
		return return_value;

	// If you came to an already marked point
	if (!check_state(prev_idx, curr_idx))
		return return_value;

	for (int i = 0; i < 4; ++i)
	{
		// If we're going to go backwards
		if ((i == 0 && curr_idx - prev_idx == 5)
			|| (i == 1 && curr_idx - prev_idx == -5)
			|| (i == 2 && curr_idx - prev_idx == -1)
			|| (i == 3 && curr_idx - prev_idx == 1))
		{
			continue;
		}

		if (is_possible_move(curr_idx, (curr_idx + possible_moves[i]), 1) && check_state(prev_idx, (curr_idx + possible_moves[i])))
		{
			return_value += evaluation_recursion((curr_idx + possible_moves[i]), (curr_level + 1), curr_idx);
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
