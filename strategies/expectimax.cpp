namespace expectimax_strategy {
    /*
        Parameters:
            depth: depth to search; note that search space increases exponentially with depth
                   a nonpositive depth argument d will be subtracted from the depth picker's result (increasing the depth)
    */

    int depth = 3;
    heuristic_t evaluator;

    // speed things up with integer arithmetic
    // expected score * 10, 4 moves, 30 tile placements, multiplied by 4 to pack score and move
    constexpr eval_t MULT = 1e18 / (heuristics::MAX_EVAL * 10 * 4 * 30 * 4);

    const eval_t helper(const board_t board, const int cur_depth) {
        if (cur_depth == 0) {
            return (MULT * evaluator(board)) << 2;  // move doesn't matter
        }

        eval_t best_score = heuristics::MIN_EVAL;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board || game::game_over(new_board)) continue;

            eval_t expected_score = 0;  // expected score times (moves * 10)
            const uint16_t empty_mask = to_tile_mask(new_board);
            for (int j=0; j<16; ++j) {
                if (((empty_mask >> j) & 1) == 0) {
                    expected_score += 9 * (helper(new_board | (1LL << (j << 2)), cur_depth - 1) >> 2);
                    expected_score += 1 * (helper(new_board | (2LL << (j << 2)), cur_depth - 1) >> 2);
                }
            }
            expected_score /= count_empty(empty_mask) * 10;  // convert to actual expected score
            if (best_score <= expected_score) {
                best_score = expected_score;
                best_move = i;
            }
        }
        return (best_score << 2) | best_move;  // pack both score and move
    }
    const int player(const board_t board) {
        return helper(board, depth <= 0 ? pick_depth(board) - depth : depth) & 3;
    }

    void init(const int _depth, heuristic_t _evaluator) {
        depth = _depth;
        evaluator = _evaluator;
    }
}

