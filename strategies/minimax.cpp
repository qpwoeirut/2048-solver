namespace minimax_strategy {
    /*
        Parameters:
            depth: depth to search, should be positive; note that search space increases exponentially with depth
                   a nonpositive depth argument d will be subtracted from the depth picker's result (increasing the depth)
    */

    int depth = 3;
    heuristic_t evaluator = heuristics::dummy_heuristic;

    const eval_t helper(const board_t board, const int cur_depth) {
        if (cur_depth == 0) {
            return evaluator(board) << 2;  // move doesn't matter
        }

        eval_t best_score = heuristics::MIN_EVAL;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board || game::game_over(new_board)) continue;

            eval_t current_score = heuristics::MAX_EVAL;  // next step will minimize this across all tile placements
            const uint16_t tile_mask = to_tile_mask(new_board);
            for (int j=0; j<16; ++j) {
                if (((tile_mask >> j) & 1) == 0) {
                    current_score = std::min(current_score, helper(new_board | (1ULL << (j << 2)), cur_depth - 1) >> 2);
                    current_score = std::min(current_score, helper(new_board | (2ULL << (j << 2)), cur_depth - 1) >> 2);
                }
            }
            if (best_score <= current_score) {
                best_score = current_score;
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

