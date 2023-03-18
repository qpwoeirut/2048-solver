#ifndef GAME_HPP
#define GAME_HPP

#include <array>
#include <cassert>
#include <iostream>
#include <random>

#include "util.hpp"

class Strategy;  // Strategy depends on GameSimulator and will be #include-ed at the bottom

//static constexpr row_t WINNING_ROW = 0xFFFF; // 2^16 - 1, represents [32768, 32768, 32768, 32768], which is very unlikely

// generates the precomputed arrays to compute the results of making a move
consteval std::array<row_t, ROWS> generate_shift() {
    std::array<row_t, ROWS> shift;
    for (int row = 0; row < ROWS; ++row) {
        int r[4] = {(row >> 12) & 0xF, (row >> 8) & 0xF, (row >> 4) & 0xF, row & 0xF};

        // pull values to the left
        for (int i = 0; i < 3; ++i) {
            if (r[0] == 0 && r[1] > 0) r[0] = r[1], r[1] = 0;
            if (r[1] == 0 && r[2] > 0) r[1] = r[2], r[2] = 0;
            if (r[2] == 0 && r[3] > 0) r[2] = r[3], r[3] = 0;
        }

        // perform the merging
        if (r[0] > 0 && r[0] == r[1]) r[0]++, r[1] = 0;
        if (r[1] > 0 && r[1] == r[2]) r[1]++, r[2] = 0;
        if (r[2] > 0 && r[2] == r[3]) r[2]++, r[3] = 0;

        // pull values to the left again
        for (int i = 0; i < 3; ++i) {
            if (r[0] == 0 && r[1] > 0) r[0] = r[1], r[1] = 0;
            if (r[1] == 0 && r[2] > 0) r[1] = r[2], r[2] = 0;
            if (r[2] == 0 && r[3] > 0) r[2] = r[3], r[3] = 0;
        }


        // we can't handle a 65536 tile in this representation, but it's unlikely that we'll ever reach that tile
        // for now just cap values at 2^15
        shift[row] = (std::min(r[0], 15) << 12) | (std::min(r[1], 15) << 8) | (std::min(r[2], 15) << 4) | std::min(r[3], 15);
    }
    return shift;
}

