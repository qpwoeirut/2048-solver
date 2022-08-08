#ifndef ORDERED_PLAYER_HPP
#define ORDERED_PLAYER_HPP

#include "Strategy.hpp"

class OrderedPlayer : public Strategy {
public:
    const int pick_move(const board_t board) override {
        if (board != simulator.make_move(board, 0)) return 0;
        if (board != simulator.make_move(board, 1)) return 1;
        if (board != simulator.make_move(board, 2)) return 2;
        return 3;
    }

    std::unique_ptr<Strategy> clone() override {
        return std::make_unique<OrderedPlayer>();
    }
};

#endif
