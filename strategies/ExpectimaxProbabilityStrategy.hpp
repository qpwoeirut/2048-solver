#ifndef EXPECTIMAX_PROBABILITY_STRATEGY_HPP
#define EXPECTIMAX_PROBABILITY_STRATEGY_HPP

#include "ExpectimaxStrategy.hpp"

class ExpectimaxProbabilityStrategy : public ExpectimaxStrategy {
public:
    const float min_probability;  // minimum probability a searched state should have
    ExpectimaxProbabilityStrategy(const float min_prob, const heuristic_t _evaluator) :
            ExpectimaxStrategy(_evaluator), min_probability(min_prob) {
        init_cache();
    }

    ExpectimaxProbabilityStrategy(const float min_prob, const int heuristic_idx) :
            ExpectimaxProbabilityStrategy(min_prob, heuristics::exports[heuristic_idx]) {}

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<ExpectimaxProbabilityStrategy>(min_probability, evaluator);
    }

    const int pick_move(const board_t board) override {
        const int move = helper(board, 1.0f, MAX_DEPTH) & 3;
        update_cache_pointers();
        return move;
    }

private:
    const eval_t helper(const board_t board, const float cur_prob, const int cur_depth) {  // depth only used for cache
        if (simulator.game_over(board)) {
            const eval_t score = MULT * evaluator(board);
            return (score - (score >> 2)) << 2;  // subtract score / 4 as penalty for dying, then pack
        }
        if (cur_prob <= min_probability || cur_depth == 0) {
            return (MULT * evaluator(board)) << 2;  // move doesn't matter
        }

        if (cur_prob > min_probability * 8) {
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
                const int empty_count = count_empty(empty_mask);
                const float two_prob = cur_prob * 0.9 / empty_count;
                const float four_prob = cur_prob * 0.1 / empty_count;
                for (int j = 0; j < 16; ++j) {
                    if (((empty_mask >> j) & 1) == 0) {
                        expected_score += 9 * (helper(new_board | (1LL << (j << 2)), two_prob, cur_depth - 1) >> 2);
                        expected_score += 1 * (helper(new_board | (2LL << (j << 2)), four_prob, cur_depth - 1) >> 2);
                    }
                }
                expected_score /= empty_count * 10;  // convert to actual expected score * MULT
            }

            if (best_score <= expected_score) {
                best_score = expected_score;
                best_move = i;
            }
        }

        if (cur_prob >= min_probability * 8) {
            add_to_cache(board, best_score, best_move, cur_depth);
        }

        return (best_score << 2) | best_move;  // pack both score and move
    }
};

#endif
