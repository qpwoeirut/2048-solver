#ifndef ROTATING_PLAYER_HPP
#define ROTATING_PLAYER_HPP

#include "Strategy.hpp"

class RotatingPlayer: public Strategy {
    int current_move = 0;

    public:
    const int pick_move(const board_t board) override {
        do {
            current_move = (current_move + 1) % 4;
        } while (board == game::make_move(board, current_move));
        return current_move;
    }
};

#endif

