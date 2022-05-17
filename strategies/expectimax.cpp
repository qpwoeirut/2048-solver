namespace expectimax_strategy {
    /*
        Parameters:
            depth: depth to search; note that search space increases exponentially with depth
                   a nonpositive depth argument d will be subtracted from the depth picker's result (increasing the depth)
    */

    int depth = 3;
    heuristic_t evaluator = heuristics::dummy_heuristic;

    // speed things up with integer arithmetic
    // expected score * 10, 4 moves, 30 tile placements, multiplied by 4 to pack score and move
    constexpr eval_t MULT = 1e18 / (heuristics::MAX_EVAL * 10 * 4 * 30 * 4);

    constexpr int CACHE_DEPTH = 2;
    cache_t cache;
    int cache_hits = 0, cache_misses = 0;

    const eval_t helper(const board_t board, const int cur_depth, const int fours) {
        if (cur_depth == 0 || fours >= 5) {  // selecting 5 fours has a 0.001% chance, which is negligible
            return (MULT * evaluator(board)) << 2;  // move doesn't matter
        }

        if (cur_depth >= CACHE_DEPTH) {
            const cache_t::iterator it = cache.find(board);
            if (it != cache.end() && (it->second & 3) >= cur_depth) {
                ++cache_hits;
                return it->second;
            }
            ++cache_misses;
        }

        eval_t best_score = heuristics::MIN_EVAL;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            eval_t expected_score = 0;
            const board_t new_board = game::make_move(board, i);
            if (board == new_board) {
                continue;
            } else if (game::game_over(new_board)) {
                expected_score = (MULT * evaluator(board));
                expected_score -= expected_score >> 4;  // subtract score / 16 as penalty for dying
            } else {
                const uint16_t empty_mask = to_tile_mask(new_board);
                for (int j=0; j<16; ++j) {
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
            cache[board] = (best_score << 2) | best_move;
        }
        return (best_score << 2) | best_move;  // pack both score and move
    }
    const int player(const board_t board) {
        cache_hits = 0;
        cache_misses = 0;
        const int move = helper(board, depth <= 0 ? pick_depth(board) - depth : depth, 0) & 3;
        std::cout << (depth <= 0 ? pick_depth(board) - depth : depth) << ' ' << cache_hits << ' ' << cache_misses << std::endl;
        return move;
    }

    void init(const int _depth, heuristic_t _evaluator) {
        depth = _depth;
        evaluator = _evaluator;

        cache = cache_t();
        cache.set_empty_key(game::INVALID_BOARD);
    }
}

