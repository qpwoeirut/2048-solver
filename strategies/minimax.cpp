namespace minimax_strategy {
    /*
        Parameters:
            depth: depth to search, should be positive; note that search space increases exponentially with depth
    */

    int depth = 3;
    int (*evaluator)(const board_t);

    const int helper(const board_t board, const int cur_depth) {
        if (cur_depth == 0) {
            return evaluator(board) << 2;  // move doesn't matter
        }

        int best_score = 0;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board || game::game_over(new_board)) continue;

            int current_score = 1e9;  // next step will minimize this across all tile placements
            const uint16_t tile_mask = game::to_tile_mask(new_board);
            for (int j=0; j<16; ++j) {
                if (((tile_mask >> j) & 1) == 0) {
                    current_score = std::min(current_score, helper(new_board | (1 << (j << 2)), cur_depth - 1) >> 2);
                    current_score = std::min(current_score, helper(new_board | (2 << (j << 2)), cur_depth - 1) >> 2);
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
        return helper(board, depth) & 3;
    }

    void init(const int _depth, int (*_evaluator)(const board_t)) {
        depth = _depth;
        evaluator = _evaluator;
    }
}

