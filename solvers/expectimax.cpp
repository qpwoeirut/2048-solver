namespace expectimax_player {
    /*
        Parameters:
            depth: depth to search, should be positive; note that search space increases exponentially with depth
    */

    int depth = 3;
    const long long MULT = 1e9;  // speed things up with integer arithmetic

    const long long helper(const board_t board, const int cur_depth) {
        if (cur_depth == 0) {
            return (MULT * approximate_score(board)) << 2;  // move doesn't matter
        }

        long long best_score = 0;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            const board_t new_board = game::make_move(board, i);
            if (board == new_board || game::game_over(new_board)) continue;

            long long expected_score = 0;  // expected score times (moves * 10)
            const uint16_t empty_mask = game::to_tile_mask(new_board);
            for (int j=0; j<16; ++j) {
                if (((empty_mask >> j) & 1) == 0) {
                    expected_score += 9 * (helper(new_board | (1 << j), cur_depth - 1) >> 2);
                    expected_score += 1 * (helper(new_board | (2 << j), cur_depth - 1) >> 2);
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
        return helper(board, depth) & 3;
    }

    void init(const int _depth) {
        depth = _depth;
    }
}

