#include <chrono>
#include <thread>
#include "util.hpp"


namespace heuristics {
    // all heuristic evaluations must be non-negative
    constexpr eval_t MIN_EVAL = 0;
    constexpr eval_t MAX_EVAL = 16ULL << 41;  // from wall heuristics

    eval_t score_heuristic(const board_t board) {
        return approximate_score(board);
    }

    eval_t merge_heuristic(const board_t board) {
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

        // using initializer list takes about same time as 3 std::max calls
        return std::max({lower_left, upper_left, lower_right, upper_right});
    }

    // compares boards lexicographically, going in a snake across a wall of the board with a gap on the side
    // order (where x doesn't get counted):
    // 0 1 2 x
    // 5 4 3 x
    // 6 7 8 x
    // x x x x
    // takes the maximum over all 4 walls, both transposed and not
    eval_t _wall_gap_heuristic(const board_t board) {
        const eval_t top =
                (tile_exp(board, 3, 3) << 40) | (tile_exp(board, 3, 2) << 36) | (tile_exp(board, 3, 1) << 32) |
                (tile_exp(board, 2, 3) << 20) | (tile_exp(board, 2, 2) << 24) | (tile_exp(board, 2, 1) << 28) |
                (tile_exp(board, 1, 3) << 16) | (tile_exp(board, 1, 2) << 12) | (tile_exp(board, 1, 1) << 8);

        const eval_t bottom =
                (tile_exp(board, 0, 0) << 40) | (tile_exp(board, 0, 1) << 36) | (tile_exp(board, 0, 2) << 32) |
                (tile_exp(board, 1, 0) << 20) | (tile_exp(board, 1, 1) << 24) | (tile_exp(board, 1, 2) << 28) |
                (tile_exp(board, 2, 0) << 16) | (tile_exp(board, 2, 1) << 12) | (tile_exp(board, 2, 2) << 8);

        const eval_t left =
                (tile_exp(board, 0, 3) << 40) | (tile_exp(board, 1, 3) << 36) | (tile_exp(board, 2, 3) << 32) |
                (tile_exp(board, 0, 2) << 20) | (tile_exp(board, 1, 2) << 24) | (tile_exp(board, 2, 2) << 28) |
                (tile_exp(board, 0, 1) << 16) | (tile_exp(board, 1, 1) << 12) | (tile_exp(board, 2, 1) << 8);

        const eval_t right =
                (tile_exp(board, 3, 0) << 40) | (tile_exp(board, 2, 0) << 36) | (tile_exp(board, 1, 0) << 32) |
                (tile_exp(board, 3, 1) << 20) | (tile_exp(board, 2, 1) << 24) | (tile_exp(board, 1, 1) << 28) |
                (tile_exp(board, 3, 2) << 16) | (tile_exp(board, 2, 2) << 12) | (tile_exp(board, 1, 2) << 8);

        return std::max({top, bottom, left, right});
    }

    eval_t wall_gap_heuristic(const board_t board) {
        return std::max(_wall_gap_heuristic(board), _wall_gap_heuristic(transpose(board))) + score_heuristic(board);
        // tiebreak by score
    }

    // compares boards lexicographically, going in a snake across an entire wall of the board
    // order (where x doesn't get counted):
    // 0 1 2 3
    // 7 6 5 4
    // 8 x x x
    // x x x x
    // takes the maximum over all 4 walls, both transposed and not
    eval_t _full_wall_heuristic(const board_t board) {
        const eval_t top =
                (tile_exp(board, 3, 3) << 40) | (tile_exp(board, 3, 2) << 36) | (tile_exp(board, 3, 1) << 32) | (tile_exp(board, 3, 0) << 28) |
                (tile_exp(board, 2, 3) << 12) | (tile_exp(board, 2, 2) << 16) | (tile_exp(board, 2, 1) << 20) | (tile_exp(board, 2, 0) << 24) |
                (tile_exp(board, 1, 3) << 8);

        const eval_t bottom =
                (tile_exp(board, 0, 0) << 40) | (tile_exp(board, 0, 1) << 36) | (tile_exp(board, 0, 2) << 32) | (tile_exp(board, 0, 3) << 28) |
                (tile_exp(board, 1, 0) << 12) | (tile_exp(board, 1, 1) << 16) | (tile_exp(board, 1, 2) << 20) | (tile_exp(board, 0, 3) << 24) |
                (tile_exp(board, 2, 0) << 8);

        const eval_t left =
                (tile_exp(board, 0, 3) << 40) | (tile_exp(board, 1, 3) << 36) | (tile_exp(board, 2, 3) << 32) | (tile_exp(board, 3, 3) << 28) |
                (tile_exp(board, 0, 2) << 12) | (tile_exp(board, 1, 2) << 16) | (tile_exp(board, 2, 2) << 20) | (tile_exp(board, 3, 2) << 24) |
                (tile_exp(board, 0, 1) << 8);

        const eval_t right =
                (tile_exp(board, 3, 0) << 40) | (tile_exp(board, 2, 0) << 36) | (tile_exp(board, 1, 0) << 32) | (tile_exp(board, 0, 0) << 28) |
                (tile_exp(board, 3, 1) << 12) | (tile_exp(board, 2, 1) << 16) | (tile_exp(board, 1, 1) << 20) | (tile_exp(board, 0, 1) << 24) |
                (tile_exp(board, 3, 2) << 8);

        return std::max({top, bottom, left, right});
    }

