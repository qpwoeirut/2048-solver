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

    m.def("random_player", &random_player::player, "testing");
    m.def("corner_player", &corner_player::player, "testing");
    m.def("ordered_player", &ordered_player::player, "testing");
    m.def("merge_player", &merge_player::player, "testing");
    m.def("score_player", &score_player::player, "testing");
    m.def("monte_carlo_player", &monte_carlo_player::player, "testing");
}
