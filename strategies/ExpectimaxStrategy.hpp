#ifndef EXPECTIMAX_STRATEGY_HPP
#define EXPECTIMAX_STRATEGY_HPP

#include "Strategy.hpp"
#include "sparsehash/dense_hash_map"
//using cache_t = std::unordered_map<board_t, eval_t>;
using cache_t = google::dense_hash_map<board_t, eval_t>;  // https://github.com/sparsehash/sparsehash

class ExpectimaxStrategy : public Strategy {
protected:
    const heuristic_t evaluator;

    ExpectimaxStrategy(const heuristic_t _evaluator) : evaluator(_evaluator) {}

    static constexpr int MAX_DEPTH = 10;
    static constexpr int USUAL_CACHE = 1 << 16;

    static constexpr board_t
    INVALID_BOARD = 0x1111111111111111ULL;  // used as the empty_key for the dense_hash_map cache
    static constexpr board_t
    INVALID_BOARD2 = 0x2222222222222222ULL;  // used as the delete_key for the dense_hash_map cache

    // according to a single benchmark that I ran:
    // cache can reach up to 700k-ish
    // but 99% of the time it's less than 130k
    // and 97% of the time it's less than 60k
    static constexpr int MAX_CACHE = 1 << 20;  // code below assumes this is power of 2

    cache_t cache;

    board_t deletion_queue[MAX_CACHE];
    int q[4] = {0, 0, 0, 0};
    int q_end = 0;
    // queue is circular, with 5 pointers
    // after each turn, the range from q_0 to q_1 gets deleted and everything shifts upward

    // speed things up with integer arithmetic
    // expected score * 10, 4 moves, 30 tile placements, multiplied by 4 to pack score and move, times 16 to pack cache
    static constexpr eval_t MULT = 9e18 / (heuristics::MAX_EVAL * 10 * 4 * 30 * 4 * 16);
    static_assert(MULT > 1);

    void init_cache() {
        cache = cache_t(USUAL_CACHE);
        cache.set_empty_key(INVALID_BOARD);
        cache.set_deleted_key(INVALID_BOARD2);
        cache.min_load_factor(0.3);  // shrink quickly
        cache.max_load_factor(0.9);  // but expand slowly
    }

    void add_to_cache(const board_t board, const eval_t score, const int move, const int depth) {
        cache[board] = (((score << 2) | move) << 4) | depth;

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

    void update_cache_pointers() {
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
    }

public:
    void reset() override {
        cache.clear();
    }
};

#endif
