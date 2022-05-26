#ifndef RANDOM_PLAYER_HPP
#define RANDOM_PLAYER_HPP

#include "Strategy.hpp"

class RandomPlayer: public Strategy {
    public:
    const int pick_move(const board_t board) override {
        int move;
        do {  // this *shouldn't* infinite loop but i guess we will see...
            move = random_move();
        }
        while (board == game::make_move(board, move));
        return move;
    }

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<RandomPlayer>();
    }
};

#endif

