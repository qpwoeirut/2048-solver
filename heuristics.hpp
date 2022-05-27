namespace heuristics {
    // used initialization value only; should be replaced when init strategy function is invoked
    eval_t dummy_heuristic(const board_t board) {
        std::cout << "strategy not initialized properly!" << std::endl;
        assert(false);
    }

    // assumes that only 2's have spawned, which is a good enough approximation
    // creating a tile of 2^n adds 2^n to the score, and requires two 2^(n-1) tiles
    // creating each of those added 2^(n-1) to the score, and following the recursive pattern gets n * 2^n
    // technically we want (n-1) * 2^n since the 2's spawning don't add to the score
    eval_t score_heuristic(const board_t board) {
        eval_t score = 0;
        for (int i=0; i<64; i+=4) {
            const uint8_t tile = (board >> i) & 0xF;
            score += tile <= 1 ? 0 : (tile - 1) * (1 << tile);
        }
        return score;
    }

    eval_t merge_heuristic(const board_t board) {  // count empty tiles on board
        return count_empty(to_tile_mask(board));
    }


    #define tile_exp(r, c) ((board >> (((r << 2) | c) << 2)) & 0xF)
    #define tile_val(r, c) (tile_exp(r, c) == 0 ? 0 : 1 << tile_exp(r, c))
    // be careful! tile_val returns an int but tile_exp returns a board_t

    // gives a score based on how the tiles are arranged in the corner, returns max over all 4 corners
    // higher value tiles should be closer to the corner
    eval_t corner_heuristic(const board_t board) {
        const eval_t lower_left =  10 * tile_val(0, 3) + 5 * tile_val(0, 2) + 2 * tile_val(0, 1) + 1 * tile_val(0, 0) +
                                   5  * tile_val(1, 3) + 3 * tile_val(1, 2) + 1 * tile_val(1, 1) +
                                   2  * tile_val(2, 3) + 1 * tile_val(2, 2) +
                                   1  * tile_val(3, 3);

        const eval_t upper_left =  10 * tile_val(3, 3) + 5 * tile_val(3, 2) + 2 * tile_val(3, 1) + 1 * tile_val(3, 0) +
                                   5  * tile_val(2, 3) + 3 * tile_val(2, 2) + 1 * tile_val(2, 1) +
                                   2  * tile_val(1, 3) + 1 * tile_val(1, 2) +
                                   1  * tile_val(0, 3);


        const eval_t lower_right = 10 * tile_val(0, 0) + 5 * tile_val(0, 1) + 2 * tile_val(0, 2) + 1 * tile_val(0, 3) +
                                   5  * tile_val(1, 0) + 3 * tile_val(1, 1) + 1 * tile_val(1, 2) +
                                   2  * tile_val(2, 0) + 1 * tile_val(2, 1) +
                                   1  * tile_val(3, 0);

        const eval_t upper_right = 10 * tile_val(3, 0) + 5 * tile_val(3, 1) + 2 * tile_val(3, 2) + 1 * tile_val(3, 3) +
                                   5  * tile_val(2, 0) + 3 * tile_val(2, 1) + 1 * tile_val(2, 2) +
                                   2  * tile_val(1, 0) + 1 * tile_val(1, 1) +
                                   1  * tile_val(0, 0);

        return std::max(std::max(lower_left, upper_left), std::max(lower_right, upper_right));
    }

    #undef tile_val


    eval_t _wall_gap_heuristic(const board_t board) {
        const eval_t top    = (tile_exp(3, 3) << 40) | (tile_exp(3, 2) << 36) | (tile_exp(3, 1) << 32) |
                              (tile_exp(2, 3) << 20) | (tile_exp(2, 2) << 24) | (tile_exp(2, 1) << 28) |
                              (tile_exp(1, 3) << 16) | (tile_exp(1, 2) << 12) | (tile_exp(1, 1) << 8);

        const eval_t bottom = (tile_exp(0, 0) << 40) | (tile_exp(0, 1) << 36) | (tile_exp(0, 2) << 32) |
                              (tile_exp(1, 0) << 20) | (tile_exp(1, 1) << 24) | (tile_exp(1, 2) << 28) |
                              (tile_exp(2, 0) << 16) | (tile_exp(2, 1) << 12) | (tile_exp(2, 2) << 8);

        const eval_t left   = (tile_exp(0, 3) << 40) | (tile_exp(1, 3) << 36) | (tile_exp(2, 3) << 32) |
                              (tile_exp(0, 2) << 20) | (tile_exp(1, 2) << 24) | (tile_exp(2, 2) << 28) |
                              (tile_exp(0, 1) << 16) | (tile_exp(1, 1) << 12) | (tile_exp(2, 1) << 8);

        const eval_t right  = (tile_exp(3, 0) << 40) | (tile_exp(2, 0) << 36) | (tile_exp(1, 0) << 32) |
                              (tile_exp(3, 1) << 20) | (tile_exp(2, 1) << 24) | (tile_exp(1, 1) << 28) |
                              (tile_exp(3, 2) << 16) | (tile_exp(2, 2) << 12) | (tile_exp(1, 2) << 8);

        return std::max(std::max(top, bottom), std::max(left, right)) + score_heuristic(board);
    }
    eval_t wall_gap_heuristic(const board_t board) {
        return std::max(_wall_gap_heuristic(board), _wall_gap_heuristic(transpose(board))) + score_heuristic(board);  // tiebreak by score
    }

    eval_t _full_wall_heuristic(const board_t board) {
        const eval_t top    = (tile_exp(3, 3) << 40) | (tile_exp(3, 2) << 36) | (tile_exp(3, 1) << 32) | (tile_exp(3, 0) << 28) |
                              (tile_exp(2, 3) << 12) | (tile_exp(2, 2) << 16) | (tile_exp(2, 1) << 20) | (tile_exp(2, 0) << 24) |
                              (tile_exp(1, 3) << 8);

        const eval_t bottom = (tile_exp(0, 0) << 40) | (tile_exp(0, 1) << 36) | (tile_exp(0, 2) << 32) | (tile_exp(0, 3) << 28) |
                              (tile_exp(1, 0) << 12) | (tile_exp(1, 1) << 16) | (tile_exp(1, 2) << 20) | (tile_exp(0, 3) << 24) |
                              (tile_exp(2, 0) << 8);

        const eval_t left   = (tile_exp(0, 3) << 40) | (tile_exp(1, 3) << 36) | (tile_exp(2, 3) << 32) | (tile_exp(3, 3) << 28) |
                              (tile_exp(0, 2) << 12) | (tile_exp(1, 2) << 16) | (tile_exp(2, 2) << 20) | (tile_exp(3, 2) << 24) |
                              (tile_exp(0, 1) << 8);

        const eval_t right  = (tile_exp(3, 0) << 40) | (tile_exp(2, 0) << 36) | (tile_exp(1, 0) << 32) | (tile_exp(0, 0) << 28) |
                              (tile_exp(3, 1) << 12) | (tile_exp(2, 1) << 16) | (tile_exp(1, 1) << 20) | (tile_exp(0, 1) << 24) |
                              (tile_exp(3, 2) << 8);

        return std::max(std::max(top, bottom), std::max(left, right));
    }
    eval_t full_wall_heuristic(const board_t board) {
        return std::max(_full_wall_heuristic(board), _full_wall_heuristic(transpose(board))) + score_heuristic(board);  // tiebreak by score
    }

    #undef tile_exp

    constexpr eval_t MAX_EVAL = 16ULL << 40;  // from wall heuristics
    constexpr eval_t MIN_EVAL = 0;  // all evaluations are positive

    constexpr heuristic_t exports[5] = {
        score_heuristic, merge_heuristic, corner_heuristic, wall_gap_heuristic, full_wall_heuristic
    };
}
