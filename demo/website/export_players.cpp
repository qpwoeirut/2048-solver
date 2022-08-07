#include <emscripten/bind.h>
#include "../../game.hpp"
#include "../../heuristics.hpp"
#include "../../machine_learning/ExportedTD0.hpp"
#include "../../strategies/ExpectimaxDepthStrategy.hpp"
#include "../../strategies/ExpectimaxProbabilityStrategy.hpp"
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
    class_<ExpectimaxDepthStrategy>("ExpectimaxDepthStrategy")
        .constructor<const int, const int>()
        .function("pick_move", &ExpectimaxDepthStrategy::pick_move);

    class_<ExpectimaxProbabilityStrategy>("ExpectimaxProbabilityStrategy")
        .constructor<const float, const int>()
        .function("pick_move", &ExpectimaxProbabilityStrategy::pick_move);

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

    class_<ExportedTD0>("ExportedTD0")
        .constructor<>()
        .function("pick_move", &ExportedTD0::pick_move);

    function("score_heuristic", &heuristics::score_heuristic);
    function("merge_heuristic", &heuristics::merge_heuristic);
    function("corner_heuristic", &heuristics::corner_heuristic);
    function("wall_gap_heuristic", &heuristics::wall_gap_heuristic);
    function("full_wall_heuristic", &heuristics::full_wall_heuristic);
    function("strict_wall_heuristic", &heuristics::strict_wall_heuristic);
    function("skewed_corner_heuristic", &heuristics::skewed_corner_heuristic);
    function("monotonicity_heuristic", &heuristics::monotonicity_heuristic);
}

