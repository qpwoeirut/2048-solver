#define tile_val(r, c) (((board >> (((r << 2) | c) << 2)) & 0xF) == 0 ? 0 : 1 << ((board >> (((r << 2) + c) << 2)) & 0xF))

namespace heuristics {
    // assumes that only 2's have spawned, which is a good enough approximation
    // creating a tile of 2^n adds 2^n to the score, and requires two 2^(n-1) tiles
    // creating each of those added 2^(n-1) to the score, and following the recursive pattern gets n * 2^n
    // technically we want (n-1) * 2^n since the 2's spawning don't add to the score
    int score_heuristic(const board_t board) {
        int score = 0;
        for (int i=0; i<64; i+=4) {
            const uint8_t tile = (board >> i) & 0xF;
            score += tile <= 1 ? 0 : (tile - 1) * (1 << tile);
        }
        return score;
    }

    int merge_heuristic(const board_t board) {  // count empty tiles on board
        return count_empty(game::to_tile_mask(board));
    }

    int _weight_heuristic(const board_t board, const int weights[]) {
        int a = 0, b = 0, c = 0, d = 0;
        for (int row = 0; row < 64; row += 16) {
            for (int col = 0; col < 16; col += 4) {
                const int tile = ((board >> (row | col)) & 0xF) == 0 ? 0 : 1 << ((board >> (row | col)) & 0xF);
                a += tile * weights[(row | col) >> 2];
                b += tile * weights[((64 - row) | (16 - col)) >> 2];
                c += tile * weights[(col << 2) | (row >> 2)];
                d += tile * weights[((16 - col) << 2 | (64 - row) >> 2)];
            }
        }
        return std::max(std::max(a, b), std::max(c, d));
    }

    // gives a score based on how the tiles are arranged in the corner, returns max over all 4 corners
    // higher value tiles should be closer to the corner
    // multipliers for now
    // 10 5 2 1
    // 5 3 1 0
    // 2 1 0 0
    // 1 0 0 0
    int corner_heuristic(const board_t board) {
        const int lower_left =  10 * tile_val(0, 3) + 5 * tile_val(0, 2) + 2 * tile_val(0, 1) + 1 * tile_val(0, 0) +
                                5  * tile_val(1, 3) + 3 * tile_val(1, 2) + 1 * tile_val(1, 1) +
                                2  * tile_val(2, 3) + 1 * tile_val(2, 2) +
                                1  * tile_val(3, 3);

        const int upper_left =  10 * tile_val(3, 3) + 5 * tile_val(3, 2) + 2 * tile_val(3, 1) + 1 * tile_val(3, 0) +
                                5  * tile_val(2, 3) + 3 * tile_val(2, 2) + 1 * tile_val(2, 1) +
                                2  * tile_val(1, 3) + 1 * tile_val(1, 2) +
                                1  * tile_val(0, 3);


        const int lower_right = 10 * tile_val(0, 0) + 5 * tile_val(0, 1) + 2 * tile_val(0, 2) + 1 * tile_val(0, 3) +
                                5  * tile_val(1, 0) + 3 * tile_val(1, 1) + 1 * tile_val(1, 2) +
                                2  * tile_val(2, 0) + 1 * tile_val(2, 1) +
                                1  * tile_val(3, 0);

        const int upper_right = 10 * tile_val(3, 0) + 5 * tile_val(3, 1) + 2 * tile_val(3, 2) + 1 * tile_val(3, 3) +
                                5  * tile_val(2, 0) + 3 * tile_val(2, 1) + 1 * tile_val(2, 2) +
                                2  * tile_val(1, 0) + 1 * tile_val(1, 1) +
                                1  * tile_val(0, 0);

        return std::max(std::max(lower_left, upper_left), std::max(lower_right, upper_right));
    }

    int wall_heuristic(const board_t board) {
        const int top    = 128 * tile_val(3, 3) + 64 * tile_val(3, 2) + 32 * tile_val(3, 1) + 16 * tile_val(3, 0) +
                           1   * tile_val(2, 3) + 2  * tile_val(2, 2) + 4  * tile_val(2, 1) + 8  * tile_val(2, 0);

        const int bottom = 128 * tile_val(0, 3) + 64 * tile_val(0, 2) + 32 * tile_val(0, 1) + 16 * tile_val(0, 0) +
                           1   * tile_val(1, 3) + 2  * tile_val(1, 2) + 4  * tile_val(1, 1) + 8  * tile_val(0, 0);

        const int left   = 128 * tile_val(3, 3) + 64 * tile_val(2, 3) + 32 * tile_val(1, 3) + 16 * tile_val(0, 3) +
                           1   * tile_val(2, 3) + 2  * tile_val(2, 2) + 4  * tile_val(2, 1) + 8  * tile_val(2, 0);

        const int right  = 128 * tile_val(3, 3) + 64 * tile_val(3, 2) + 32 * tile_val(3, 1) + 16 * tile_val(3, 0) +
                           1   * tile_val(2, 3) + 2  * tile_val(2, 2) + 4  * tile_val(2, 1) + 8  * tile_val(2, 0);
        assert(0);
        return 0;
    }
}

