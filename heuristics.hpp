#include <chrono>
#include <thread>
#include "util.hpp"
#include "machine_learning/td0.hpp"


namespace heuristics {
    // all heuristic evaluations must be non-negative
    constexpr eval_t MIN_EVAL = 0;
    constexpr eval_t MAX_EVAL = 16ULL << 40;  // from wall heuristics

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

        return std::max({lower_left, upper_left, lower_right, upper_right});  // using initializer list takes about same time as 3 std::max calls
    }

    // compares boards lexicographically, going in a snake across a wall of the board with a gap on the side
    // order (where x doesn't get counted):
    // 0 1 2 x
    // 5 4 3 x
    // 6 7 8 x
    // x x x x
    // takes the maximum over all 4 walls, both transposed and not
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

        return std::max({top, bottom, left, right});
    }
    eval_t wall_gap_heuristic(const board_t board) {
        return std::max(_wall_gap_heuristic(board), _wall_gap_heuristic(transpose(board))) + score_heuristic(board);  // tiebreak by score
    }

    // compares boards lexicographically, going in a snake across an entire wall of the board
    // order (where x doesn't get counted):
    // 0 1 2 3
    // 7 6 5 4
    // 8 x x x
    // x x x x
    // takes the maximum over all 4 walls, both transposed and not
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

        return std::max({top, bottom, left, right});
    }
    eval_t full_wall_heuristic(const board_t board) {
        return std::max(_full_wall_heuristic(board), _full_wall_heuristic(transpose(board))) + score_heuristic(board);  // tiebreak by score
    }

    // same as full_wall_heuristic, but with a penalty for tiles that are inverted in the order
    eval_t _strict_wall_heuristic(const board_t board) {
        const int vals[9] = {
            static_cast<int>(tile_exp(board, 3, 3)), static_cast<int>(tile_exp(board, 3, 2)),
            static_cast<int>(tile_exp(board, 3, 1)), static_cast<int>(tile_exp(board, 3, 0)),
            static_cast<int>(tile_exp(board, 2, 0)), static_cast<int>(tile_exp(board, 2, 1)),
            static_cast<int>(tile_exp(board, 2, 2)), static_cast<int>(tile_exp(board, 2, 3)),
            static_cast<int>(tile_exp(board, 1, 3))};
        eval_t ret = vals[0];
        for (int i = 0; i < 8; ++i) {
            if (vals[i] < vals[i+1] && vals[i] != 0 && vals[i+1] != 0) {
                return (ret - vals[i+1]) * (1LL << (4 * (10 - i)));
            }
            ret = (ret << 4) | vals[i+1];
        }
        return ret << 8;
    }
    eval_t strict_wall_heuristic(const board_t board) {
        const board_t flip_h_board = flip_h(board);
        const board_t flip_v_board = flip_v(board);
        const board_t flip_vh_board = flip_v(flip_h_board);
        return std::max({_strict_wall_heuristic(board),         _strict_wall_heuristic(transpose(board)),
                         _strict_wall_heuristic(flip_h_board),  _strict_wall_heuristic(transpose(flip_h_board)),
                         _strict_wall_heuristic(flip_v_board),  _strict_wall_heuristic(transpose(flip_v_board)),
                         _strict_wall_heuristic(flip_vh_board), _strict_wall_heuristic(transpose(flip_vh_board)),
                         0LL})  // make sure the evaluation is non-negative
             + score_heuristic(board);  // tiebreak by score
    }

    // similar to corner_heuristic but with different weights
    eval_t _skewed_corner_heuristic(const board_t board) {
        const eval_t top    = 16 * tile_val(board, 3, 3) + 10 * tile_val(board, 3, 2) + 6 * tile_val(board, 3, 1) + 3 * tile_val(board, 3, 0) +
                              10 * tile_val(board, 2, 3) + 6  * tile_val(board, 2, 2) + 3 * tile_val(board, 2, 1) + 1 * tile_val(board, 2, 0) +
                              4  * tile_val(board, 1, 3) + 3  * tile_val(board, 1, 2) + 1 * tile_val(board, 1, 1) +
                              1  * tile_val(board, 0, 3) + 1  * tile_val(board, 0, 2);

        const eval_t bottom = 16 * tile_val(board, 0, 0) + 10 * tile_val(board, 0, 1) + 6 * tile_val(board, 0, 2) + 3 * tile_val(board, 0, 3) +
                              10 * tile_val(board, 1, 0) + 6  * tile_val(board, 1, 1) + 3 * tile_val(board, 1, 2) + 1 * tile_val(board, 1, 3) +
                              4  * tile_val(board, 2, 0) + 3  * tile_val(board, 2, 1) + 1 * tile_val(board, 2, 2) +
                              1  * tile_val(board, 3, 0) + 1  * tile_val(board, 3, 1);

        const eval_t left   = 16 * tile_val(board, 0, 3) + 10 * tile_val(board, 1, 3) + 6 * tile_val(board, 2, 3) + 3 * tile_val(board, 3, 3) +
                              10 * tile_val(board, 0, 2) + 6  * tile_val(board, 1, 2) + 3 * tile_val(board, 2, 2) + 1 * tile_val(board, 3, 2) +
                              4  * tile_val(board, 0, 1) + 3  * tile_val(board, 1, 1) + 1 * tile_val(board, 2, 1) +
                              1  * tile_val(board, 0, 0) + 1  * tile_val(board, 1, 0);

        const eval_t right  = 16 * tile_val(board, 3, 0) + 10 * tile_val(board, 2, 0) + 6 * tile_val(board, 1, 0) + 3 * tile_val(board, 0, 0) +
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
            const int r[4] = { (row >> 12) & 0xF, (row >> 8) & 0xF, (row >> 4) & 0xF, row & 0xF };
            monotonicity[row] = (1 << r[0]) + (1 << r[1]) + (1 << r[2]) + (1 << r[3]);
            for (int i = 0; i < 3; ++i) {
                if (r[i] < r[i + 1] && r[i] > 0) {
                    monotonicity[row] -= 1LL << (3 * r[i + 1] - 2 * r[i]);
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
                        monotonicity[ transposed_board        & 0xFFFF] -
                        _duplicate_score(board));  // penalize having duplicate tiles
    }

    // loads and uses the best ML model
    eval_t n_tuple_heuristic(const board_t board) {  // TODO: this is slightly inaccurate since model.evaluate is trained on afterstates
        if (!TD0::best_model_loaded) {
            std::cerr << "model isn't loaded yet! evaluation may be messed up." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // pause a bit to let the model finish loading
            // TODO: figure out a better action
        }
        return TD0::best_model.evaluate(board);
    }

    constexpr heuristic_t exports[9] = {
        score_heuristic,
        merge_heuristic,
        corner_heuristic,
        wall_gap_heuristic,
        full_wall_heuristic,
        strict_wall_heuristic,
        skewed_corner_heuristic,
        monotonicity_heuristic,
        n_tuple_heuristic
    };
}
