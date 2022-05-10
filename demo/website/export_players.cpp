#include "../../game/game.cpp"
#include "../../heuristics.cpp"
#include "../../strategies/blind/random.cpp"
#include "../../strategies/blind/spam_corner.cpp"
#include "../../strategies/blind/ordered.cpp"
#include "../../strategies/blind/rotating.cpp"
#include "../../strategies/rand_trials.cpp"
#include "../../strategies/minimax.cpp"
#include "../../strategies/expectimax.cpp"
#include "../../strategies/monte_carlo.cpp"

extern "C" {

void init_game() { game::init(); }
int random_player(const board_t board) { return random_player::player(board); }
int spam_corner_player(const board_t board) { return spam_corner_player::player(board); }
int ordered_player(const board_t board) { return ordered_player::player(board); }
int rotating_player(const board_t board) { return rotating_player::player(board); }

int rand_trials_player(const board_t board) { return rand_trials_player::player(board); }
void init_rand_trials_player_with_merge_heuristic(const int d, const int t) { rand_trials_player::init(d, t, heuristics::merge_heuristic); }
void init_rand_trials_player_with_score_heuristic(const int d, const int t) { rand_trials_player::init(d, t, heuristics::score_heuristic); }
void init_rand_trials_player_with_corner_heuristic(const int d, const int t) { rand_trials_player::init(d, t, heuristics::corner_heuristic); }

int minimax_player(const board_t board) { return minimax_player::player(board); }
void init_minimax_player_with_merge_heuristic(const int d) { minimax_player::init(d, heuristics::merge_heuristic); }
void init_minimax_player_with_score_heuristic(const int d) { minimax_player::init(d, heuristics::score_heuristic); }
void init_minimax_player_with_corner_heuristic(const int d) { minimax_player::init(d, heuristics::corner_heuristic); }

int expectimax_player(const board_t board) { return expectimax_player::player(board); }
void init_expectimax_player_with_merge_heuristic(const int d) { expectimax_player::init(d, heuristics::merge_heuristic); }
void init_expectimax_player_with_score_heuristic(const int d) { expectimax_player::init(d, heuristics::score_heuristic); }
void init_expectimax_player_with_corner_heuristic(const int d) { expectimax_player::init(d, heuristics::corner_heuristic); }

int monte_carlo_player(const board_t board) { return monte_carlo_player::player(board); }
void init_monte_carlo_player(const int d) { monte_carlo_player::init(d); }

}

