#include <emscripten/bind.h>
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

using emscripten::class_;

EMSCRIPTEN_BINDINGS(players) {
    class_<ExpectimaxStrategy>("ExpectimaxStrategy")
        .constructor<const int, const int>()
        .function("pick_move", &ExpectimaxStrategy::pick_move);

    class_<MinimaxStrategy>("MinimaxStrategy")
        .constructor<const int, const int>()
        .function("pick_move", &MinimaxStrategy::pick_move);

    class_<RandomTrialsStrategy>("RandomTrialsStrategy")
        .constructor<const int, const int, const int>()
        .function("pick_move", &RandomTrialsStrategy::pick_move);

    class_<MonteCarloPlayer>("MonteCarloPlayer")
        .constructor<const int>()
        .function("pick_move", &MonteCarloPlayer::pick_move);

    class_<OrderedPlayer>("OrderedPlayer")
        .constructor<>()
        .function("pick_move", &OrderedPlayer::pick_move);

    class_<RandomPlayer>("RandomPlayer")
        .constructor<>()
        .function("pick_move", &RandomPlayer::pick_move);

    class_<RotatingPlayer>("RotatingPlayer")
        .constructor<>()
        .function("pick_move", &RotatingPlayer::pick_move);

    class_<SpamCornerPlayer>("SpamCornerPlayer")
        .constructor<>()
        .function("pick_move", &SpamCornerPlayer::pick_move);
}

