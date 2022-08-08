#ifndef MINIMAX_STRATEGY_HPP
#define MINIMAX_STRATEGY_HPP

#include "Strategy.hpp"

class MinimaxStrategy : public Strategy {
    /*
        Parameters:
            depth: depth to search, should be positive; note that search space increases exponentially with depth
                   a nonpositive depth argument d will be subtracted from the depth picker's result (increasing the depth)
    */

    heuristic_t evaluator;

public:
    int depth = 0;

    MinimaxStrategy(const int _depth, const heuristic_t _evaluator) {
        depth = _depth;
        evaluator = _evaluator;
    }

    MinimaxStrategy(const int _depth, const int heuristic_idx) :
            MinimaxStrategy(_depth, heuristics::exports[heuristic_idx]) {}

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<MinimaxStrategy>(depth, evaluator);
    }

    const int pick_move(const board_t board) override {
        const int depth_to_use = depth <= 0 ? pick_depth(board) - depth : depth;
        const int move = helper(board, depth_to_use, heuristics::MIN_EVAL, heuristics::MAX_EVAL, 0) & 3;
        return move;
    }

private:
    const eval_t helper(const board_t board, const int cur_depth, eval_t alpha, const eval_t beta0, const int fours) {
        if (simulator.game_over(board)) {
            const eval_t score = evaluator(board);
            return score - (score >> 4);  // subtract score / 16 as penalty for dying
        }
        if (cur_depth == 0 || fours >= 5) { // selecting 5 fours has a 0.001% chance, which is negligible
            return evaluator(board) << 2;  // move doesn't matter
        }

        eval_t best_score = heuristics::MIN_EVAL;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i = 0; i < 4; ++i) {
            eval_t current_score = heuristics::MAX_EVAL;  // next step will minimize this across all tile placements
            const board_t new_board = simulator.make_move(board, i);
            if (board == new_board) {
                continue;
            } else {
                const uint16_t tile_mask = to_tile_mask(new_board);
                eval_t beta = beta0;
                for (int j = 0; j < 16; ++j) {
                    if (((tile_mask >> j) & 1) == 0) {
                        current_score = std::min(current_score,
                                                 helper(new_board | (1ULL << (j << 2)), cur_depth - 1, alpha, beta, fours) >> 2);
                        current_score = std::min(current_score,
                                                 helper(new_board | (2ULL << (j << 2)), cur_depth - 1, alpha, beta, (fours + 1)) >> 2);

                        beta = std::min(beta, current_score);
                        if (current_score < alpha) break;
                    }
                }
            }
            if (best_score <= current_score) {
                best_score = current_score;
                best_move = i;

                alpha = std::max(alpha, best_score);
                if (best_score > beta0) break;
            }
        }

        return (best_score << 2) | best_move;  // pack both score and move
    }

    const int pick_depth(const board_t board) {
        const int tile_ct = count_set(to_tile_mask(board));
        const int score = count_distinct_tiles(board) + (tile_ct <= 6 ? 0 : (tile_ct - 6) >> 1);
        return 2 + (score > 6) + (score > 9) + (score > 11) + (score > 14) + (score > 16);
    }
};

#endif
