namespace heuristics {
    constexpr eval_t MAX_EVAL = 16ULL << 40;  // from wall heuristics
    constexpr eval_t MIN_EVAL = 0;  // all evaluations are positive

    eval_t score_heuristic(const board_t board) {
        return approximate_score(board);
    }

    eval_t merge_heuristic(const board_t board) {  // count empty tiles on board
        return count_empty(to_tile_mask(board));
    }

    inline board_t tile_exp(const board_t board, const int pos) {
        return (board >> pos) & 0xF;
    }
    inline board_t tile_exp(const board_t board, const int r, const int c) {
        return (board >> (((r << 2) | c) << 2)) & 0xF;
    }
    inline board_t tile_val(const board_t board, const int pos) {
        return tile_exp(board, pos) == 0 ? 0 : 1 << tile_exp(board, pos);
    }
    inline board_t tile_val(const board_t board, const int r, const int c) {
        return tile_val(board, ((r << 2) | c) << 2);
    }


    eval_t _duplicate_score(const board_t board) {
        eval_t score = 0;
        for (int i = 0; i < 64; i += 4) {
            const int val = tile_val(board, i);
            for (int j = i + 8; j < 64; j += 4) {
                // don't count duplicates that are adjacent
                score += val == tile_val(board, j) && j != i + 16 ? 0 : val;
            }
        }
        return score;
    }

    // gives a score based on how the tiles are arranged in the corner, returns max over all 4 corners
    // higher value tiles should be closer to the corner
    // these weights are mostly arbitrary and could do with some tuning
    eval_t corner_heuristic(const board_t board) {
        const eval_t lower_left =  10 * tile_val(board, 0, 3) + 5 * tile_val(board, 0, 2) + 2 * tile_val(board, 0, 1) + 1 * tile_val(board, 0, 0) +
                                   5  * tile_val(board, 1, 3) + 3 * tile_val(board, 1, 2) + 1 * tile_val(board, 1, 1) +
                                   2  * tile_val(board, 2, 3) + 1 * tile_val(board, 2, 2) +
                                   1  * tile_val(board, 3, 3);

        const eval_t upper_left =  10 * tile_val(board, 3, 3) + 5 * tile_val(board, 3, 2) + 2 * tile_val(board, 3, 1) + 1 * tile_val(board, 3, 0) +
                                   5  * tile_val(board, 2, 3) + 3 * tile_val(board, 2, 2) + 1 * tile_val(board, 2, 1) +
                                   2  * tile_val(board, 1, 3) + 1 * tile_val(board, 1, 2) +
                                   1  * tile_val(board, 0, 3);

        const eval_t lower_right = 10 * tile_val(board, 0, 0) + 5 * tile_val(board, 0, 1) + 2 * tile_val(board, 0, 2) + 1 * tile_val(board, 0, 3) +
                                   5  * tile_val(board, 1, 0) + 3 * tile_val(board, 1, 1) + 1 * tile_val(board, 1, 2) +
                                   2  * tile_val(board, 2, 0) + 1 * tile_val(board, 2, 1) +
                                   1  * tile_val(board, 3, 0);

        const eval_t upper_right = 10 * tile_val(board, 3, 0) + 5 * tile_val(board, 3, 1) + 2 * tile_val(board, 3, 2) + 1 * tile_val(board, 3, 3) +
                                   5  * tile_val(board, 2, 0) + 3 * tile_val(board, 2, 1) + 1 * tile_val(board, 2, 2) +
                                   2  * tile_val(board, 1, 0) + 1 * tile_val(board, 1, 1) +
                                   1  * tile_val(board, 0, 0);

        return std::max(std::max(lower_left, upper_left), std::max(lower_right, upper_right));
    }

    // penalty for having duplicate tiles
    // multiplying by 2 was an arbitrary decision and probably very suboptimal
    eval_t corner_duplicate_heuristic(const board_t board) {
        return 2 * corner_heuristic(board) - _duplicate_score(board);
    }