    eval_t full_wall_heuristic(const board_t board) {
        return std::max(_full_wall_heuristic(board), _full_wall_heuristic(transpose(board))) + score_heuristic(board);
        // tiebreak by score
    }

    inline eval_t _val_cmp(const int a, const int b) {
        return (1 << a) * ((a <= b) ? 1 : -1);
    }

    // same as full_wall_heuristic, but with a penalty for tiles that are inverted in the order
    eval_t _strict_wall_heuristic(const board_t board, const eval_t max_tile) {
        if ((board & 0xF) < max_tile)
            // fixing the position will be easier if the board has more space
            return count_empty(to_tile_mask(board));

        // 60 56 52 48
        // 44 40 36 32
        // 28 24 20 16
        // 12  8  4  0
        constexpr int idxs[8] = {0, 4, 8, 12, 28, 24, 20, 16};

        eval_t mx = std::max({
            (board >> 32) & 0xF, (board >> 36) & 0xF, (board >> 40) & 0xF, (board >> 44) & 0xF,
            (board >> 48) & 0xF, (board >> 52) & 0xF, (board >> 56) & 0xF, (board >> 60) & 0xF
        });
        int inv = -1;
        eval_t ret = max_tile << 32;
        for (int i = 7; i >= 0; --i) {
            const eval_t val = (board >> idxs[i]) & 0xF;
            if (val < mx) {
                inv = idxs[i];
                ret = (max_tile << 32) - ((mx - val) << (4 * (7 - i)));
            } else {
                mx = val;
                ret += val << (4 * (7 - i));
            }
        }
        ret <<= 9;

        if (inv != -1) {
            const int inv_val = (board >> inv) & 0xF;
            if ((inv & 0b1100) != 0b1100) {  // not on left edge
                // left side should be smaller only if inv is on first row
                if (inv < 16) ret += _val_cmp((board >> (inv + 4)) & 0xF, inv_val);
            }
            // no check required for going up, inversion location is guaranteed to not be on top edge
            ret += _val_cmp((board >> (inv + 16)) & 0xF, inv_val);
            if ((inv & 0b1100) != 0) {  // not on right edge
                // right side should be smaller only if inv is on second row
                if (inv >= 16) ret += _val_cmp((board >> (inv - 4)) & 0xF, inv_val);
            }
        } else {
            ret += _val_cmp((board >> 32) & 0xF, (board >> 16) & 0xF) +
                   _val_cmp((board >> 36) & 0xF, (board >> 20) & 0xF) +
                   _val_cmp((board >> 40) & 0xF, (board >> 24) & 0xF) +
                   _val_cmp((board >> 44) & 0xF, (board >> 28) & 0xF);
        }
        return ret;
    }

    eval_t strict_wall_heuristic(const board_t board) {
        const int max_tile = get_max_tile(board);
        const board_t flip_h_board = flip_h(board);
        const board_t flip_v_board = flip_v(board);
        const board_t flip_vh_board = flip_v(flip_h_board);
        return std::max({_strict_wall_heuristic(board, max_tile),         _strict_wall_heuristic(transpose(board), max_tile),
                         _strict_wall_heuristic(flip_h_board, max_tile),  _strict_wall_heuristic(transpose(flip_h_board), max_tile),
                         _strict_wall_heuristic(flip_v_board, max_tile),  _strict_wall_heuristic(transpose(flip_v_board), max_tile),
                         _strict_wall_heuristic(flip_vh_board, max_tile), _strict_wall_heuristic(transpose(flip_vh_board), max_tile),
                         0LL});  // make sure the evaluation is non-negative
    }

