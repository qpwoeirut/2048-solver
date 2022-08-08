#ifndef EXPECTIMAX_DEPTH_STRATEGY_HPP
#define EXPECTIMAX_DEPTH_STRATEGY_HPP

#include "ExpectimaxStrategy.hpp"

class ExpectimaxDepthStrategy : public ExpectimaxStrategy {
    static constexpr int CACHE_DEPTH = 2;

public:
    const int depth;  // note that depth increases runtime exponentially; non-positive depth uses depth picker
    ExpectimaxDepthStrategy(const int _depth, const heuristic_t _evaluator) :
            ExpectimaxStrategy(_evaluator), depth(_depth) {
        init_cache();
    }

    ExpectimaxDepthStrategy(const int _depth, const int heuristic_idx) :
            ExpectimaxDepthStrategy(_depth, heuristics::exports[heuristic_idx]) {}

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<ExpectimaxDepthStrategy>(depth, evaluator);
    }

    const int pick_move(const board_t board) override {
        const int depth_to_use = depth <= 0 ? pick_depth(board) - depth : depth;

        const int move = helper(board, depth_to_use, 0) & 3;
        update_cache_pointers();
        return move;
    }

private:
    const eval_t helper(const board_t board, const int cur_depth, const int fours) {
        if (simulator.game_over(board)) {
            const eval_t score = MULT * evaluator(board);
            return (score - (score >> 2)) << 2;  // subtract score / 4 as penalty for dying, then pack
        }
        if (cur_depth == 0 || fours >= 4) {  // selecting 4 fours has a 0.01% chance, which is negligible
            return (MULT * evaluator(board)) << 2;  // move doesn't matter
        }

        if (cur_depth >= CACHE_DEPTH) {
            const cache_t::iterator it = cache.find(board);
#ifdef REQUIRE_DETERMINISTIC
            if (it != cache.end() && (it->second & 0xF) == cur_depth) return it->second >> 4;
#else
            if (it != cache.end() && (it->second & 0xF) >= cur_depth) return it->second >> 4;
#endif
        }

        eval_t best_score = heuristics::MIN_EVAL;
        int best_move = -1;
        for (int i = 0; i < 4; ++i) {
            eval_t expected_score = 0;
            const board_t new_board = simulator.make_move(board, i);
            if (board == new_board) {
                continue;
            } else {
                const uint16_t empty_mask = to_tile_mask(new_board);
                for (int j = 0; j < 16; ++j) {
                    if (((empty_mask >> j) & 1) == 0) {
                        expected_score += 9 * (helper(new_board | (1LL << (j << 2)), cur_depth - 1, fours) >> 2);
                        expected_score += 1 * (helper(new_board | (2LL << (j << 2)), cur_depth - 1, fours + 1) >> 2);
                    }
                }
                expected_score /= count_empty(empty_mask) * 10;  // convert to actual expected score * MULT
            }

            if (best_score <= expected_score) {
                best_score = expected_score;
                best_move = i;
            }
        }

        if (cur_depth >= CACHE_DEPTH) {
            add_to_cache(board, best_score, best_move, cur_depth);
        }

        return (best_score << 2) | best_move;  // pack both score and move
    }

    const int pick_depth(const board_t board) {
        const int tile_ct = count_set(to_tile_mask(board));
        const int score = count_distinct_tiles(board) + (tile_ct <= 6 ? 0 : (tile_ct - 6) >> 1);
        return 2 + (score >= 8) + (score >= 11) + (score >= 14) + (score >= 15) + (score >= 17) + (score >= 19);
    }
};

#endif
