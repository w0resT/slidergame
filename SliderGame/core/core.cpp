#include "core.h"
#include "settings.h"
#include "../gui/gui.h"

void c_core::on_move(int n, int payload_n)
{
	static bool first_ = false;

	auto curr = current_session;
	if (!curr)
		return;

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
		if (!check_state(payload_n, n, curr))
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

		// Switch player
		if (get_game_active())
			curr->player = !curr->player;

		// Remember the coordinates of the move
		std::pair<ImVec2, ImVec2> temp;
		temp.first = curr->map_dot_pos[payload_n];
		temp.second = curr->map_dot_pos[n];
		curr->made_moves.push_back(temp);

		// Remember the indices of the move
		std::pair<int, int> temp_2;
		temp_2.first = payload_n;
		temp_2.second = n;
		curr->made_moves_idx.push_back(temp_2);

		curr->made_moves_player.push_back(get_game_active() == true ? curr->player : !curr->player);

		//calculate_score();
	}
}

void c_core::run_bot()
{
	/*
		TODO: 

	*/

	int best_way = -1;

	auto curr = current_session;
	if (!curr)
		return;

	// Best score of each ways
	int score_h = 0, score_t = 0;

	// Getting best way for move
	int best_way_h = run_prediction(curr->player, 0, 0, &score_h);
	int best_way_t = run_prediction(curr->player, 0, 1, &score_t);

	int ht = 0;

	// If best_way = 0 => none normal moves => do random move if it possible
	if (best_way_h == 0 && best_way_t == 0)
	{
		// We have one more move
		for (int i = 0; i < 4; ++i)
		{
			// Head
			if(is_possible_move(curr->headtail.second, curr->headtail.second + possible_moves[i], 1))
			{
				if (check_state(curr->headtail.second, curr->headtail.second + possible_moves[i], curr))
				{
					best_way = i + 1;
					ht = 0;
					break;
				}
			}

			// Tail
			if (is_possible_move(curr->headtail.first, curr->headtail.first + possible_moves[i], 1))
			{
				if (check_state(curr->headtail.first, curr->headtail.first + possible_moves[i], curr))
				{
					best_way = i + 1;
					ht = 1;
					break;
				}	
			}
		}

		// We still don't have any way => we lose so just doing random move
		if (best_way == -1)
		{
			for (int i = 0; i < 4; ++i)
			{
				// Head
				if (is_possible_move(curr->headtail.second, curr->headtail.second + possible_moves[i], 1))
				{
					best_way = i + 1;
					ht = 0;
					break;
				}

				// Tail
				if (is_possible_move(curr->headtail.first, curr->headtail.first + possible_moves[i], 1))
				{
					best_way = i + 1;
					ht = 1;
					break;
				}
			}
		}
	}
	else // Choose the best move from the tail or from the head
	{
		if (score_h >= score_t)
		{
			best_way = best_way_h;
			ht = 0;
		}
		else
		{
			best_way = best_way_t;
			ht = 1;
		}
	}

	// If we block the move for the opponent, thereby he loses
	int amb = ambush(curr->best_way_t == 0 ? curr->headtail.second : curr->headtail.first);
	if (amb && g_settings.game.difficulty_level > 0)
	{
		best_way = amb;
		ht = curr->best_way_t == 0 ? 0 : 1;
	}

	do_move(best_way, ht);
}

void c_core::reset_data()
{
	auto curr = current_session;
	if (!curr)
		return;

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
	current_session->best_score_h = evaluation(current_session->headtail.second, 0, 0, current_session);
	current_session->best_score_t = evaluation(current_session->headtail.first, 0, 1, current_session);
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
	if (!curr)
		return false;

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

bool c_core::check_state(int _from, int _to, s_session *curr)
{
	if (curr->map_dot[_to]) // We go to the already used point = lose
		return false;

	return true;
}

int c_core::evaluation(int curr_idx, int level, int ht, s_session *curr)
{
	int return_value = 0;
	int up_score = 0, down_score = 0, right_score = 0, left_score = 0;

	if(!curr)
		curr = current_session;

	// Up
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[0]), 1))
		up_score += evaluation_recursion((curr_idx + possible_moves[0]), 1, curr_idx, curr);

	// Down
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[1]), 1))
		down_score += evaluation_recursion((curr_idx + possible_moves[1]), 1, curr_idx, curr);

	// Right
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[2]), 1))
		right_score += evaluation_recursion((curr_idx + possible_moves[2]), 1, curr_idx, curr);

	// Left
	if (is_possible_move(curr_idx, (curr_idx + possible_moves[3]), 1))
		left_score += evaluation_recursion((curr_idx + possible_moves[3]), 1, curr_idx, curr);

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

