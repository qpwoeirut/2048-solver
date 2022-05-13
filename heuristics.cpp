#define tile_val(r, c) (((board >> (((r << 2) | c) << 2)) & 0xF) == 0 ? 0 : 1 << ((board >> (((r << 2) + c) << 2)) & 0xF))

using heuristic_t = int (*)(const board_t);

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
        return count_empty(to_tile_mask(board));
    }

    // I tried writing a generalized weight heuristic to combine the corner and wall building heuristics but it was a lot slower

    // gives a score based on how the tiles are arranged in the corner, returns max over all 4 corners
    // higher value tiles should be closer to the corner
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

    constexpr int WALL_GAP[9] = {
        0x7000, 0x6000, 0x4000,
        0x0010, 0x0040, 0x0400,
        0x0004, 0x0002, 0x0001
    };
    int _wall_gap_heuristic(const board_t board) {
        const int top    = WALL_GAP[0] * tile_val(3, 3) + WALL_GAP[1] * tile_val(3, 2) + WALL_GAP[2] * tile_val(3, 1) +
                           WALL_GAP[3] * tile_val(2, 3) + WALL_GAP[4] * tile_val(2, 2) + WALL_GAP[5] * tile_val(2, 1) +
                           WALL_GAP[6] * tile_val(1, 3) + WALL_GAP[7] * tile_val(1, 2) + WALL_GAP[8] * tile_val(1, 1);

        const int bottom = WALL_GAP[0] * tile_val(0, 0) + WALL_GAP[1] * tile_val(0, 1) + WALL_GAP[2] * tile_val(0, 2) +
                           WALL_GAP[3] * tile_val(1, 0) + WALL_GAP[4] * tile_val(1, 1) + WALL_GAP[5] * tile_val(1, 2) +
                           WALL_GAP[6] * tile_val(2, 0) + WALL_GAP[7] * tile_val(2, 1) + WALL_GAP[8] * tile_val(2, 2);

        const int left   = WALL_GAP[0] * tile_val(0, 3) + WALL_GAP[1] * tile_val(1, 3) + WALL_GAP[2] * tile_val(2, 3) +
                           WALL_GAP[3] * tile_val(0, 2) + WALL_GAP[4] * tile_val(1, 2) + WALL_GAP[5] * tile_val(2, 2) +
                           WALL_GAP[6] * tile_val(0, 1) + WALL_GAP[7] * tile_val(1, 1) + WALL_GAP[8] * tile_val(2, 1);

        const int right  = WALL_GAP[0] * tile_val(3, 0) + WALL_GAP[1] * tile_val(2, 0) + WALL_GAP[2] * tile_val(1, 0) +
                           WALL_GAP[3] * tile_val(3, 1) + WALL_GAP[4] * tile_val(2, 1) + WALL_GAP[5] * tile_val(1, 1) +
                           WALL_GAP[6] * tile_val(3, 2) + WALL_GAP[7] * tile_val(2, 2) + WALL_GAP[8] * tile_val(1, 2);
        return std::max(std::max(top, bottom), std::max(left, right));
    }
    int wall_gap_heuristic(const board_t board) {
        return std::max(_wall_gap_heuristic(board), _wall_gap_heuristic(game::transpose(board)));
    }

    constexpr int FULL_WALL[9] = {
        0x7000, 0x6000, 0x4000, 0x2000,
        0x0002, 0x0008, 0x0020, 0x0200,
        0x0001
    };
    int _full_wall_heuristic(const board_t board) {
        const int top    = FULL_WALL[0] * tile_val(3, 3) + FULL_WALL[1] * tile_val(3, 2) + FULL_WALL[2] * tile_val(3, 1) + FULL_WALL[3] * tile_val(3, 0) +
                           FULL_WALL[4] * tile_val(2, 3) + FULL_WALL[5] * tile_val(2, 2) + FULL_WALL[6] * tile_val(2, 1) + FULL_WALL[7] * tile_val(2, 0) +
                           FULL_WALL[8] * tile_val(1, 3);

        const int bottom = FULL_WALL[0] * tile_val(0, 0) + FULL_WALL[1] * tile_val(0, 1) + FULL_WALL[2] * tile_val(0, 2) + FULL_WALL[3] * tile_val(0, 3) +
                           FULL_WALL[4] * tile_val(1, 0) + FULL_WALL[5] * tile_val(1, 1) + FULL_WALL[6] * tile_val(1, 2) + FULL_WALL[7] * tile_val(0, 3) +
                           FULL_WALL[8] * tile_val(2, 0);

        const int left   = FULL_WALL[0] * tile_val(0, 3) + FULL_WALL[1] * tile_val(1, 3) + FULL_WALL[2] * tile_val(2, 3) + FULL_WALL[3] * tile_val(3, 3) +
                           FULL_WALL[4] * tile_val(0, 2) + FULL_WALL[5] * tile_val(1, 2) + FULL_WALL[6] * tile_val(2, 2) + FULL_WALL[7] * tile_val(3, 2) +
                           FULL_WALL[8] * tile_val(0, 1);

        const int right  = FULL_WALL[0] * tile_val(3, 0) + FULL_WALL[1] * tile_val(2, 0) + FULL_WALL[2] * tile_val(1, 0) + FULL_WALL[3] * tile_val(0, 0) +
                           FULL_WALL[4] * tile_val(3, 1) + FULL_WALL[5] * tile_val(2, 1) + FULL_WALL[6] * tile_val(1, 1) + FULL_WALL[7] * tile_val(0, 1) +
                           FULL_WALL[8] * tile_val(3, 2);
        return std::max(std::max(top, bottom), std::max(left, right));
    }
    int full_wall_heuristic(const board_t board) {
        return std::max(_full_wall_heuristic(board), _full_wall_heuristic(game::transpose(board)));
    }

    constexpr heuristic_t exports[5] = {
        score_heuristic, merge_heuristic, corner_heuristic, wall_gap_heuristic, full_wall_heuristic
    };
}

