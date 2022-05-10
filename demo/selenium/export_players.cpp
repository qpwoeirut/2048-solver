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
// so instead a workaround with an array of the heuristics is used
void init_rand_trials_strategy(const int d, const int t, const int heuristic_id) {
    rand_trials_strategy::init(d, t, heuristics::exports[heuristic_id]);
}
void init_minimax_strategy(const int d, const int heuristic_id) {
    minimax_strategy::init(d, heuristics::exports[heuristic_id]);
}
void init_expectimax_strategy(const int d, const int heuristic_id) {
    expectimax_strategy::init(d, heuristics::exports[heuristic_id]);
}


PYBIND11_MODULE(players, m) {
    m.doc() = "Solving strategies for 2048 written in C++ and exported to Python";

    m.def("init_game",                  &game::init,                    "initialize game");

    m.def("random_strategy",            &random_strategy::player,       "random strategy");
    m.def("spam_corner_strategy",       &spam_corner_strategy::player,  "corner spam strategy");
    m.def("ordered_strategy",           &ordered_strategy::player,      "ordered strategy");
    m.def("rotating_strategy",          &rotating_strategy::player,     "rotating strategy");

    m.def("rand_trials_strategy",       &rand_trials_strategy::player,  "random trials strategy");
    m.def("minimax_strategy",           &minimax_strategy::player,      "minimax strategy");
    m.def("expectimax_strategy",        &expectimax_strategy::player,   "expectimax strategy");

    m.def("init_rand_trials_strategy",  &init_rand_trials_strategy,     "initialize random trials strategy");
    m.def("init_minimax_strategy",      &init_minimax_strategy,         "initialize minimax strategy");
    m.def("init_expectimax_strategy",   &init_expectimax_strategy,      "initialize expectimax strategy");

    m.def("monte_carlo_strategy",       &monte_carlo_strategy::player,  "monte carlo tree search strategy");
    m.def("init_monte_carlo_strategy",  &monte_carlo_strategy::init,    "initialize monte carlo tree search strategy");
}

