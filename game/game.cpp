#include <cassert>
#include <chrono>
#include <random>

#include "util.cpp"

namespace game {
    const int ROWS = 0x10000;
    const int EMPTY_MASKS = 0x10000; // number of tile_masks, where an tile_mask stores whether a tile is empty
    const row_t WINNING_ROW = 0xFFFF; // 2^16 - 1, represents [32768, 32768, 32768, 32768], which is very unlikely

    const board_t WINNING_BOARD = 18446744073709551615ULL;  // 2^64 - 1, represents grid full of 32768 tiles (which is impossible)

    const uint16_t FULL_MASK = 0xFFFF;

    row_t shift[2][ROWS];  // shift[0] is left shift, shift[1] is right shift

    // this uses a fancy way of implementing adjacency lists in competitive programming
    // stores the empty tile positions for each tile_mask
    uint8_t empty_tiles[15 * EMPTY_MASKS];  // up to 15 empty tiles per mask
    int empty_index[EMPTY_MASKS];  // a pointer to where this tile_mask starts

    row_t reverse_row(const row_t row) {
        return ((row & 0xF) << 12) | (((row >> 4) & 0xF) << 8) | (((row >> 8) & 0xF) << 4) | ((row >> 12) & 0xF);
    }

    // this part doesn't have to be fast
    void init() {
        for (int row=0; row<ROWS; ++row) {
            uint8_t r[4] = {(row >> 12) & 0xF, (row >> 8) & 0xF, (row >> 4) & 0xF, row & 0xF};

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

            // we can't handle a 65536 tile in this representation
            if (r[0] >= 16 ||r[1] >= 16 || r[2] >= 16 || r[3] >= 16) shift[0][row] = shift[1][row] = WINNING_ROW;
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
    }

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

    board_t make_move(board_t board, const int dir) {  // 0=left, 1=up, 2=right, 3=down
        if (dir & 1) board = transpose(board);
        board = ((board_t)shift[dir >> 1][ board >> 48          ] << 48) |
                ((board_t)shift[dir >> 1][(board >> 32) & 0xFFFF] << 32) |
                ((board_t)shift[dir >> 1][(board >> 16) & 0xFFFF] << 16) |
                 (board_t)shift[dir >> 1][ board        & 0xFFFF];
        if (shift[dir >> 1][board >> 48] == WINNING_ROW ||
            shift[dir >> 1][(board >> 32) & 0xFFFF] == WINNING_ROW ||
            shift[dir >> 1][(board >> 16) & 0xFFFF] == WINNING_ROW ||
            shift[dir >> 1][board & 0xFFFF] == WINNING_ROW) return WINNING_BOARD;
        return (dir & 1) ? transpose(board) : board;
    }

    std::mt19937 empty_tile_gen(get_current_time_ms());  // seed the rng with current time
    std::uniform_int_distribution<> empty_tile_distrib(0, 720720 - 1);  // 720720 is lcm(1, 2, 3, ... , 15, 16), providing an even distribution

    board_t add_random_tile(const board_t board) {
        const uint16_t tile_mask = to_tile_mask(board);

        // can't add a tile to a full board
        // also prevents any possible overflow on the next few lines
        assert(tile_mask != FULL_MASK);

        const int option_count = empty_index[tile_mask + 1] - empty_index[tile_mask];
        const uint8_t option = empty_tiles[empty_index[tile_mask] + (empty_tile_distrib(empty_tile_gen) % option_count)];

        const board_t new_board = board | (((empty_tile_distrib(empty_tile_gen) % 10) == 0 ? 2ULL : 1ULL) << option);  // 90% for 2^1 = 2, 10% for 2^2 = 4 
        
        return new_board;
    }

    bool game_over(const board_t board) {
        return (board == make_move(board, 0) && board == make_move(board, 1) && board == make_move(board, 2) && board == make_move(board, 3)) ||
                board == WINNING_BOARD;
    }

    board_t play(int (*player)(board_t)) {
        board_t board = add_random_tile(0);

        while (!game_over(board)) {
            const board_t old_board = board;

            int attempts = 0x10000;
            while (old_board == board) {
                int dir = player(board);
                assert(0 <= dir && dir < 4);
                board = make_move(board, dir);

                if (game_over(board)) return board;

                assert(--attempts > 0);  // abort the game if it seems stuck
            } 

            board = add_random_tile(board);
        }

        return board;
    }
    
    board_t play_slow(int (*player)(board_t)) {
        board_t board = add_random_tile(0);

        int moves = 0;
        while (!game_over(board)) {
            if (moves-- == 0) {
                print_board(board);
                std::cout << "Moves to jump? ";
                std::cin >> moves;
            }
            const board_t old_board = board;

            while (old_board == board) {
                int dir = player(board);
                board = make_move(board, dir);

                if (game_over(board)) return board;
            } 

            board = add_random_tile(board);
        }

        return board;
    }
}
