#ifndef GAME_HPP
#define GAME_HPP

#include <array>
#include <cassert>
#include <iostream>
#include <random>

#include "util.hpp"

class Strategy;  // Strategy depends on GameSimulator and will be #include-ed at the bottom

consteval row_t reverse_row(const row_t row) {
    return ((row & 0xF) << 12) | (((row >> 4) & 0xF) << 8) | (((row >> 8) & 0xF) << 4) | ((row >> 12) & 0xF);
}

static constexpr int ROWS = 0x10000;
static constexpr row_t WINNING_ROW = 0xFFFF; // 2^16 - 1, represents [32768, 32768, 32768, 32768], which is very unlikely
consteval std::array<std::array<row_t, ROWS>, 2> generate_shift() {
    std::array<std::array<row_t, ROWS>, 2> shift;
    for (int row=0; row<ROWS; ++row) {
        uint8_t r[4] = {
            static_cast<uint8_t>((row >> 12) & 0xF),
            static_cast<uint8_t>((row >> 8) & 0xF),
            static_cast<uint8_t>((row >> 4) & 0xF),
            static_cast<uint8_t>(row & 0xF)
        };

        // pull values to the left
        for (int i=0; i<3; ++i) {
            if (r[0] == 0 && r[1] > 0) std::swap(r[0], r[1]);
            if (r[1] == 0 && r[2] > 0) std::swap(r[1], r[2]);
            if (r[2] == 0 && r[3] > 0) std::swap(r[2], r[3]);
        }

        // perform the merging
        if (r[0] > 0 && r[0] == r[1]) r[0]++, r[1] = 0;
        if (r[1] > 0 && r[1] == r[2]) r[1]++, r[2] = 0;
        if (r[2] > 0 && r[2] == r[3]) r[2]++, r[3] = 0;

        // pull values to the left again
        for (int i=0; i<3; ++i) {
            if (r[0] == 0 && r[1] > 0) std::swap(r[0], r[1]);
            if (r[1] == 0 && r[2] > 0) std::swap(r[1], r[2]);
            if (r[2] == 0 && r[3] > 0) std::swap(r[2], r[3]);
        }

        shift[0][row] = (r[0] << 12) | (r[1] << 8) | (r[2] << 4) | r[3];
        shift[1][reverse_row(row)] = reverse_row(shift[0][row]);

        // we can't handle a 65536 tile in this representation, but it's unlikely that this will happen
        if (r[0] >= 16 ||r[1] >= 16 || r[2] >= 16 || r[3] >= 16) shift[0][row] = shift[1][row] = WINNING_ROW;
    }
    return shift;
}

static constexpr int EMPTY_TILE_POSITIONS = 524288;  // exactly 524288 different values across all tile_masks
static constexpr int EMPTY_MASKS = 0x10000;  // number of tile_masks, where an tile_mask stores whether a tile is empty
consteval std::array<uint8_t, EMPTY_TILE_POSITIONS> generate_empty_tiles() {
    std::array<uint8_t, EMPTY_TILE_POSITIONS> empty_tiles;
    int idx = 0;
    for (int em=0; em<EMPTY_MASKS; ++em) {
        for (int pos=0; pos<16; ++pos) {
            if (((em >> pos) & 1) == 0) {
                empty_tiles[idx++] = 4 * pos;
            }
        }
    }
    assert(idx == EMPTY_TILE_POSITIONS);
    return empty_tiles;
}
consteval std::array<int, EMPTY_MASKS> generate_empty_index() {
    std::array<int, EMPTY_MASKS> empty_index;
    int idx = 0;
    for (int em=0; em<EMPTY_MASKS; ++em) {
        empty_index[em] = idx;
        for (int pos=0; pos<16; ++pos) {
            if (((em >> pos) & 1) == 0) {
                ++idx;
            }
        }
    }
    assert(idx == EMPTY_TILE_POSITIONS);
    return empty_index;
}

class GameSimulator {
    static constexpr board_t WINNING_BOARD  = 0xFFFFFFFFFFFFFFFFULL;  // 2^64 - 1, represents grid full of 32768 tiles (which is impossible)

    static constexpr uint16_t FULL_MASK = 0xFFFF;

    static constexpr std::array<std::array<row_t, ROWS>, 2> shift = generate_shift();

    // this uses a fancy way of implementing adjacency lists in competitive programming
    // stores the empty tile positions for each tile_mask
    static constexpr std::array<uint8_t, EMPTY_TILE_POSITIONS> empty_tiles = generate_empty_tiles();
    static constexpr std::array<int, EMPTY_MASKS> empty_index = generate_empty_index();  // a pointer to where this tile_mask starts

