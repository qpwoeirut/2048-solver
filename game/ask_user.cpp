#include <iostream>

#include "game.cpp"


void print_board(const uint64_t board) {
    for (int r=48; r>=0; r-=16) {
        for (int c=12; c>=0; c-=4) {
            std::cout << ((board >> (r + c)) & 0xF) << ' ';
        }
        std::cout << '\n';
    }
}

inline int move_to_int(const char c) {
    if (c == 'L') return 0;
    if (c == 'U') return 1;
    if (c == 'R') return 2;
    if (c == 'D') return 3;
    return -1;
}

int ask_user(const board_t board) {
    print_board(board);
    std::cout << "Next move? (L, U, R, D)\n";

    char move;
    std::cin >> move;

    int dir = move_to_int(move);
    if (dir == -1) {
        std::cout << "Invalid move!\n";
        return ask_user(board);
    }
    if (board == make_move(board, dir)) {
        std::cout << "No change!\n";
        return ask_user(board);
    }
    return dir;
}

int main() {
    init();
    const board_t board = play_game(ask_user);
    std::cout << "Final board:\n";
    print_board(board);
    std::cout << "Score: " << get_max_tile(board) << std::endl;
}

