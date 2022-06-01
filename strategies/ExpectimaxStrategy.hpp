#ifndef EXPECTIMAX_STRATEGY_HPP
#define EXPECTIMAX_STRATEGY_HPP

#include "Strategy.hpp"
#include "sparsehash/dense_hash_map"
//using cache_t = std::unordered_map<board_t, eval_t>;
using cache_t = google::dense_hash_map<board_t, eval_t>;  // https://github.com/sparsehash/sparsehash

class ExpectimaxStrategy: public Strategy {
    /*
        Parameters:
            depth: depth to search; note that search space increases exponentially with depth
                   a nonpositive depth argument d will be subtracted from the depth picker's result (increasing the depth)
    */

    static constexpr int MAX_DEPTH = 16;
    static constexpr int CACHE_DEPTH = 2;
    static constexpr int USUAL_CACHE = 1 << 16;

    static constexpr board_t INVALID_BOARD  = 0x1111111111111111ULL;  // used as the empty_key for the dense_hash_map cache
    static constexpr board_t INVALID_BOARD2 = 0x2222222222222222ULL;  // used as the delete_key for the dense_hash_map cache

    // according to a single benchmark than I ran:
    // cache can reach up to 700k-ish
    // but 99% of the time it's less than 130k
    // and 97% of the time it's less than 60k
    static constexpr int MAX_CACHE = 1 << 20;  // code below assumes this is power of 2

    heuristic_t evaluator;
    cache_t cache;

    board_t deletion_queue[MAX_CACHE];
    int q[4] = {0, 0, 0, 0};
    int q_end = 0;
    // queue looks like [q_0, c_0, q_1, c_1, q_2, c_2, q_3, c_3, q_end] where c_n holds the cache that will be deleted after n moves (with wrap around)
    // after each turn, the range from q_0 to q_1 gets deleted and everything shifts upward

    // speed things up with integer arithmetic
    // expected score * 10, 4 moves, 30 tile placements, multiplied by 4 to pack score and move, times 16 to pack cache
    static constexpr eval_t MULT = 4e18 / (heuristics::MAX_EVAL * 10 * 4 * 30 * 4 * 16);
    static_assert(MULT > 1);

    public:
    int depth;
    ExpectimaxStrategy(const int _depth, const heuristic_t _evaluator) {
        depth = _depth;
        evaluator = _evaluator;

        cache = cache_t(USUAL_CACHE);
        cache.set_empty_key(INVALID_BOARD);
        cache.set_deleted_key(INVALID_BOARD2);
        cache.min_load_factor(0.3);  // shrink quickly
        cache.max_load_factor(0.9);  // but expand slowly
    }
    ExpectimaxStrategy(const int _depth, const int heuristic_idx) :
        ExpectimaxStrategy(_depth, heuristics::exports[heuristic_idx]) {}

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<ExpectimaxStrategy>(depth, evaluator);
    }

    void reset() override {
        cache.clear();
    }

    const int pick_move(const board_t board) override {
        const int depth_to_use = depth <= 0 ? pick_depth(board) - depth : depth;

        const int move = helper(board, depth_to_use, 0) & 3;

        while (q[0] < q[1]) {  // delete everything in range q_0 ... q_1
            cache.erase(deletion_queue[q[0]++]);
            if (q[0] >= MAX_CACHE) {
                q[0] -= MAX_CACHE;
                q[1] -= MAX_CACHE;
                q[2] -= MAX_CACHE;
                q[3] -= MAX_CACHE;
                q_end -= MAX_CACHE;
            }
        }
        q[1] = std::max(q[0], q[2]);
        q[2] = std::max(q[0], q[3]);  // possible that stuff was deleted while searching because cache got too big
        q[3] = q_end;
        //std::cout << q[0] << ' ' << q[1] << ' ' << q[2] << ' ' << q[3] << ' ' << q_end << ' ' << q_end - q[0] << ' ' << cache.size() << std::endl;
        return move;
    }

    private:
    const eval_t helper(const board_t board, const int cur_depth, const int fours) {
        if (simulator.game_over(board)) {
            const eval_t score = MULT * evaluator(board);
            return score - (score >> 4);  // subtract score / 16 as penalty for dying
        }
        if (cur_depth == 0 || fours >= 5) {  // selecting 5 fours has a 0.001% chance, which is negligible
            return (MULT * evaluator(board)) << 2;  // move doesn't matter
        }

        if (cur_depth >= CACHE_DEPTH) {
            const cache_t::iterator it = cache.find(board);
            #ifdef REQUIRE_DETERMINISTIC
            if (it != cache.end() && (it->second & 0xF) == cur_depth) return it->second >> 4;
            #else
            if (it != cache.end() && (it->second & 0xF) >= cur_depth) return it->second >> 4;
            #endif
        }

        eval_t best_score = heuristics::MIN_EVAL;
        int best_move = 0;  // default best_move to 0; -1 causes issues with the packing in cases of full boards
        for (int i=0; i<4; ++i) {
            eval_t expected_score = 0;
            const board_t new_board = simulator.make_move(board, i);
            if (board == new_board) {
                continue;
            } else {
                const uint16_t empty_mask = to_tile_mask(new_board);
                for (int j=0; j<16; ++j) {
                    if (((empty_mask >> j) & 1) == 0) {
                        expected_score += 9 * (helper(new_board | (1LL << (j << 2)), cur_depth - 1, fours) >> 2);
                        expected_score += 1 * (helper(new_board | (2LL << (j << 2)), cur_depth - 1, fours + 1) >> 2);
                    }
                }
                expected_score /= count_empty(empty_mask) * 10;  // convert to actual expected score * MULT
            }

            if (best_score <= expected_score) {
                best_score = expected_score;
                best_move = i;
            }
        }

        if (cur_depth >= CACHE_DEPTH) {
            cache[board] = (((best_score << 2) | best_move) << 4) | cur_depth;

            // relies on MAX_CACHE being a power of 2
            deletion_queue[q_end & (MAX_CACHE - 1)] = board;
            ++q_end;
            if (q[0] + MAX_CACHE == q_end) {
                cache.erase(deletion_queue[q[0]++]);
                if (q[0] >= MAX_CACHE) {
                    q[0] -= MAX_CACHE;
                    q[1] -= MAX_CACHE;
                    q[2] -= MAX_CACHE;
                    q[3] -= MAX_CACHE;
                    q_end -= MAX_CACHE;
                }
            }
        }

        return (best_score << 2) | best_move;  // pack both score and move
    }

    const int pick_depth(const board_t board) {
        const int tile_ct = count_set(to_tile_mask(board));
        const int score = count_distinct_tiles(board) + (tile_ct <= 6 ? 0 : (tile_ct - 6) >> 1);
        return 2 + (score > 7) + (score > 10) + (score > 13) + (score > 14) + (score > 16);
    }
};
#endif

