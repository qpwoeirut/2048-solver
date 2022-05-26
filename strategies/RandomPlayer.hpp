#ifndef RANDOM_PLAYER_HPP
#define RANDOM_PLAYER_HPP

#include "Strategy.hpp"
#include <random>

static std::mt19937 seed_generator(get_current_time_ms());  // make sure multiple instances of the random player don't end up making the same moves

class RandomPlayer: public Strategy {

    std::mt19937 move_gen{seed_generator()};
    std::uniform_int_distribution<> move_distrib{0, 3};

    int random_move() {
        return move_distrib(move_gen);
    }

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

