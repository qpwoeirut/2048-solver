#ifndef RANDOM_TRIALS_STRATEGY_HPP
#define RANDOM_TRIALS_STRATEGY_HPP

#include "Strategy.hpp"

class RandomTrialsStrategy: public Strategy {
    /*
        Parameters:
            depth: depth to search, should be positive; note that search space increases exponentially with depth
            trials: trials for each move checked
    */

    // speed things up with integer arithmetic
    // 4 moves, 20 max depth, multiplied by 4 to pack score and move
    static constexpr eval_t MULT = 4e18 / (heuristics::MAX_EVAL * 4 * 20 * 4);
    static_assert(MULT > 1);

    heuristic_t evaluator = heuristics::dummy_heuristic;

    public:
    int depth, trials;
    void init(const int _depth, const int _trials, heuristic_t _evaluator) {
        depth = _depth;
        trials = _trials;
        evaluator = _evaluator;
    }

    const int player(const board_t board) {
        return helper(board, depth) & 3;
    }

    private:
    const eval_t helper(const board_t board, const int cur_depth) {
        if (game::game_over(board)) {
            const eval_t score = (evaluator(board) * MULT) << 2;
            return score - (score >> 4);
        }
        if (cur_depth == 0) {
            return (evaluator(board) * MULT) << 2;  // move doesn't matter
        }

        eval_t best_score = 0;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board) continue;

            eval_t current_score = 0;
            for (int j=0; j<trials; ++j) {
                current_score += helper(game::add_random_tile(new_board, game::generate_random_tile_val()),
                                        cur_depth - 1) >> 2;  // extract score
            }
            if (best_score <= current_score) {
                best_score = current_score;
                best_move = i;
            }
        }
        return ((best_score / trials) << 2) | best_move;  // pack both score and move
    }
};

#endif

