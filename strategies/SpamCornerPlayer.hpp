#ifndef SPAM_CORNER_PLAYER_HPP
#define SPAM_CORNER_PLAYER_HPP

#include "Strategy.hpp"

class SpamCornerPlayer : public Strategy {
    std::mt19937 move_gen{std::mt19937()};
    std::uniform_int_distribution<> move_distrib{0, 3};

    int random_move() {
        return move_distrib(move_gen);
    }

public:
    const int pick_move(const board_t board) override {
        const int move = random_move() & 1;
        if (board != simulator.make_move(board, move)) return move;
        if (board != simulator.make_move(board, move ^ 1)) return move ^ 1;
        if (board != simulator.make_move(board, move ^ 2)) return move ^ 2;
        return move ^ 3;
    }

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<SpamCornerPlayer>();
    }
};

#endif
