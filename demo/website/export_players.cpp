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

// TODO write some sort of enum to replace init_x_strategy_with_y_heuristic
void init_game() { game::init(); }
int random_player(const board_t board) { return random_strategy::player(board); }
int spam_corner_player(const board_t board) { return spam_corner_strategy::player(board); }
int ordered_player(const board_t board) { return ordered_strategy::player(board); }
int rotating_player(const board_t board) { return rotating_strategy::player(board); }

int rand_trials_player(const board_t board) { return rand_trials_strategy::player(board); }
void init_rand_trials_strategy(const int d, const int t, const int heuristic_id) {
    rand_trials_strategy::init(d, t, heuristics::exports[heuristic_id]);
}

int minimax_player(const board_t board) { return minimax_strategy::player(board); }
void init_minimax_strategy(const int d, const int heuristic_id) {
    minimax_strategy::init(d, heuristics::exports[heuristic_id]);
}

int expectimax_player(const board_t board) { return expectimax_strategy::player(board); }
void init_expectimax_strategy(const int d, const int heuristic_id) {
    expectimax_strategy::init(d, heuristics::exports[heuristic_id]);
}

int monte_carlo_player(const board_t board) { return monte_carlo_strategy::player(board); }
void init_monte_carlo_strategy(const int d) { monte_carlo_strategy::init(d); }

board_t test(const board_t x) {
    return x + 1;
}

}

