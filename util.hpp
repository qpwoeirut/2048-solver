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

// from https://github.com/nneonneo/2048-ai/blob/master/2048.cpp#L38-L48
// transposes 0123 to 048c
//            4567    159d
//            89ab    26ae
//            cdef    37bf
board_t transpose(const board_t board) {
    const board_t a = ((board & 0x0000F0F00000F0F0ULL) << 12) | ((board & 0xF0F00F0FF0F00F0FULL) | (board & 0x0F0F00000F0F0000ULL) >> 12);
    return ((a & 0x00000000FF00FF00ULL) << 24) | (a & 0xFF00FF0000FF00FFULL) | ((a & 0x00FF00FF00000000ULL) >> 24);
}

unsigned long long get_current_time_ms() {
    const std::chrono::time_point now = std::chrono::system_clock::now();
    const unsigned long long seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
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

// every move, a 2 or 4 tile spawns, so we can calculate move count by board sum
// the -2 is because the board starts with two tiles
int count_moves_made(const board_t board, const int fours) {
    return (board_sum(board) >> 1) - fours - 2;
}

// assumes that only 2's have spawned, which is a good enough approximation
// creating a tile of 2^n adds 2^n to the score, and requires two 2^(n-1) tiles
// creating each of those added 2^(n-1) to the score, and following the recursive pattern gets n * 2^n
// technically we want (n-1) * 2^n since the 2's spawning don't add to the score
int approximate_score(const board_t board) {
    int score = 0;
    for (int i=0; i<64; i+=4) {
        const uint8_t tile = (board >> i) & 0xF;
        score += tile <= 1 ? 0 : (tile - 1) * (1 << tile);
    }
    return score;
}
int actual_score(const board_t board, const int fours) {
    return approximate_score(board) - 4 * fours;
}

#endif