int c_core::evaluation_recursion(int curr_idx, int level, int prev_idx, s_session *curr)
{
	int curr_level = level;
	int return_value = 0;

	int max_level = g_settings.game.difficulty_level + 1;
	if (max_level == 3) max_level = 5;

	// Max level - 5
	if (curr_level > max_level)
		return return_value;

	// If you came to an already marked point
	if (!check_state(prev_idx, curr_idx, curr))
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

		if (is_possible_move(curr_idx, (curr_idx + possible_moves[i]), 1) && check_state(prev_idx, (curr_idx + possible_moves[i]), curr))
		{
			return_value += evaluation_recursion((curr_idx + possible_moves[i]), (curr_level + 1), curr_idx, curr);
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
			case 4:
				return_value += 27;
				break;
			case 5:
				return_value += 81;
				break;
			default:
				break;
			}
		}
	}

	return return_value;
}

int c_core::run_prediction(int _player, int level, int ht, int *_score)
{
	//return number of the best way, 1 - up, 2 - down, 3 - right, 4 - left, 0 if NONE

	int curr_level = level;
	int curr_way = 0;

	presetup_bot(curr_level);

	auto curr = temp_session[curr_level];
	auto curr_idx = ht == 0 ? curr->headtail.second : curr->headtail.first;

	*_score = evaluation(curr_idx, curr_level, ht, curr);
	curr_way = ht == 0 ? curr->best_way_h : curr->best_way_t;

	// If we can do the best move at a given level
	if (curr_way != 0 && is_possible_move(curr_idx, (curr_idx + possible_moves[curr_way - 1])))
	{
		return curr_way;
	}

	return 0;
}

void c_core::presetup_bot(int _level)
{
	temp_session[_level] = current_session;
	/*auto curr = temp_session[_level];
	auto orig = current_session;

	for (int i = 0; i < 25; ++i)
	{
		curr->map_dot[i] = orig->map_dot[i];
		curr->map_dot_pos[i] = orig->map_dot_pos[i];
	}

	curr->best_way_h = orig->best_way_h;
	curr->best_way_t = orig->best_way_t;
	curr->best_score_h = orig->best_score_h;
	curr->best_score_t = orig->best_score_t;

	curr->made_moves = orig->made_moves;
	curr->made_moves_idx = orig->made_moves_idx;
	curr->made_moves_player = orig->made_moves_player;
	curr->headtail.first = orig->headtail.first;
	curr->headtail.second = orig->headtail.second;

	curr->player = orig->player; */
}

void c_core::setup_bot(s_session *curr, int _way)
{
	// doing temp moveeee

}

void c_core::restore_bot()
{
	//auto orig = current_session;

	for (int k = 0; k < 5; ++k)
	{
		temp_session[k] = current_session;
		/*auto curr = temp_session[k];

		for (int i = 0; i < 25; ++i)
		{
			curr->map_dot[i] = orig->map_dot[i];
			curr->map_dot_pos[i] = orig->map_dot_pos[i];
		}

		curr->best_way_h = orig->best_way_h;
		curr->best_way_t = orig->best_way_t;
		curr->best_score_h = orig->best_score_h;
		curr->best_score_t = orig->best_score_t;

		curr->made_moves = orig->made_moves;
		curr->made_moves_idx = orig->made_moves_idx;
		curr->made_moves_player = orig->made_moves_player;
		curr->headtail.first = orig->headtail.first;
		curr->headtail.second = orig->headtail.second;

		curr->player = orig->player;*/
	}
}

