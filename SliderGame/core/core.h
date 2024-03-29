#pragma once
#include "../globals.h"

class c_core
{
private:
	struct s_session
	{
		s_session()
		{
			for (int i = 0; i < 25; ++i)
			{
				map_dot[i] = false;
				map_dot_pos[i] = ImVec2(0, 0);
			}

			best_way_h = 0;
			best_way_t = 0;
			best_score_h = 0;
			best_score_t = 0;

			made_moves.clear();
			made_moves_idx.clear();
			made_moves_player.clear();
			message.clear();
			headtail.first = -1;
			headtail.second = -1;

			player = false; // first player move
		}

		int best_way_h;
		int best_way_t;
		int best_score_h;
		int best_score_t;

		std::vector<std::pair<ImVec2, ImVec2>> made_moves;
		std::vector<std::pair<int, int>> made_moves_idx;
		std::vector<bool> made_moves_player;
		std::pair<int, int> headtail;
		std::string message;
		ImVec2 map_dot_pos[25];
		bool map_dot[25];
		bool player; // false - first player move, true - second player move
	};

	s_session *current_session;
	s_session *temp_session[5];

	bool game_active;

public:
	c_core() 
	{ 
		possible_moves[0] = -5; // Up
		possible_moves[1] = 5; // Down
		possible_moves[2] = 1; // Right
		possible_moves[3] = -1; // Left

		game_active = true;

		current_session = new s_session;
		*temp_session = new s_session[5];
	}
	~c_core() 
	{	
		//delete current_session; 
		//delete[] temp_session;
	}

	void on_move(int n, int payload_n);
	void run_bot();
	void reset_data();
	void calculate_score();
	void set_game_active(bool _b) { game_active = _b; }
	bool get_game_active() { return game_active; }

	s_session *get_current_session() { return current_session; }

	int possible_moves[4];

private:
	bool is_possible_move(int _from, int _to, int _type = 0);
	bool check_state(int _from, int _to, s_session *curr = nullptr);
	int evaluation(int curr_idx, int level = 0, int ht = 0, s_session *curr = nullptr);
	int evaluation_recursion(int curr_idx, int level, int prev_idx, s_session *curr);

	// return number of the best way, 0 - up, 1 - down, 2 - right, 3 - left
	int run_prediction(int _player, int level = 0, int ht = 0, int *_score = nullptr);
	void presetup_bot(int _level); // temp_session = current_session
	void setup_bot(s_session *curr, int _way); // doing temp moves
	void restore_bot(); // restoring temp_session to current_session and removing all moves
	void reset_temp_data();
	void do_move(int _way, int ht);
	int ambush(int curr_idx);
};