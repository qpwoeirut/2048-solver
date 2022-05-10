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
void init_rand_trials_strategy_with_merge_heuristic(const int d, const int t) { rand_trials_strategy::init(d, t, heuristics::merge_heuristic); }
void init_rand_trials_strategy_with_score_heuristic(const int d, const int t) { rand_trials_strategy::init(d, t, heuristics::score_heuristic); }
void init_rand_trials_strategy_with_corner_heuristic(const int d, const int t) { rand_trials_strategy::init(d, t, heuristics::corner_heuristic); }

void init_minimax_strategy_with_merge_heuristic(const int d) { minimax_strategy::init(d, heuristics::merge_heuristic); }
void init_minimax_strategy_with_score_heuristic(const int d) { minimax_strategy::init(d, heuristics::score_heuristic); }
void init_minimax_strategy_with_corner_heuristic(const int d) { minimax_strategy::init(d, heuristics::corner_heuristic); }

void init_expectimax_strategy_with_merge_heuristic(const int d) { expectimax_strategy::init(d, heuristics::merge_heuristic); }
void init_expectimax_strategy_with_score_heuristic(const int d) { expectimax_strategy::init(d, heuristics::score_heuristic); }
void init_expectimax_strategy_with_corner_heuristic(const int d) { expectimax_strategy::init(d, heuristics::corner_heuristic); }


PYBIND11_MODULE(players, m) {
    m.doc() = "Solving strategies for 2048 written in C++ and exported to Python";

    m.def("init_game", &game::init, "initialize game");

    m.def("random_strategy", &random_strategy::player, "random strategy");
    m.def("spam_corner_strategy", &spam_corner_strategy::player, "corner spam strategy");
    m.def("ordered_strategy", &ordered_strategy::player, "ordered strategy");
    m.def("rotating_strategy", &rotating_strategy::player, "rotating strategy");

    m.def("rand_trials_strategy", &rand_trials_strategy::player, "random trials strategy");
    m.def("minimax_strategy", &minimax_strategy::player, "minimax strategy");
    m.def("expectimax_strategy", &expectimax_strategy::player, "expectimax strategy");

    m.def("init_rand_trials_strategy_with_merge_heuristic", &init_rand_trials_strategy_with_merge_heuristic,
          "initialize rand_trials strategy with merge heuristic");
    m.def("init_rand_trials_strategy_with_score_heuristic", &init_rand_trials_strategy_with_score_heuristic,
          "initialize rand_trials strategy with score heuristic");
    m.def("init_rand_trials_strategy_with_corner_heuristic", &init_rand_trials_strategy_with_corner_heuristic,
          "initialize rand_trials strategy with corner heuristic");

    m.def("init_minimax_strategy_with_merge_heuristic", &init_minimax_strategy_with_merge_heuristic,
          "initialize minimax strategy with merge heuristic");
    m.def("init_minimax_strategy_with_score_heuristic", &init_minimax_strategy_with_score_heuristic,
          "initialize minimax strategy with score heuristic");
    m.def("init_minimax_strategy_with_corner_heuristic", &init_minimax_strategy_with_corner_heuristic,
          "initialize minimax strategy with corner heuristic");

    m.def("init_expectimax_strategy_with_merge_heuristic", &init_expectimax_strategy_with_merge_heuristic,
          "initialize expectimax strategy with merge heuristic");
    m.def("init_expectimax_strategy_with_score_heuristic", &init_expectimax_strategy_with_score_heuristic,
          "initialize expectimax strategy with score heuristic");
    m.def("init_expectimax_strategy_with_corner_heuristic", &init_expectimax_strategy_with_corner_heuristic,
          "initialize expectimax strategy with corner heuristic");

    m.def("monte_carlo_strategy", &monte_carlo_strategy::player, "monte carlo tree search strategy");
    m.def("init_monte_carlo_strategy", &monte_carlo_strategy::init, "initialize monte carlo tree search strategy");
}