void c_core::reset_temp_data()
{
	for (auto curr : temp_session)
	{
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
		curr->headtail.first = -1;
		curr->headtail.second = -1;

		curr->player = false; // first player move
	}
}

void c_core::do_move(int _way, int ht)
{
	// 1 - up, 2 - down, 3 - right, 4 - left, 0 if NONE
	// ht: head - 0, tail - 1

	auto curr = current_session;

	int payload_n = ht == 0 ? curr->headtail.second : curr->headtail.first;
	int n = payload_n + possible_moves[_way - 1];

	//if (is_possible_move(payload_n, n, 1))
	{
		if (!check_state(payload_n, n, curr))
		{
			curr->message.append("Bot lose.");
			gui::show_message_window = true;
			set_game_active(false);
		}

		if (payload_n == curr->headtail.first)
			curr->headtail.first = n;

		if (payload_n == curr->headtail.second)
			curr->headtail.second = n;

		curr->map_dot[n] = true;
		curr->map_dot[payload_n] = true;

		// Switch player
		if (get_game_active())
			curr->player = !curr->player;

		// Remember the coordinates of the move
		std::pair<ImVec2, ImVec2> temp;
		temp.first = curr->map_dot_pos[payload_n];
		temp.second = curr->map_dot_pos[n];
		curr->made_moves.push_back(temp);

		// Remember the indices of the move
		std::pair<int, int> temp_2;
		temp_2.first = payload_n;
		temp_2.second = n;
		curr->made_moves_idx.push_back(temp_2);

		curr->made_moves_player.push_back(get_game_active() == true ? curr->player : !curr->player);
	}
}

int c_core::ambush(int curr_idx)
{
	auto curr = current_session;

	static bool can_move = false;

	/*
		If one of the sides of the slider has no moves, then we try to find a move in which the player loses in any case
	*/
	if( curr_idx == curr->headtail.second && curr->best_way_t == 0
		|| curr_idx == curr->headtail.first && curr->best_way_h == 0)
	{
		for (int i = 0; i < 4; ++i)
		{
			//If we're going to go backwards
			if ((i == 0 && (curr_idx + possible_moves[i]) - curr_idx == 5)
				|| (i == 1 && (curr_idx + possible_moves[i]) - curr_idx == -5)
				|| (i == 2 && (curr_idx + possible_moves[i]) - curr_idx == -1)
				|| (i == 3 && (curr_idx + possible_moves[i]) - curr_idx == 1))
			{
				continue;
			}

			can_move = false;

			// We look 1 move forward and then we check if the next player has any moves
			if (is_possible_move(curr_idx, (curr_idx + possible_moves[i]), 1) && check_state(curr_idx, (curr_idx + possible_moves[i]), curr))
			{
				for (int j = 0; j < 4; ++j)
				{
					//If we're going to go backwards
					if ((i == 0 && (curr_idx + possible_moves[i] + possible_moves[j]) - (curr_idx + possible_moves[i]) == 5)
						|| (i == 1 && (curr_idx + possible_moves[i] + possible_moves[j]) - (curr_idx + possible_moves[i]) == -5)
						|| (i == 2 && (curr_idx + possible_moves[i] + possible_moves[j]) - (curr_idx + possible_moves[i]) == -1)
						|| (i == 3 && (curr_idx + possible_moves[i] + possible_moves[j]) - (curr_idx + possible_moves[i]) == 1))
					{
						continue;
					}

					// If the player still has moves, then we cannot close it, so we return true for 'can_move'
					if (is_possible_move(curr_idx + possible_moves[i], (curr_idx + possible_moves[i] + possible_moves[j]), 1)
						&& check_state(curr_idx + possible_moves[i], (curr_idx + possible_moves[i] + possible_moves[j]), curr))
					{
						can_move = true;
					}
				}

				// If the player has no moves, then return the move number that can be used to block the move
				if (!can_move)
					return (i + 1);
			}
		}
	}

	// There is no move that can block the player
	return 0;
}
