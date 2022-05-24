#include <cassert>
#include <iostream>
#include <random>

#include "util.hpp"
#include "strategies/Strategy.hpp"

namespace game {
    constexpr int ROWS = 0x10000;
    constexpr int EMPTY_MASKS = 0x10000; // number of tile_masks, where an tile_mask stores whether a tile is empty
    constexpr row_t WINNING_ROW = 0xFFFF; // 2^16 - 1, represents [32768, 32768, 32768, 32768], which is very unlikely

    constexpr board_t WINNING_BOARD  = 0xFFFFFFFFFFFFFFFFULL;  // 2^64 - 1, represents grid full of 32768 tiles (which is impossible)
    constexpr board_t INVALID_BOARD  = 0x1111111111111111ULL;  // used as the empty_key for the dense_hash_map cache
    constexpr board_t INVALID_BOARD2 = 0x2222222222222222ULL;  // used as the delete_key for the dense_hash_map cache


    constexpr uint16_t FULL_MASK = 0xFFFF;

    row_t shift[2][ROWS];  // shift[0] is left shift, shift[1] is right shift

    // this uses a fancy way of implementing adjacency lists in competitive programming
    // stores the empty tile positions for each tile_mask
    constexpr int EMPTY_TILE_POSITIONS = 524288;  // exactly 524288 different values across all tile_masks
    uint8_t empty_tiles[EMPTY_TILE_POSITIONS];
    int empty_index[EMPTY_MASKS];  // a pointer to where this tile_mask starts

    std::mt19937 empty_tile_gen;
    std::uniform_int_distribution<> empty_tile_distrib(0, 720720 - 1);  // 720720 is lcm(1, 2, 3, ... , 15, 16), providing an even distribution

    // performance isn't important; this is only used in init()
    row_t reverse_row(const row_t row) {
        return ((row & 0xF) << 12) | (((row >> 4) & 0xF) << 8) | (((row >> 8) & 0xF) << 4) | ((row >> 12) & 0xF);
    }

    // this part doesn't have to be fast
    void init(const long long rng_seed = get_current_time_ms()) {
        empty_tile_gen.seed(rng_seed);  // seed the rng with current time
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
            // if (r[0] >= 16 ||r[1] >= 16 || r[2] >= 16 || r[3] >= 16) shift[0][row] = shift[1][row] = WINNING_ROW;
        }

        int idx = 0;
        for (int em=0; em<EMPTY_MASKS; ++em) {
            empty_index[em] = idx;
            for (int pos=0; pos<16; ++pos) {
                if (((em >> pos) & 1) == 0) {
                    empty_tiles[idx++] = 4 * pos;
                }
            }
        }
        assert(idx == EMPTY_TILE_POSITIONS);
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

    board_t play(Strategy& player, int& fours) {
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
    
    board_t play_slow(Strategy& player, int& fours) {
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
}