// the game mechanics include adding an tile to an empty position
// to speed up the process, we can precompute where the empty tiles are for each possible board
// since we only care about whether tiles are open or full, we can store a "tile mask" of the board where each tile is a boolean value
// then for each tile mask we store the position of every empty tile
// all empty tiles are in a single array, and a second array stores pointers to which sections correspond to which tile masks
static constexpr int EMPTY_TILE_POSITIONS = 524288;  // exactly 524288 different values across all tile_masks
static constexpr int EMPTY_MASKS = 0x10000;  // number of tile_masks, where an tile_mask stores whether a tile is empty
consteval std::array<uint8_t, EMPTY_TILE_POSITIONS> generate_empty_tiles() {
    std::array<uint8_t, EMPTY_TILE_POSITIONS> empty_tiles;
    int idx = 0;
    for (int em = 0; em < EMPTY_MASKS; ++em) {
        for (int pos = 0; pos < 16; ++pos) {
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
    for (int em = 0; em < EMPTY_MASKS; ++em) {
        empty_index[em] = idx;
        for (int pos = 0; pos < 16; ++pos) {
            if (((em >> pos) & 1) == 0) {
                ++idx;
            }
        }
    }
    assert(idx == EMPTY_TILE_POSITIONS);
    return empty_index;
}

class GameSimulator {
//    static constexpr board_t WINNING_BOARD  = 0xFFFFFFFFFFFFFFFFULL;  // 2^64 - 1, represents grid full of 32768 tiles (which is impossible)

    static constexpr uint16_t FULL_MASK = 0xFFFF;

    static constexpr std::array<row_t, ROWS> shift = generate_shift();

    // this uses a fancy way of implementing adjacency lists in competitive programming
    // stores the empty tile positions for each tile_mask
    static constexpr std::array<uint8_t, EMPTY_TILE_POSITIONS> empty_tiles = generate_empty_tiles();
    static constexpr std::array<int, EMPTY_MASKS> empty_index = generate_empty_index();  // a pointer to where this tile_mask starts

    static constexpr std::array<char, 4> MOVES = {'l', 'u', 'r', 'd'};  // use lowercase to make counting # of 4's placed easier

    std::mt19937 empty_tile_gen;
    std::uniform_int_distribution<> empty_tile_distrib{0, 720720 - 1};
    // 720720 is lcm(1, 2, 3, ... , 15, 16), providing an even distribution

public:
    GameSimulator(const long long rng_seed = get_current_time_ms()) {
        empty_tile_gen.seed(rng_seed);
    }

    board_t make_move(board_t board, const int dir) const {  // 0=left, 1=up, 2=right, 3=down
        if (dir & 1) board = transpose(board);
        if (dir >= 2) board = flip_h(board);
        board = (static_cast<board_t>(shift[(board >> 48) & 0xFFFF]) << 48) |
                (static_cast<board_t>(shift[(board >> 32) & 0xFFFF]) << 32) |
                (static_cast<board_t>(shift[(board >> 16) & 0xFFFF]) << 16) |
                (static_cast<board_t>(shift[ board        & 0xFFFF]));

        // checks if the 65536 tile is reached
//        if (shift[dir >> 1][(board >> 48) & 0xFFFF] == WINNING_ROW ||
//            shift[dir >> 1][(board >> 32) & 0xFFFF] == WINNING_ROW ||
//            shift[dir >> 1][(board >> 16) & 0xFFFF] == WINNING_ROW ||
//            shift[dir >> 1][ board        & 0xFFFF] == WINNING_ROW) return WINNING_BOARD;
        if (dir >= 2) board = flip_h(board);
        return (dir & 1) ? transpose(board) : board;
    }

    board_t generate_random_tile_val() {
        return 1ULL + ((empty_tile_distrib(empty_tile_gen) % 10) == 0);
    }

    uint8_t pick_empty_position(const board_t board) {
        const uint16_t tile_mask = to_tile_mask(board);

        // can't add a tile to a full board
        // also prevents any possible overflow on the next few lines
        assert(tile_mask != FULL_MASK);

        const int option_count = empty_index[tile_mask + 1] - empty_index[tile_mask];
        return empty_tiles[empty_index[tile_mask] + (empty_tile_distrib(empty_tile_gen) % option_count)];
    }

    board_t add_tile(const board_t board, const board_t tile_val) {
        return board | (tile_val << pick_empty_position(board));
    }

    board_t add_tile(const board_t board, const board_t tile_val, std::string& record) {
        const uint8_t position = pick_empty_position(board);
        const board_t new_board = board | (tile_val << position);

        record.push_back((position / 4) + (tile_val == 1 ? 'a' : 'A'));

        return new_board;
    }

    bool game_over(const board_t board) const {
        return (board == make_move(board, 0) && board == make_move(board, 1) && board == make_move(board, 2) &&
                board == make_move(board, 3));// || board == WINNING_BOARD;
    }

    board_t play(Strategy&, std::string&);

    board_t play_slow(Strategy&, std::string&, void (*)(const board_t));
};

#include "strategies/Strategy.hpp"

board_t GameSimulator::play(Strategy& player, std::string& record) {
    // reserve space for 6000 chars, enough for almost 3000 moves. should be enough for most games
    record.reserve(6000);

    const board_t tile_val0 = generate_random_tile_val();
    const board_t tile_val1 = generate_random_tile_val();
    board_t board = add_tile(0, tile_val0, record);
    board = add_tile(board, tile_val1, record);

    while (!game_over(board)) {
        const board_t old_board = board;

        int attempts = 0x10000;
        int dir;
        while (old_board == board) {
            dir = player.pick_move(board);
            assert(0 <= dir && dir < 4);
            board = make_move(board, dir);

            assert(--attempts > 0);  // abort the game if the strategy keeps picking an invalid move
        }
        record.push_back(MOVES[dir]);

        if (game_over(board)) return board;

        // 90% for 2^1 = 2, 10% for 2^2 = 4
        const board_t new_tile_val = generate_random_tile_val();
        board = add_tile(board, new_tile_val, record);
    }

    return board;
}

// similar to GameSimulator::play, but pauses the game for debugging purposes
board_t GameSimulator::play_slow(Strategy& player, std::string& record, void (*callback)(const board_t)) {
    const board_t tile_val0 = generate_random_tile_val();
    const board_t tile_val1 = generate_random_tile_val();
    board_t board = add_tile(0, tile_val0, record);
    board = add_tile(board, tile_val1, record);

    int moves = 0;
    while (!game_over(board)) {
        if (moves-- == 0) {
            print_board(board);
            callback(board);
            std::cout << "Moves to jump? ";
            std::cin >> moves;
        }
        const board_t old_board = board;

        int attempts = 0x10000;
        int dir;
        while (old_board == board) {
            dir = player.pick_move(board);
            assert(0 <= dir && dir < 4);
            board = make_move(board, dir);

            assert(--attempts > 0);  // abort the game if the strategy keeps picking an invalid move
        }
        record.push_back(MOVES[dir]);

        if (game_over(board)) return board;

        // 90% for 2^1 = 2, 10% for 2^2 = 4
        const board_t new_tile_val = generate_random_tile_val();
        board = add_tile(board, new_tile_val, record);
    }

    return board;
}

#endif

