#include <pybind11/pybind11.h>
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


// pybind11 only works with the std::function wrapper, which is slow
// so instead a workaround with hardcoded function pointers is used
void init_rand_trials_player_with_merge_heuristic(const int d, const int t) { rand_trials_player::init(d, t, heuristics::merge_heuristic); }
void init_rand_trials_player_with_score_heuristic(const int d, const int t) { rand_trials_player::init(d, t, heuristics::score_heuristic); }
void init_rand_trials_player_with_corner_heuristic(const int d, const int t) { rand_trials_player::init(d, t, heuristics::corner_heuristic); }

void init_minimax_player_with_merge_heuristic(const int d) { minimax_player::init(d, heuristics::merge_heuristic); }
void init_minimax_player_with_score_heuristic(const int d) { minimax_player::init(d, heuristics::score_heuristic); }
void init_minimax_player_with_corner_heuristic(const int d) { minimax_player::init(d, heuristics::corner_heuristic); }

void init_expectimax_player_with_merge_heuristic(const int d) { expectimax_player::init(d, heuristics::merge_heuristic); }
void init_expectimax_player_with_score_heuristic(const int d) { expectimax_player::init(d, heuristics::score_heuristic); }
void init_expectimax_player_with_corner_heuristic(const int d) { expectimax_player::init(d, heuristics::corner_heuristic); }


PYBIND11_MODULE(players, m) {
    m.doc() = "Solving strategies for 2048 written in C++ and exported to Python";

    m.def("init_game", &game::init, "initialize game");

    m.def("random_player", &random_player::player, "random player");
    m.def("spam_corner_player", &spam_corner_player::player, "corner spam player");
    m.def("ordered_player", &ordered_player::player, "ordered player");
    m.def("rotating_player", &rotating_player::player, "rotating player");

    m.def("rand_trials_player", &rand_trials_player::player, "random trials player");
    m.def("minimax_player", &minimax_player::player, "minimax player");
    m.def("expectimax_player", &expectimax_player::player, "expectimax player");

    m.def("init_rand_trials_player_with_merge_heuristic", &init_rand_trials_player_with_merge_heuristic,
          "initialize rand_trials player with merge heuristic");
    m.def("init_rand_trials_player_with_score_heuristic", &init_rand_trials_player_with_score_heuristic,
          "initialize rand_trials player with score heuristic");
    m.def("init_rand_trials_player_with_corner_heuristic", &init_rand_trials_player_with_corner_heuristic,
          "initialize rand_trials player with corner heuristic");

    m.def("init_minimax_player_with_merge_heuristic", &init_minimax_player_with_merge_heuristic,
          "initialize minimax player with merge heuristic");
    m.def("init_minimax_player_with_score_heuristic", &init_minimax_player_with_score_heuristic,
          "initialize minimax player with score heuristic");
    m.def("init_minimax_player_with_corner_heuristic", &init_minimax_player_with_corner_heuristic,
          "initialize minimax player with corner heuristic");

    m.def("init_expectimax_player_with_merge_heuristic", &init_expectimax_player_with_merge_heuristic,
          "initialize expectimax player with merge heuristic");
    m.def("init_expectimax_player_with_score_heuristic", &init_expectimax_player_with_score_heuristic,
          "initialize expectimax player with score heuristic");
    m.def("init_expectimax_player_with_corner_heuristic", &init_expectimax_player_with_corner_heuristic,
          "initialize expectimax player with corner heuristic");

    m.def("monte_carlo_player", &monte_carlo_player::player, "monte carlo tree search player");
    m.def("init_monte_carlo_player", &monte_carlo_player::init, "initialize monte carlo tree search player");
}