    // similar to corner_heuristic but with different weights
    eval_t _skewed_corner_heuristic(const board_t board) {
        const eval_t top =
                16 * tile_val(board, 3, 3) + 10 * tile_val(board, 3, 2) + 6 * tile_val(board, 3, 1) + 3 * tile_val(board, 3, 0) +
                10 * tile_val(board, 2, 3) + 6  * tile_val(board, 2, 2) + 3 * tile_val(board, 2, 1) + 1 * tile_val(board, 2, 0) +
                4  * tile_val(board, 1, 3) + 3  * tile_val(board, 1, 2) + 1 * tile_val(board, 1, 1) +
                1  * tile_val(board, 0, 3) + 1  * tile_val(board, 0, 2);

        const eval_t bottom =
                16 * tile_val(board, 0, 0) + 10 * tile_val(board, 0, 1) + 6 * tile_val(board, 0, 2) + 3 * tile_val(board, 0, 3) +
                10 * tile_val(board, 1, 0) + 6  * tile_val(board, 1, 1) + 3 * tile_val(board, 1, 2) + 1 * tile_val(board, 1, 3) +
                4  * tile_val(board, 2, 0) + 3  * tile_val(board, 2, 1) + 1 * tile_val(board, 2, 2) +
                1  * tile_val(board, 3, 0) + 1  * tile_val(board, 3, 1);

        const eval_t left =
                16 * tile_val(board, 0, 3) + 10 * tile_val(board, 1, 3) + 6 * tile_val(board, 2, 3) + 3 * tile_val(board, 3, 3) +
                10 * tile_val(board, 0, 2) + 6  * tile_val(board, 1, 2) + 3 * tile_val(board, 2, 2) + 1 * tile_val(board, 3, 2) +
                4  * tile_val(board, 0, 1) + 3  * tile_val(board, 1, 1) + 1 * tile_val(board, 2, 1) +
                1  * tile_val(board, 0, 0) + 1  * tile_val(board, 1, 0);

        const eval_t right =
                16 * tile_val(board, 3, 0) + 10 * tile_val(board, 2, 0) + 6 * tile_val(board, 1, 0) + 3 * tile_val(board, 0, 0) +
                10 * tile_val(board, 3, 1) + 6  * tile_val(board, 2, 1) + 3 * tile_val(board, 1, 1) + 1 * tile_val(board, 0, 1) +
                4  * tile_val(board, 3, 2) + 3  * tile_val(board, 2, 2) + 1 * tile_val(board, 1, 2) +
                1  * tile_val(board, 3, 3) + 1  * tile_val(board, 2, 3);

        return std::max({top, bottom, left, right});
    }

    eval_t skewed_corner_heuristic(const board_t board) {
        return std::max(_skewed_corner_heuristic(board), _skewed_corner_heuristic(transpose(board)));
    }

    // duplicate tiles are only counted if they're not right next to each other
    // for each duplicate pair, the value of the tile is added to the score
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

    // generates monotonicity scores for each row
    // rows with larger tiles receive higher scores but also larger penalties in the case where the row isn't monotonic
    consteval std::array<eval_t, ROWS> gen_monotonicity() {
        std::array<eval_t, ROWS> monotonicity;
        for (int row = 0; row < ROWS; ++row) {
            const int r[4] = {(row >> 12) & 0xF, (row >> 8) & 0xF, (row >> 4) & 0xF, row & 0xF};
            monotonicity[row] = (1 << r[0]) + (1 << r[1]) + (1 << r[2]) + (1 << r[3]);
            for (int i = 0; i < 3; ++i) {
                if (r[i] < r[i + 1]) {
                    if (r[i] == 0) monotonicity[row] -= 1LL << (3 * r[i + 1] / 2);
                    else monotonicity[row] -= 1LL << (3 * r[i + 1] - 2 * r[i]);
                }
            }
        }
        for (int row = 0; row < ROWS; ++row) {
            monotonicity[row] = std::max(monotonicity[row], monotonicity[reversed[row]]);
        }
        return monotonicity;
    }

    constexpr std::array<eval_t, ROWS> monotonicity = gen_monotonicity();
    eval_t monotonicity_heuristic(const board_t board) {
        const board_t transposed_board = transpose(board);
        return std::max(0LL,  // all evaluations should be non-negative
                        monotonicity[(board >> 48) & 0xFFFF] +
                        monotonicity[(board >> 32) & 0xFFFF] +
                        monotonicity[(board >> 16) & 0xFFFF] +
                        monotonicity[ board        & 0xFFFF] +
                        monotonicity[(transposed_board >> 48) & 0xFFFF] +
                        monotonicity[(transposed_board >> 32) & 0xFFFF] +
                        monotonicity[(transposed_board >> 16) & 0xFFFF] +
                        monotonicity[ transposed_board        & 0xFFFF] +
                        std::max({monotonicity[(board >> 48) & 0xFFFF], monotonicity[board & 0xFFFF],
                                  monotonicity[(transposed_board >> 48) & 0xFFFF], monotonicity[transposed_board & 0xFFFF]}) * 8) +
               count_empty(to_tile_mask(board));  // if things go bad and the main heuristic becomes 0, the best way to fix it is to clear up the board
    }

    constexpr heuristic_t exports[8] = {
        score_heuristic,
        merge_heuristic,
        corner_heuristic,
        wall_gap_heuristic,
        full_wall_heuristic,
        strict_wall_heuristic,
        skewed_corner_heuristic,
        monotonicity_heuristic,
    };
}
