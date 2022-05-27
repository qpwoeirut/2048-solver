#include <pybind11/pybind11.h>
#include "../../game.hpp"
#include "../../heuristics.hpp"
#include "../../strategies/ExpectimaxStrategy.hpp"
#include "../../strategies/MinimaxStrategy.hpp"
#include "../../strategies/MonteCarloPlayer.hpp"
#include "../../strategies/OrderedPlayer.hpp"
#include "../../strategies/RandomPlayer.hpp"
#include "../../strategies/RandomTrialsStrategy.hpp"
#include "../../strategies/RotatingPlayer.hpp"
#include "../../strategies/SpamCornerPlayer.hpp"
#include "../../strategies/UserPlayer.hpp"

using pybind11::class_;
using pybind11::init;

PYBIND11_MODULE(players, m) {
    m.doc() = "Solving strategies for 2048 written in C++ and exported to Python";

    class_<ExpectimaxStrategy>(m, "ExpectimaxStrategy")
        .def(init<const int, const int>())
        .def("pick_move", &ExpectimaxStrategy::pick_move);

    class_<MinimaxStrategy>(m, "MinimaxStrategy")
        .def(init<const int, const int>())
        .def("pick_move", &MinimaxStrategy::pick_move);

    class_<RandomTrialsStrategy>(m, "RandomTrialsStrategy")
        .def(init<const int, const int, const int>())
        .def("pick_move", &RandomTrialsStrategy::pick_move);

    class_<MonteCarloPlayer>(m, "MonteCarloPlayer")
        .def(init<const int>())
        .def("pick_move", &MonteCarloPlayer::pick_move);

    class_<OrderedPlayer>(m, "OrderedPlayer")
        .def("pick_move", &OrderedPlayer::pick_move);

    class_<RandomPlayer>(m, "RandomPlayer")
        .def("pick_move", &RandomPlayer::pick_move);

    class_<RotatingPlayer>(m, "RotatingPlayer")
        .def("pick_move", &RotatingPlayer::pick_move);

    class_<SpamCornerPlayer>(m, "SpamCornerPlayer")
        .def("pick_move", &SpamCornerPlayer::pick_move);
}