    std::mt19937 empty_tile_gen;
    std::uniform_int_distribution<> empty_tile_distrib{0, 720720 - 1};  // 720720 is lcm(1, 2, 3, ... , 15, 16), providing an even distribution

    public:
    GameSimulator(const long long rng_seed = get_current_time_ms()) {
        empty_tile_gen.seed(rng_seed);
    }

    board_t make_move(board_t board, const int dir) {  // 0=left, 1=up, 2=right, 3=down
        if (dir & 1) board = transpose(board);
        board = ((board_t)shift[dir >> 1][ board >> 48          ] << 48) |
                ((board_t)shift[dir >> 1][(board >> 32) & 0xFFFF] << 32) |
                ((board_t)shift[dir >> 1][(board >> 16) & 0xFFFF] << 16) |
                 (board_t)shift[dir >> 1][ board        & 0xFFFF];
        
        // checks if the 65536 tile is reached; this won't be happening for a while
        //if (shift[dir >> 1][board >> 48] == WINNING_ROW ||
        //    shift[dir >> 1][(board >> 32) & 0xFFFF] == WINNING_ROW ||
        //    shift[dir >> 1][(board >> 16) & 0xFFFF] == WINNING_ROW ||
        //    shift[dir >> 1][board & 0xFFFF] == WINNING_ROW) return WINNING_BOARD;
        return (dir & 1) ? transpose(board) : board;
    }

    board_t generate_random_tile_val() {
        return 1ULL + ((empty_tile_distrib(empty_tile_gen) % 10) == 0);
    }

    board_t add_random_tile(const board_t board, const board_t tile_val) {
        const uint16_t tile_mask = to_tile_mask(board);

        // can't add a tile to a full board
        // also prevents any possible overflow on the next few lines
        assert(tile_mask != FULL_MASK);

        const int option_count = empty_index[tile_mask + 1] - empty_index[tile_mask];
        const uint8_t option = empty_tiles[empty_index[tile_mask] + (empty_tile_distrib(empty_tile_gen) % option_count)];

        const board_t new_board = board | (tile_val << option);
        
        return new_board;
    }

    bool game_over(const board_t board) {
        return (board == make_move(board, 0) && board == make_move(board, 1) && board == make_move(board, 2) && board == make_move(board, 3));// || board == WINNING_BOARD;
    }

    board_t play(Strategy&, int&);
    board_t play_slow(Strategy&, int&);
};

#include "strategies/Strategy.hpp"

board_t GameSimulator::play(Strategy& player, int& fours) {
    const board_t tile_val0 = generate_random_tile_val();
    const board_t tile_val1 = generate_random_tile_val();
    fours += (tile_val0 == 2) + (tile_val1 == 2);
    board_t board = add_random_tile(add_random_tile(0, tile_val0), tile_val1);

    while (!game_over(board)) {
        const board_t old_board = board;

        int attempts = 0x10000;
        while (old_board == board) {
            const int dir = player.pick_move(board);
            assert(0 <= dir && dir < 4);
            board = make_move(board, dir);

            if (game_over(board)) return board;

            assert(--attempts > 0);  // abort the game if it seems stuck
        } 

        // 90% for 2^1 = 2, 10% for 2^2 = 4
        const board_t new_tile_val = generate_random_tile_val();
        if (new_tile_val == 2) ++fours;
        board = add_random_tile(board, new_tile_val);
    }

    return board;
}
    
board_t GameSimulator::play_slow(Strategy& player, int& fours) {
    const board_t tile_val0 = generate_random_tile_val();
    const board_t tile_val1 = generate_random_tile_val();
    fours += (tile_val0 == 2) + (tile_val1 == 2);
    board_t board = add_random_tile(add_random_tile(0, tile_val0), tile_val1);

    int moves = 0;
    while (!game_over(board)) {
        if (moves-- == 0) {
            print_board(board);
            std::cout << "Moves to jump? ";
            std::cin >> moves;
        }
        const board_t old_board = board;

        int attempts = 0x10000;
        while (old_board == board) {
            const int dir = player.pick_move(board);
            assert(0 <= dir && dir < 4);
            board = make_move(board, dir);

            if (game_over(board)) return board;

            assert(--attempts > 0);  // abort the game if it seems stuck
        } 

        // 90% for 2^1 = 2, 10% for 2^2 = 4
        const board_t new_tile_val = generate_random_tile_val();
        if (new_tile_val == 2) ++fours;
        board = add_random_tile(board, new_tile_val);
    }

    return board;
}

#endif

