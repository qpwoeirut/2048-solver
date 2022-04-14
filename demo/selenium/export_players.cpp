#include <pybind11/pybind11.h>
#include "../../game/game.cpp"
#include "../../solvers/random.cpp"
#include "../../solvers/corner.cpp"
#include "../../solvers/ordered.cpp"
#include "../../solvers/merge.cpp"
#include "../../solvers/score.cpp"
#include "../../solvers/monte_carlo.cpp"

PYBIND11_MODULE(players, m) {
    m.doc() = "Solving strategies for 2048 written in C++ and exported to Python";

    m.def("init_game", &game::init, "initialize game");

    m.def("random_player", &random_player::player, "random player");
    m.def("corner_player", &corner_player::player, "corner spam player");
    m.def("ordered_player", &ordered_player::player, "ordered player");

    m.def("merge_player", &merge_player::player, "marge player");
    m.def("init_merge_player", &merge_player::init, "initialize marge player");

    m.def("score_player", &score_player::player, "score player");
    m.def("init_score_player", &score_player::init, "initialize score player");

    m.def("monte_carlo_player", &monte_carlo_player::player, "monte carlo tree search player");
    m.def("init_monte_carlo_player", &monte_carlo_player::init, "initialize monte carlo tree search player");
}
