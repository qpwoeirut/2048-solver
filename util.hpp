#ifndef UTIL_HPP
#define UTIL_HPP

#include <chrono>
#include <random>

using row_t = uint16_t;
using board_t = uint64_t;

using eval_t = int64_t;
using heuristic_t = eval_t (*)(const board_t);

// bitmask of whether a tile is empty or not
// TODO there are probably faster ways, such as:
// https://stackoverflow.com/questions/34154745/efficient-way-to-or-adjacent-bits-in-64-bit-integer
// https://stackoverflow.com/questions/4909263/how-to-efficiently-de-interleave-bits-inverse-morton
// but this works for now
uint16_t to_tile_mask(const board_t board) {
    uint16_t tile_mask = 0;
    for (int i=0; i<16; ++i) {
        tile_mask |= (((board >> (4*i)) & 0xF) > 0) << i;
    }
    return tile_mask;
}

long long get_current_time_ms() {
    const std::chrono::time_point now = std::chrono::system_clock::now();
    const long long seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return seconds;
}

void print_board(const board_t board) {
    for (int r=48; r>=0; r-=16) {
        for (int c=12; c>=0; c-=4) {
            std::cout << ((board >> (r + c)) & 0xF) << ' ';
        }
        std::cout << '\n';
    }
}

int get_max_tile(const board_t board) {
    int max_tile = 0;
    for (int i=0; i<64; i+=4) max_tile = std::max(max_tile, (int)((board >> i) & 0xF));
    return max_tile;
}

// a somewhat understandable way to count set bits
// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
int count_empty(uint16_t mask) {
    int empty_ct;
    for (empty_ct = 16; mask > 0; --empty_ct) {
        mask &= mask - 1;
    }
    return empty_ct;
}
int count_set(const uint16_t mask) {
    return 16 - count_empty(mask);
}

int count_distinct_tiles(const board_t board) {
    uint16_t tile_exists = 0;
    for (int i = 0; i < 64; i += 4) if (((board >> i) & 0xF) != 0) tile_exists |= 1 << ((board >> i) & 0xF);
    return count_set(tile_exists);
}

int board_sum(const board_t board) {
    int sum = 0;
    for (int i = 0; i < 64; i += 4) if (((board >> i) & 0xF) != 0) sum += 1 << ((board >> i) & 0xF);
    return sum;
}

#endif

