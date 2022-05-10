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
int random_strategy(const board_t board) { return random_strategy::player(board); }
int spam_corner_strategy(const board_t board) { return spam_corner_strategy::player(board); }
int ordered_strategy(const board_t board) { return ordered_strategy::player(board); }
int rotating_strategy(const board_t board) { return rotating_strategy::player(board); }

int rand_trials_strategy(const board_t board) { return rand_trials_strategy::player(board); }
void init_rand_trials_strategy_with_merge_heuristic(const int d, const int t) { rand_trials_strategy::init(d, t, heuristics::merge_heuristic); }
void init_rand_trials_strategy_with_score_heuristic(const int d, const int t) { rand_trials_strategy::init(d, t, heuristics::score_heuristic); }
void init_rand_trials_strategy_with_corner_heuristic(const int d, const int t) { rand_trials_strategy::init(d, t, heuristics::corner_heuristic); }

int minimax_strategy(const board_t board) { return minimax_strategy::player(board); }
void init_minimax_strategy_with_merge_heuristic(const int d) { minimax_strategy::init(d, heuristics::merge_heuristic); }
void init_minimax_strategy_with_score_heuristic(const int d) { minimax_strategy::init(d, heuristics::score_heuristic); }
void init_minimax_strategy_with_corner_heuristic(const int d) { minimax_strategy::init(d, heuristics::corner_heuristic); }

int expectimax_strategy(const board_t board) { return expectimax_strategy::player(board); }
void init_expectimax_strategy_with_merge_heuristic(const int d) { expectimax_strategy::init(d, heuristics::merge_heuristic); }
void init_expectimax_strategy_with_score_heuristic(const int d) { expectimax_strategy::init(d, heuristics::score_heuristic); }
void init_expectimax_strategy_with_corner_heuristic(const int d) { expectimax_strategy::init(d, heuristics::corner_heuristic); }

int monte_carlo_strategy(const board_t board) { return monte_carlo_strategy::player(board); }
void init_monte_carlo_strategy(const int d) { monte_carlo_strategy::init(d); }

}

