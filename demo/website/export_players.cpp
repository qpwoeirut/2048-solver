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
using emscripten::function;

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

    class_<TD0>("TD0")  // don't need constructor; will use TD0::load_best instead
        .function("pick_move", &TD0::pick_move)
        .class_property("best_model", &TD0::best_model);

    function("td0_load_best", &TD0::load_best);

    function("score_heuristic", &heuristics::score_heuristic);
    function("merge_heuristic", &heuristics::merge_heuristic);
    function("corner_heuristic", &heuristics::corner_heuristic);
    function("wall_gap_heuristic", &heuristics::wall_gap_heuristic);
    function("full_wall_heuristic", &heuristics::full_wall_heuristic);
    function("strict_wall_heuristic", &heuristics::strict_wall_heuristic);
    function("skewed_corner_heuristic", &heuristics::skewed_corner_heuristic);
    function("monotonicity_heuristic", &heuristics::monotonicity_heuristic);
    function("n_tuple_heuristic", &heuristics::n_tuple_heuristic);
}

