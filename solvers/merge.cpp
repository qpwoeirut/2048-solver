namespace merge_player {
    // depth to search, should be positive
    // search space increases exponentially with depth
    int depth = 4;  // default depth, can be changed externally

    // trials per depth
    int trials = 3;

    std::pair<float,int> helper(const board_t board, const int cur_depth) {
        if (cur_depth == 0) {
            return std::make_pair(count_empty(game::to_tile_mask(board)), -1);
        }

        float best_score = 100 * trials;
        int best_move = -1;
        for (int i=0; i<4; ++i) {
            float current_score = 0;
            const board_t new_board = game::make_move(board, i);
            if (board == new_board || game::game_over(board)) continue;
            for (int j=0; j<trials; ++j) {
                current_score += helper(game::add_random_tile(new_board), cur_depth - 1).first;
            }
            if (best_score >= current_score) {
                best_score = current_score;
                best_move = i;
            }
        }
        return std::make_pair(best_score / trials, best_move);
    }

    int player(const board_t board) {
        return helper(board, depth).second;
    }
}

