namespace expectimax_strategy {
    /*
        Parameters:
            depth: depth to search; note that search space increases exponentially with depth
                   a nonpositive depth argument d will be subtracted from the depth picker's result (increasing the depth)
    */

    int depth = 3;
    heuristic_t evaluator = heuristics::dummy_heuristic;

    #ifdef USE_CACHE
    constexpr int CACHE_DEPTH = 2;
    cache_t cache(1 << 20);
    // speed things up with integer arithmetic
    // expected score * 10, 4 moves, 30 tile placements, multiplied by 4 to pack score and move, times 16 to pack cache
    constexpr eval_t MULT = 4e18 / (heuristics::MAX_EVAL * 10 * 4 * 30 * 4 * 16);
    #else
    // speed things up with integer arithmetic
    // expected score * 10, 4 moves, 30 tile placements, multiplied by 4 to pack score and move
    constexpr eval_t MULT = 4e18 / (heuristics::MAX_EVAL * 10 * 4 * 30 * 4);
    #endif
    static_assert(MULT > 1);

    const eval_t helper(const board_t board, const int cur_depth, const bool add_to_cache, const int fours) {
        if (game::game_over(board)) {
            const eval_t score = MULT * evaluator(board);
            return score - (score >> 4);  // subtract score / 16 as penalty for dying
        }
        if (cur_depth == 0 || fours >= 5) {  // selecting 5 fours has a 0.001% chance, which is negligible
            return (MULT * evaluator(board)) << 2;  // move doesn't matter
        }

        #ifdef USE_CACHE
        if (cur_depth >= CACHE_DEPTH) {
            const cache_t::iterator it = cache.find(board);
            #ifdef REQUIRE_DETERMINISTIC
            if (it != cache.end() && (it->second & 0xF) == cur_depth) return it->second >> 4;
            #else
            if (it != cache.end() && (it->second & 0xF) >= cur_depth) return it->second >> 4;
            #endif
        }
        #endif

        eval_t best_score = heuristics::MIN_EVAL;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            eval_t expected_score = 0;
            const board_t new_board = game::make_move(board, i);
            if (board == new_board) {
                continue;
            } else {
                const uint16_t empty_mask = to_tile_mask(new_board);
                for (int j=0; j<16; ++j) {
                    if (((empty_mask >> j) & 1) == 0) {
                        expected_score += 9 * (helper(new_board | (1LL << (j << 2)), cur_depth - 1, add_to_cache, fours) >> 2);
                        expected_score += 1 * (helper(new_board | (2LL << (j << 2)), cur_depth - 1, add_to_cache, fours + 1) >> 2);
                    }
                }
                expected_score /= count_empty(empty_mask) * 10;  // convert to actual expected score * MULT
            }

            if (best_score <= expected_score) {
                best_score = expected_score;
                best_move = i;
            }
        }

        #ifdef USE_CACHE
        if (add_to_cache && cur_depth >= CACHE_DEPTH) {
            cache[board] = (((best_score << 2) | best_move) << 4) | cur_depth;
        }
        #endif

        return (best_score << 2) | best_move;  // pack both score and move
    }
    const int player(const board_t board) {
        const int depth_to_use = depth <= 0 ? pick_depth(board) - depth : depth;
        #ifdef USE_CACHE
        // if depth <= CACHE_DEPTH + 1, caching results isn't worth it
        const bool add_to_cache = depth_to_use > CACHE_DEPTH + 1;
        const int move = helper(board, depth_to_use, add_to_cache, 0) & 3;
        if (!add_to_cache) cache.clear_no_resize();
        #else
        const int move = helper(board, depth_to_use, false, 0) & 3;
        #endif
        return move;
    }

    void init(const int _depth, heuristic_t _evaluator) {
        depth = _depth;
        evaluator = _evaluator;

        #ifdef USE_CACHE
        if (cache.hash_funct().use_empty() == false) {  // empty_key hasn't been set, so let's set everything
            cache.set_empty_key(game::INVALID_BOARD);
            cache.set_deleted_key(game::INVALID_BOARD2);
            cache.min_load_factor(0.0);
        }
        #endif
    }
}


/*
reached 273M without clearing

around 590k size (49M) with clearing by add_to_cache value
Playing 3 games took 258.542 seconds (86.1807 seconds per game)

clearing by add_to_cache, init size 1<<20
memory stayed 65M
Playing 3 games took 266.666 seconds (88.8887 seconds per game)

+ setting min_load_factor to 0
Playing 3 games took 266.075 seconds (88.6917 seconds per game)

change init size 600000
Playing 3 games took 261.784 seconds (87.2613 seconds per game)
*/