    eval_t _wall_gap_heuristic(const board_t board) {
        const eval_t top    = (tile_exp(board, 3, 3) << 40) | (tile_exp(board, 3, 2) << 36) | (tile_exp(board, 3, 1) << 32) |
                              (tile_exp(board, 2, 3) << 20) | (tile_exp(board, 2, 2) << 24) | (tile_exp(board, 2, 1) << 28) |
                              (tile_exp(board, 1, 3) << 16) | (tile_exp(board, 1, 2) << 12) | (tile_exp(board, 1, 1) << 8);

        const eval_t bottom = (tile_exp(board, 0, 0) << 40) | (tile_exp(board, 0, 1) << 36) | (tile_exp(board, 0, 2) << 32) |
                              (tile_exp(board, 1, 0) << 20) | (tile_exp(board, 1, 1) << 24) | (tile_exp(board, 1, 2) << 28) |
                              (tile_exp(board, 2, 0) << 16) | (tile_exp(board, 2, 1) << 12) | (tile_exp(board, 2, 2) << 8);

        const eval_t left   = (tile_exp(board, 0, 3) << 40) | (tile_exp(board, 1, 3) << 36) | (tile_exp(board, 2, 3) << 32) |
                              (tile_exp(board, 0, 2) << 20) | (tile_exp(board, 1, 2) << 24) | (tile_exp(board, 2, 2) << 28) |
                              (tile_exp(board, 0, 1) << 16) | (tile_exp(board, 1, 1) << 12) | (tile_exp(board, 2, 1) << 8);

        const eval_t right  = (tile_exp(board, 3, 0) << 40) | (tile_exp(board, 2, 0) << 36) | (tile_exp(board, 1, 0) << 32) |
                              (tile_exp(board, 3, 1) << 20) | (tile_exp(board, 2, 1) << 24) | (tile_exp(board, 1, 1) << 28) |
                              (tile_exp(board, 3, 2) << 16) | (tile_exp(board, 2, 2) << 12) | (tile_exp(board, 1, 2) << 8);

        return std::max(std::max(top, bottom), std::max(left, right)) + score_heuristic(board);
    }
    eval_t wall_gap_heuristic(const board_t board) {
        return std::max(_wall_gap_heuristic(board), _wall_gap_heuristic(transpose(board))) + score_heuristic(board);  // tiebreak by score
    }

    eval_t _full_wall_heuristic(const board_t board) {
        const eval_t top    = (tile_exp(board, 3, 3) << 40) | (tile_exp(board, 3, 2) << 36) | (tile_exp(board, 3, 1) << 32) | (tile_exp(board, 3, 0) << 28) |
                              (tile_exp(board, 2, 3) << 12) | (tile_exp(board, 2, 2) << 16) | (tile_exp(board, 2, 1) << 20) | (tile_exp(board, 2, 0) << 24) |
                              (tile_exp(board, 1, 3) << 8);

        const eval_t bottom = (tile_exp(board, 0, 0) << 40) | (tile_exp(board, 0, 1) << 36) | (tile_exp(board, 0, 2) << 32) | (tile_exp(board, 0, 3) << 28) |
                              (tile_exp(board, 1, 0) << 12) | (tile_exp(board, 1, 1) << 16) | (tile_exp(board, 1, 2) << 20) | (tile_exp(board, 0, 3) << 24) |
                              (tile_exp(board, 2, 0) << 8);

        const eval_t left   = (tile_exp(board, 0, 3) << 40) | (tile_exp(board, 1, 3) << 36) | (tile_exp(board, 2, 3) << 32) | (tile_exp(board, 3, 3) << 28) |
                              (tile_exp(board, 0, 2) << 12) | (tile_exp(board, 1, 2) << 16) | (tile_exp(board, 2, 2) << 20) | (tile_exp(board, 3, 2) << 24) |
                              (tile_exp(board, 0, 1) << 8);

        const eval_t right  = (tile_exp(board, 3, 0) << 40) | (tile_exp(board, 2, 0) << 36) | (tile_exp(board, 1, 0) << 32) | (tile_exp(board, 0, 0) << 28) |
                              (tile_exp(board, 3, 1) << 12) | (tile_exp(board, 2, 1) << 16) | (tile_exp(board, 1, 1) << 20) | (tile_exp(board, 0, 1) << 24) |
                              (tile_exp(board, 3, 2) << 8);

        return std::max(std::max(top, bottom), std::max(left, right));
    }
    eval_t full_wall_heuristic(const board_t board) {
        return std::max(_full_wall_heuristic(board), _full_wall_heuristic(transpose(board))) + score_heuristic(board);  // tiebreak by score
    }


    constexpr heuristic_t exports[6] = {
        score_heuristic, merge_heuristic, corner_heuristic, corner_duplicate_heuristic, wall_gap_heuristic, full_wall_heuristic
    };
}
