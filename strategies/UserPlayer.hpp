#ifndef USER_PLAYER_HPP
#define USER_PLAYER_HPP

#include "Strategy.hpp"

class UserPlayer : public Strategy {
public:
    const int pick_move(const board_t board) override {
        print_board(board);
        std::cout << "Next move? (L, U, R, D)\n";

        char move;
        std::cin >> move;

        const int dir = move_to_int(move);
        if (dir == -1) {
            std::cout << "Invalid move!\n";
            return pick_move(board);
        }
        if (board == simulator.make_move(board, dir)) {
            std::cout << "No change!\n";
            return pick_move(board);
        }
        return dir;
    }

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<UserPlayer>();
    }

private:
    const inline int move_to_int(const char c) {
        if (c == 'L') return 0;
        if (c == 'U') return 1;
        if (c == 'R') return 2;
        if (c == 'D') return 3;
        return -1;
    }
};

#endif

